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
Record regionWrite{};
RuntimePatch regionWritePatch{};
std::uint64_t regionWriteSequence{};
Record objectLook{};
RuntimePatch objectLookPatch{};
std::uint64_t objectLookSequence{};
bool entityManagerLogged{};
std::uintptr_t entityManager{};
std::vector<std::uintptr_t> previousEntityPointers;
Record objectResource{};
RuntimePatch objectResourcePatch{};
std::uint64_t objectResourceSequence{};
std::array<Record,2> building{};
std::array<RuntimePatch,2> buildingPatches{};
std::array<std::uint64_t,3> buildingSequences{};
constexpr const char* BuildingNames[]{"building_place_event","building_remove_event","voxel_write_candidate"};
RuntimePatch terrainPatch{};
std::ofstream output;
ULONGLONG started{},firstSample{};
std::uint64_t cursorSequence{},terrainSequence{};
bool running{};
constexpr char Owner[]="shroudtopia.world-diagnostic";
constexpr unsigned CaptureSeconds=600;
bool Pointer(std::uintptr_t address,std::uintptr_t& value) {
    SIZE_T count{};
    return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&value,sizeof(value),&count) && count==sizeof(value);
}
bool Memory(std::uintptr_t address,void* value,std::size_t size) {
    SIZE_T count{};
    return address && value && size && ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),value,size,&count) && count==size;
}
bool EntityPointers(std::vector<std::uintptr_t>& pointers) {
    std::uint64_t count{},table{};
    if (!entityManager || !Memory(entityManager+0x158,&count,sizeof(count)) ||
        !Memory(entityManager+0x188,&table,sizeof(table)) || !count || count>(1u<<20) || !table) return false;
    pointers.resize(static_cast<std::size_t>(count));
    return Memory(static_cast<std::uintptr_t>(table),pointers.data(),pointers.size()*sizeof(pointers[0]));
}
bool EntityDescription(std::uintptr_t entity,nlohmann::json& description) {
    struct Header {
        std::uint8_t padding[0x10]; std::uint32_t id; std::uint32_t unused;
        std::uintptr_t layout,storage,definition; std::uint64_t row; std::uint64_t componentCount;
    } header{};
    if (!Memory(entity,&header,sizeof(header)) || !header.id || !header.layout || !header.storage ||
        !header.definition || header.componentCount>1024) return false;
    struct Definition { std::uint64_t uuid[2]; std::uintptr_t name; std::uint64_t nameSize; } definition{};
    if (!Memory(header.definition,&definition,sizeof(definition)) || !definition.name ||
        !definition.nameSize || definition.nameSize>128) return false;
    std::string name(static_cast<std::size_t>(definition.nameSize),'\0');
    if (!Memory(definition.name,name.data(),name.size())) return false;
    if (const auto end=name.find('\0'); end!=std::string::npos) name.resize(end);
    constexpr std::uint16_t TransformComponent=125;
    std::uint64_t componentBits{};
    std::uint16_t offset{},stride{};
    if (!Memory(header.layout+(TransformComponent/64)*8,&componentBits,sizeof(componentBits)) ||
        !(componentBits&(std::uint64_t{1}<<(TransformComponent%64))) ||
        !Memory(header.layout+0x84+TransformComponent*2,&offset,sizeof(offset)) ||
        !Memory(header.layout+0xa84+TransformComponent*2,&stride,sizeof(stride)) ||
        stride!=sizeof(WorldNative::Transform)) return false;
    WorldNative::Transform native{};
    const auto row=static_cast<std::uint32_t>(header.row);
    if (!Memory(header.storage+offset+static_cast<std::uintptr_t>(row)*stride,&native,sizeof(native))) return false;
    WorldTransform transform{sizeof(transform)};
    if (!WorldNative::DecodeTransform(native,transform)) return false;
    description={{"object",header.id},{"template_name",name},
        {"template_uuid_qwords",{definition.uuid[0],definition.uuid[1]}},
        {"position",{transform.position.x,transform.position.y,transform.position.z}},
        {"rotation",{transform.rotation.x,transform.rotation.y,transform.rotation.z,transform.rotation.w}},
        {"scale",{transform.scale.x,transform.scale.y,transform.scale.z}}};
    return true;
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
    InterlockedExchange(&regionWrite.enabled,0);
    InterlockedExchange(&objectLook.enabled,0);
    InterlockedExchange(&objectResource.enabled,0);
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
    const std::uint8_t regionOriginal[]{0x4c,0x8b,0xdc,0x4d,0x89,0x4b,0x20};
    const std::uint8_t resourceOriginal[]{0x48,0x8b,0,0x48,0x85,0xc0};
    if (std::memcmp(base+0xe8cb20,regionOriginal,sizeof(regionOriginal)) ||
        std::memcmp(base+0x994810,terrainOriginal,7) ||
        std::memcmp(base+0x23a067,resourceOriginal,sizeof(resourceOriginal))) {
        Utils::Log(LOG_ERROR,"World diagnostic refused: original instructions changed."); return;
    }
    const auto log=std::filesystem::path(path).parent_path()/
        ("shroudtopia-world-diagnostic-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64())+".jsonl");
    output.open(log,std::ios::out);
    if (!output) { Utils::Log(LOG_ERROR,"World diagnostic cannot open evidence log."); return; }
    auto result=Install(terrain,Kind::TerrainEntry,
        "48 8B C4 48 89 58 18 4C 89 48 20 55 56 57 41 54 41 55 41 56 41 57 48 81 EC 00 05 00 00",terrainOriginal,terrainPatch);
    // The same low-level writer is reached by the game and the mod. Capture
    // identities only: argument buffers are transient and must not escape.
    if (result==RESULT_OK) result=Install(regionWrite,Kind::WorldContext,
        "4C 8B DC 4D 89 4B 20 4D 89 43 18 49 89 53 10 49 89 4B 08 41 54 48 81 EC C0 04 00 00 45 33 E4 49 8B C1",
        regionOriginal,regionWritePatch);
    if (result==RESULT_OK) result=Install(objectResource,Kind::ObjectResource,
        "48 8B 00 48 85 C0 75 1D 4C 8B 44 24 58 48 8D 8C 24 C0 00 00 00",
        resourceOriginal,objectResourcePatch,true);
    if (result!=RESULT_OK) {
        // Mailboxes remain disabled. Do not hot-unpatch a possibly installed jump.
        Utils::Log(LOG_ERROR,"World diagnostic installation failed (%d); captures remain disabled.",static_cast<int>(result));
        output<<nlohmann::json{{"event","install_failed"},{"result",static_cast<int>(result)}}.dump()<<'\n'; output.flush(); return;
    }
    started=GetTickCount64(); running=true;
    output<<nlohmann::json{{"event","armed"},{"cursor_rva","0x24aa1d"},{"terrain_rva","0x994810"},
        {"building_rvas",{"0x3ebc82","0x3ebdb2","0x996f90"}},{"object_look_rvas",{"0x23a067","0x23a159"}},
        {"image_base",reinterpret_cast<std::uintptr_t>(base)},
        {"mode","latest sample, not complete event trace"},{"max_capture_seconds",CaptureSeconds},{"pid",GetCurrentProcessId()}}.dump()<<'\n'; output.flush();
    InterlockedExchange(&terrain.enabled,1);
    InterlockedExchange(&regionWrite.enabled,1);
    InterlockedExchange(&objectLook.enabled,1);
    InterlockedExchange(&objectResource.enabled,1);
    for (auto& record:building) InterlockedExchange(&record.enabled,1);
    Utils::Log(LOG_INFO,"World diagnostic armed: %s; %u seconds from first sample; no world writes.",log.string().c_str(),CaptureSeconds);
}
void Tick(bool enabled) {
    if (!running) return;
    if (!enabled) { Stop("disabled"); return; }
    const auto now=GetTickCount64();
    if ((firstSample && now-firstSample>=CaptureSeconds*1000ULL) || (!firstSample && now-started>=600000)) { Stop("time_limit"); return; }
    Record sample{};
    if (Snapshot(objectResource,sample) && sample.sequence!=objectResourceSequence) {
        objectResourceSequence=sample.sequence; if (!firstSample) firstSample=now;
        std::array<std::uint64_t,2> prefix{};
        std::memcpy(prefix.data(),&sample.cursor,sizeof(prefix));
        output<<nlohmann::json{{"event","object_resource_candidate"},{"tick_ms",now},
            {"sequence",sample.sequence},{"thread",sample.thread},{"object",sample.arguments[4]},
            {"component_identity",sample.arguments[0]},{"component_prefix",prefix},
            {"note","16-byte prefix read synchronously where the engine dereferences the component"}}.dump()<<'\n';
    }
    if (WorldEngine::DiagnosticObjectSnapshot(sample) && sample.sequence!=objectLookSequence) {
        objectLookSequence=sample.sequence; if (!firstSample) firstSample=now;
        const auto* bytes=reinterpret_cast<const std::uint8_t*>(&sample.cursor);
        std::uint32_t object{}; std::uint64_t resource{};
        std::memcpy(&object,bytes+0x64,sizeof(object));
        std::memcpy(&resource,bytes+0x78,sizeof(resource));
        std::array<std::uint64_t,17> raw{}; std::memcpy(raw.data(),bytes,0x88);
        std::int64_t fixed[3]{}; std::memcpy(fixed,bytes,sizeof(fixed));
        float rotation[4]{},scale[3]{};
        std::memcpy(rotation,bytes+0x18,sizeof(rotation));
        std::memcpy(scale,bytes+0x28,sizeof(scale));
        constexpr double divisor=4294967296.0;
        struct Metadata { std::uint64_t uuid[2]; std::uintptr_t name; std::uint64_t name_size; } metadata{};
        std::uintptr_t metadataAddress{}; std::string name;
        const bool metadataReadable=sample.arguments[2] &&
            Pointer(static_cast<std::uintptr_t>(sample.arguments[2])+0x30,metadataAddress) &&
            Memory(metadataAddress,&metadata,sizeof(metadata));
        if (metadataReadable && metadata.name && metadata.name_size && metadata.name_size<=256) {
            name.resize(static_cast<std::size_t>(metadata.name_size));
            if (!Memory(metadata.name,name.data(),name.size())) name.clear();
            else if (const auto end=name.find('\0'); end!=std::string::npos) name.resize(end);
        }
        output<<nlohmann::json{{"event","object_look_candidate"},{"tick_ms",now},
            {"sequence",sample.sequence},{"thread",sample.thread},{"object",object},
            {"lookup_context",sample.arguments[4]},
            {"resource_identity",resource},{"slot_flags",bytes[0x82]},
            {"candidate_position",{fixed[0]/divisor,fixed[1]/divisor,fixed[2]/divisor}},
            {"candidate_rotation",rotation},{"candidate_scale",scale},{"raw_qwords",raw},
            {"metadata_readable",metadataReadable},{"metadata_uuid",{metadata.uuid[0],metadata.uuid[1]}},
            {"metadata_name",name},{"note","candidate layout; post-snapshot metadata uses bounded ReadProcessMemory"}}.dump()<<'\n';
        if (!entityManagerLogged && (sample.arguments[5] || sample.arguments[4])) {
            std::uintptr_t contextRoot{},manager{};
            std::array<std::uint64_t,11> header{};
            manager=static_cast<std::uintptr_t>(sample.arguments[5]);
            if (!manager && sample.arguments[4] &&
                Pointer(static_cast<std::uintptr_t>(sample.arguments[4]),contextRoot))
                Pointer(contextRoot+0x30,manager);
            if (manager && Memory(manager+0x140,header.data(),sizeof(header))) {
                entityManagerLogged=true;
                entityManager=manager;
                output<<nlohmann::json{{"event","entity_manager_candidate"},{"tick_ms",now},
                    {"lookup_context",sample.arguments[4]},{"context_root",contextRoot},{"manager",manager},
                    {"captured_in_hook",sample.arguments[5]!=0},
                    {"offset_140_qwords",header},
                    {"note","bounded table-header snapshot; no object pointers dereferenced"}}.dump()<<'\n';
            }
        }
    }
    if (Snapshot(regionWrite,sample) && sample.sequence!=regionWriteSequence) {
        regionWriteSequence=sample.sequence; if (!firstSample) firstSample=now;
        const auto base=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
        const auto caller=sample.arguments[4];
        output<<nlohmann::json{{"event","native_region_write"},{"tick_ms",now},
            {"sequence",sample.sequence},{"thread",sample.thread},{"world",sample.arguments[0]},
            {"caller",caller},{"caller_game_rva",caller>=base && caller<base+0x2da7000 ? caller-base : 0},
            {"note","latest sample only; pointers are identities, not dereferenced"}}.dump()<<'\n';
    }
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
        if (!WorldEngine::DiagnosticBuildingSnapshot(i==1,sample) || sample.sequence==buildingSequences[i]) continue;
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
    if (entityManager) {
        std::vector<std::uintptr_t> current;
        if (EntityPointers(current)) {
            if (!previousEntityPointers.empty() && previousEntityPointers.size()==current.size()) {
                std::size_t emitted{};
                for (std::size_t i=0;i<current.size() && emitted<256;++i) {
                    if (!current[i] || current[i]==previousEntityPointers[i]) continue;
                    nlohmann::json entity;
                    if (!EntityDescription(current[i],entity)) continue;
                    entity["event"]="entity_created_candidate";
                    entity["tick_ms"]=now;
                    entity["slot"]=i;
                    output<<entity.dump()<<'\n';
                    ++emitted;
                }
            }
            previousEntityPointers=std::move(current);
        }
    }
    std::uintptr_t capturedWorld{}; std::uint64_t worldSequence{}; std::uint32_t worldThread{};
    std::array<std::uint64_t,6> worldArguments{}; WorldNative::Cursor worldDescriptor{};
    if (WorldEngine::DiagnosticWorldSnapshot(capturedWorld,worldSequence,worldThread,worldArguments,worldDescriptor) &&
        worldSequence!=buildingSequences[2]) {
        buildingSequences[2]=worldSequence; if (!firstSample) firstSample=now;
        std::array<std::uint32_t,18> descriptor{};
        std::memcpy(descriptor.data(),&worldDescriptor,sizeof(descriptor));
        const auto* data=reinterpret_cast<const std::uint8_t*>(&worldDescriptor);
        output<<nlohmann::json{{"event",BuildingNames[2]},{"tick_ms",now},{"sequence",worldSequence},
            {"thread",worldThread},{"arguments",worldArguments},{"captured_world",capturedWorld},
            {"descriptor_u32",descriptor},{"argument_bytes_7_8_9",{data[72],data[80],data[88]}}}.dump()<<'\n';
    }
    output.flush();
    if (!output) Stop("log_write_failed");
}
}
