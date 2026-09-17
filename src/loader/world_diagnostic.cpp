#include "pch.h"
#include "world_diagnostic.h"
#include "world_diagnostic_payload.h"
#include "building_native_layout.h"
#include "world_engine.h"
#include "runtime_patches.h"
#include "utils.h"
#include <filesystem>
#include <fstream>

namespace WorldDiagnostic {
namespace {
Record terrain{};
std::array<Record,3> building{};
std::array<RuntimePatch,3> buildingPatches{};
std::array<std::uint64_t,3> buildingSequences{};
constexpr const char* BuildingNames[]{"building_place_event","building_remove_event","voxel_write_candidate"};
RuntimePatch terrainPatch{};
std::ofstream output;
ULONGLONG started{},firstSample{};
std::uint64_t cursorSequence{},terrainSequence{};
bool running{};
constexpr char Owner[]="shroudtopia.world-diagnostic";
constexpr unsigned CaptureSeconds=300;
bool Pointer(std::uintptr_t address,std::uintptr_t& value) {
    SIZE_T count{};
    return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&value,sizeof(value),&count) && count==sizeof(value);
}
bool Snapshot(Record& source,Record& destination) {
    if (InterlockedCompareExchange(&source.lock,1,0)!=0) return false;
    std::memcpy(&destination,&source,sizeof(source));
    InterlockedExchange(&source.lock,0);
    return true;
}
Result Install(Record& record,Kind kind,const char* signature,
               std::span<const std::uint8_t> original,RuntimePatch& patch,bool replayBefore=false) {
    auto payload=Payload(&record,kind);
    payload.insert(replayBefore ? payload.begin() : payload.end(),original.begin(),original.end());
    payload.push_back(0xe9);
    const auto returnOffset=payload.size(); payload.insert(payload.end(),4,0);
    const RuntimeRelocation relocation{sizeof(relocation),returnOffset,RUNTIME_RELOCATION_REL32_RETURN};
    const RuntimePatchOptions options{sizeof(options),{signature,std::strlen(signature)},0,
        RUNTIME_PATCH_DETOUR,original.size(),payload.data(),payload.size(),&relocation,1};
    auto result=RuntimePatches::Create(Owner,&options,&patch);
    if (result==RESULT_OK) result=RuntimePatches::SetEnabled(Owner,patch,true);
    return result;
}
void Stop(const char* reason) {
    InterlockedExchange(&terrain.enabled,0);
    for (auto& record:building) InterlockedExchange(&record.enabled,0);
    running=false;
    if (output) { output<<nlohmann::json{{"event","stop"},{"reason",reason},{"cursor_samples",cursorSequence},{"terrain_samples",terrainSequence},{"building_samples",buildingSequences}}.dump()<<'\n'; output.flush(); }
    Utils::Log(LOG_INFO,"World diagnostic stopped: %s (cursor=%llu terrain=%llu). Hooks remain inert until process shutdown.",reason,cursorSequence,terrainSequence);
}
}
void Start(bool enabled) {
    if (!enabled) return;
    const auto base=reinterpret_cast<std::uint8_t*>(GetModuleHandleW(nullptr));
    const auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    const auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    wchar_t path[32768]{};
    const auto length=GetModuleFileNameW(nullptr,path,32768);
    if (!length || length==32768 || std::filesystem::path(path).filename()!=L"enshrouded.exe" ||
        nt->FileHeader.TimeDateStamp!=0x6a4236c8 || nt->OptionalHeader.SizeOfImage!=0x2da7000) {
        Utils::Log(LOG_ERROR,"World diagnostic refused: unsupported client build."); return;
    }
    // Never arm this experiment by configuration reload in an active world.
    // Start is called once during loader startup; active-context check is additional.
    std::uintptr_t singleton{},context{},world{};
    if (!Pointer(reinterpret_cast<std::uintptr_t>(base)+0x273f588,singleton) ||
        (singleton && (!Pointer(singleton+0xc8,context) || (context && (!Pointer(context,world) || world))))) {
        Utils::Log(LOG_ERROR,"World diagnostic refused: active or unreadable world context; restart required."); return;
    }
    const std::uint8_t terrainOriginal[]{0x48,0x8b,0xc4,0x48,0x89,0x58,0x18};
    const std::uint8_t placeOriginal[]{0x41,0x8b,0x86,0x34,0x01,0,0,0x89,0x43,0x44};
    const std::uint8_t removeOriginal[]{0x41,0x8b,0x86,0x34,0x01,0,0,0x89,0x43,0x40};
    const std::uint8_t writeOriginal[]{0x4c,0x89,0x4c,0x24,0x20,0x4c,0x89,0x44,0x24,0x18};
    if (std::memcmp(base+0x994810,terrainOriginal,7) ||
        std::memcmp(base+0x3ebc82,placeOriginal,10) || std::memcmp(base+0x3ebdb2,removeOriginal,10) || std::memcmp(base+0x996f90,writeOriginal,10)) {
        Utils::Log(LOG_ERROR,"World diagnostic refused: original instructions changed."); return;
    }
    const auto log=std::filesystem::path(path).parent_path()/
        ("shroudtopia-world-diagnostic-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64())+".jsonl");
    output.open(log,std::ios::out);
    if (!output) { Utils::Log(LOG_ERROR,"World diagnostic cannot open evidence log."); return; }
    auto result=Install(terrain,Kind::TerrainEntry,
        "48 8B C4 48 89 58 18 4C 89 48 20 55 56 57 41 54 41 55 41 56 41 57 48 81 EC 00 05 00 00",terrainOriginal,terrainPatch);
    if (result==RESULT_OK) result=Install(building[0],Kind::BuildingEvent,
        "41 8B 86 34 01 00 00 89 43 44 48 8B 5C 24 48",placeOriginal,buildingPatches[0],true);
    if (result==RESULT_OK) result=Install(building[1],Kind::BuildingEvent,
        "41 8B 86 34 01 00 00 89 43 40 48 8B 5C 24 48",removeOriginal,buildingPatches[1],true);
    if (result==RESULT_OK) result=Install(building[2],Kind::VoxelWriteEntry,
        "4C 89 4C 24 20 4C 89 44 24 18 48 89 54 24 10 48 89 4C 24 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 18 FF FF FF 48 81 EC E8 01 00 00",writeOriginal,buildingPatches[2]);
    if (result!=RESULT_OK) {
        // Mailboxes remain disabled. Do not hot-unpatch a possibly installed jump.
        Utils::Log(LOG_ERROR,"World diagnostic installation failed (%d); captures remain disabled.",static_cast<int>(result));
        output<<nlohmann::json{{"event","install_failed"},{"result",static_cast<int>(result)}}.dump()<<'\n'; output.flush(); return;
    }
    started=GetTickCount64(); running=true;
    output<<nlohmann::json{{"event","armed"},{"cursor_rva","0x24aa1d"},{"terrain_rva","0x994810"},
        {"building_rvas",{"0x3ebc82","0x3ebdb2","0x996f90"}},
        {"image_base",reinterpret_cast<std::uintptr_t>(base)},
        {"mode","latest sample, not complete event trace"},{"max_capture_seconds",CaptureSeconds},{"pid",GetCurrentProcessId()}}.dump()<<'\n'; output.flush();
    InterlockedExchange(&terrain.enabled,1);
    for (auto& record:building) InterlockedExchange(&record.enabled,1);
    Utils::Log(LOG_INFO,"World diagnostic armed: %s; %u seconds from first sample; no world writes.",log.string().c_str(),CaptureSeconds);
}
void Tick(bool enabled) {
    if (!running) return;
    if (!enabled) { Stop("disabled"); return; }
    const auto now=GetTickCount64();
    if ((firstSample && now-firstSample>=CaptureSeconds*1000ULL) || (!firstSample && now-started>=600000)) { Stop("time_limit"); return; }
    Record sample{};
    WorldNative::Cursor cursorValue{}; std::uint64_t sequence{}; std::uint32_t thread{}; std::uintptr_t input{};
    if (WorldEngine::DiagnosticSnapshot(cursorValue,sequence,thread,input) && sequence!=cursorSequence) {
        cursorSequence=sequence; if (!firstSample) firstSample=now;
        CursorSnapshot decoded{sizeof(decoded)};
        const auto bytes=std::span(reinterpret_cast<const std::uint8_t*>(&cursorValue),sizeof(cursorValue));
        if (WorldNative::DecodeCursor(bytes,&decoded)==RESULT_OK) {
            output<<nlohmann::json{{"event","cursor"},{"tick_ms",now},{"sequence",sequence},{"thread",thread},
                {"input",input},{"primary",{decoded.primary.position.x,decoded.primary.position.y,decoded.primary.position.z}},
                {"secondary",{decoded.secondary.position.x,decoded.secondary.position.y,decoded.secondary.position.z}},
                {"flags",{decoded.primary_flags,decoded.secondary_flags}},{"material",decoded.material},
                {"object",cursorValue.selected.value},{"object_type",cursorValue.selected.type}}.dump()<<'\n';
        }
    }
    if (Snapshot(terrain,sample) && sample.sequence!=terrainSequence) {
        terrainSequence=sample.sequence; if (!firstSample) firstSample=now;
        output<<nlohmann::json{{"event","terrain_entry"},{"tick_ms",now},{"sequence",sample.sequence},{"thread",sample.thread},
            {"arguments",sample.arguments},{"note","raw identities only; no argument dereference or retained-pointer use"}}.dump()<<'\n';
    }
    for (std::size_t i=0;i<building.size();++i) {
        if (!Snapshot(building[i],sample) || sample.sequence==buildingSequences[i]) continue;
        buildingSequences[i]=sample.sequence; if (!firstSample) firstSample=now;
        nlohmann::json event{{"event",BuildingNames[i]},{"tick_ms",now},{"sequence",sample.sequence},
            {"thread",sample.thread},{"arguments",sample.arguments}};
        if (i<2) {
            BuildingNative::Event data{}; std::memcpy(&data,&sample.cursor,sizeof(data));
            event["position"]=data.position; event["orientation"]=data.orientation;
            event["volume_min"]=data.volume_min; event["volume_max"]=data.volume_max;
            event["material_feedback"]=data.material_feedback;
            event["owner"]=i==0 ? data.owner_or_padding : data.tracking_or_owner;
            if (i==0) event["tracking_item"]=data.tracking_or_owner;
        } else {
            std::array<std::uint32_t,18> descriptor{};
            std::memcpy(descriptor.data(),&sample.cursor,sizeof(descriptor));
            event["descriptor_u32"]=descriptor;
            const auto* data=reinterpret_cast<const std::uint8_t*>(&sample.cursor);
            event["argument_bytes_7_8_9"]={data[72],data[80],data[88]};
        }
        output<<event.dump()<<'\n';
    }
    output.flush();
    if (!output) Stop("log_write_failed");
}
}
