#include "pch.h"
#include "world_engine.h"
#include "world_diagnostic_payload.h"
#include "runtime_patches.h"
#include "shroudtopia_world.h"
#include "utils.h"
#include <limits>
#include <mutex>

namespace WorldEngine {
namespace {
WorldDiagnostic::Record cursor{};
RuntimePatch cursorPatch{};
Registration serviceRegistration{};
constexpr char Owner[]="shroudtopia.world-native";
constexpr char VoxelGrid[]="voxel";
constexpr std::uintptr_t ActiveContextRva=0x273f588;
constexpr std::uintptr_t ReadVoxelRegionRva=0xe819d0;
constexpr char ReadVoxelRegionSignature[]="44 89 4C 24 20 4C 89 44 24 18 48 89 4C 24 08 55 53 56 57 41 54 41 55 41 56 48 8D AC 24 40 FB FF FF 48 81 EC C0 05 00 00 48 8B 85 20 05 00 00 45 8B D1 8B 3A 40 B6 01";
constexpr std::uintptr_t WriteVoxelRegionRva=0xe8cb20;
constexpr char WriteVoxelRegionSignature[]="4C 8B DC 4D 89 4B 20 4D 89 43 18 49 89 53 10 49 89 4B 08 41 54 48 81 EC C0 04 00 00 45 33 E4 49 8B C1";
constexpr std::ptrdiff_t VoxelWorldFromContext=-0x5a8;
constexpr std::size_t MaximumReadCells=65'536; // Raised only after bounded live-read validation.
constexpr std::size_t MaximumWriteCells=4'096; // Expanded, chunk-aligned cells for the first live write test.
std::mutex gridMutex;
struct CellSpan { WorldNative::TerrainCell* data; std::uint64_t size; };
using NativeRead=bool (CALL*)(CellSpan*,const std::int32_t*,const void*,std::uint32_t,const std::uint32_t*);
using NativeWrite=void (CALL*)(void*,CellSpan*,const std::uint32_t*,const std::int32_t*);
bool SafeNativeRead(NativeRead read,CellSpan* span,const std::int32_t* origin,
                    const void* world,const std::uint32_t* dimensions) {
    __try { return read(span,origin,world,6,dimensions); }
    __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool SafeNativeWrite(NativeWrite write,void* world,CellSpan* span,
                     const std::uint32_t* dimensions,const std::int32_t* origin) {
    __try { write(world,span,dimensions,origin); return true; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
StringView View(const char* value) { return {value,std::strlen(value)}; }
bool Equal(StringView value,const char* expected) {
    const auto size=std::strlen(expected);
    return value.data && value.size==size && std::memcmp(value.data,expected,size)==0;
}
bool ReadPointer(std::uintptr_t address,std::uintptr_t& value) {
    SIZE_T count{};
    return address && ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),
        &value,sizeof(value),&count) && count==sizeof(value);
}
bool ResolveWorld(const std::uint8_t* base,std::uintptr_t& world) {
    std::uintptr_t singleton{},context{},voxelStore{};
    if (!ReadPointer(reinterpret_cast<std::uintptr_t>(base)+ActiveContextRva,singleton) || !singleton ||
        !ReadPointer(singleton+0xc8,context) || !context ||
        context<static_cast<std::uintptr_t>(-VoxelWorldFromContext)) return false;
    world=context+VoxelWorldFromContext;
    return ReadPointer(world+0x1210,voxelStore) && voxelStore;
}
bool NativeReadRegion(const std::uint8_t* base,std::uintptr_t world,const std::int32_t* origin,
                      const std::uint32_t* dimensions,std::vector<WorldNative::TerrainCell>& cells) {
    CellSpan span{cells.data(),cells.size()};
    const auto read=reinterpret_cast<NativeRead>(const_cast<std::uint8_t*>(base)+ReadVoxelRegionRva);
    return SafeNativeRead(read,&span,origin,reinterpret_cast<const void*>(world),dimensions);
}
bool AlignChunk(std::int32_t minimum,std::int32_t maximum,std::int32_t& alignedMinimum,
                std::int32_t& alignedMaximum) {
    auto floor=static_cast<std::int64_t>(minimum)/8;
    if (minimum<0 && minimum%8) --floor;
    auto ceil=static_cast<std::int64_t>(maximum)/8;
    if (maximum>0 && maximum%8) ++ceil;
    const auto low=floor*8,high=ceil*8;
    if (low<INT32_MIN || low>INT32_MAX || high<INT32_MIN || high>INT32_MAX || high<=low) return false;
    alignedMinimum=static_cast<std::int32_t>(low);
    alignedMaximum=static_cast<std::int32_t>(high);
    return true;
}
bool Coordinate(double value,std::int32_t& result) {
    if (!std::isfinite(value)) return false;
    const auto scaled=value*2.0;
    const auto rounded=std::round(scaled);
    if (std::abs(scaled-rounded)>1e-7 || rounded<(std::numeric_limits<std::int32_t>::min)() ||
        rounded>(std::numeric_limits<std::int32_t>::max)()) return false;
    result=static_cast<std::int32_t>(rounded);
    return true;
}
bool Snapshot(WorldDiagnostic::Record& output) {
    if (InterlockedCompareExchange(&cursor.lock,1,0)!=0) return false;
    std::memcpy(&output,&cursor,sizeof(output));
    InterlockedExchange(&cursor.lock,0);
    return true;
}
Result CALL GetCursor(StringView owner,CursorSnapshot* output) {
    if ((owner.size && !owner.data) || !owner.size || !output || output->struct_size<sizeof(*output))
        return RESULT_INVALID_ARGUMENT;
    WorldDiagnostic::Record sample{};
    if (!Snapshot(sample) || !sample.sequence) return RESULT_NOT_AVAILABLE;
    const auto bytes=std::span(reinterpret_cast<const std::uint8_t*>(&sample.cursor),sizeof(sample.cursor));
    const auto result=WorldNative::DecodeCursor(bytes,output);
    if (result==RESULT_OK) output->sequence=sample.sequence;
    return result;
}
Result CALL GetGrid(StringView owner,StringView id,WorldGridSpec* output) {
    if ((owner.size && !owner.data) || !owner.size || !Equal(id,VoxelGrid) || !output ||
        output->struct_size<sizeof(*output)) return RESULT_INVALID_ARGUMENT;
    *output={sizeof(*output),View(VoxelGrid),{0,0,0},{.5,.5,.5},
        static_cast<std::size_t>(INT32_MAX),static_cast<std::size_t>(INT32_MAX),static_cast<std::size_t>(INT32_MAX)};
    return RESULT_OK;
}
Result CALL ReadGrid(StringView owner,StringView id,const WorldBounds* bounds,std::uint32_t* values,
                     WorldCellState* states,std::size_t capacity,std::size_t* actual) {
    if ((owner.size && !owner.data) || !owner.size || !Equal(id,VoxelGrid) || !bounds ||
        bounds->struct_size<sizeof(*bounds) || !values || !states || !actual) return RESULT_INVALID_ARGUMENT;
    std::int32_t minimum[3]{},maximum[3]{};
    const double low[]{bounds->minimum.x,bounds->minimum.y,bounds->minimum.z};
    const double high[]{bounds->maximum.x,bounds->maximum.y,bounds->maximum.z};
    std::uint32_t dimensions[3]{};
    std::size_t count=1;
    for (int i=0;i<3;++i) {
        if (!Coordinate(low[i],minimum[i]) || !Coordinate(high[i],maximum[i]) || maximum[i]<=minimum[i])
            return RESULT_INVALID_ARGUMENT;
        const auto extent=static_cast<std::uint64_t>(static_cast<std::int64_t>(maximum[i])-minimum[i]);
        if (extent>UINT32_MAX || extent>MaximumReadCells/count) return RESULT_INVALID_ARGUMENT;
        dimensions[i]=static_cast<std::uint32_t>(extent); count*=dimensions[i];
    }
    *actual=count;
    if (count>capacity || count>MaximumReadCells) return RESULT_INVALID_ARGUMENT;
    const auto* base=reinterpret_cast<const std::uint8_t*>(GetModuleHandleW(nullptr));
    std::uintptr_t world{};
    if (!ResolveWorld(base,world)) return RESULT_NOT_AVAILABLE;
    std::vector<WorldNative::TerrainCell> cells(count);
    bool complete{};
    {
        std::scoped_lock lock(gridMutex);
        complete=NativeReadRegion(base,world,minimum,dimensions,cells);
    }
    if (!complete) {
        std::fill(states,states+count,WorldCellState::Unknown);
        std::fill(values,values+count,0);
        return RESULT_NOT_FOUND;
    }
    for (std::size_t i=0;i<count;++i) {
        values[i]=WorldNative::EncodeCell(cells[i]);
        states[i]=values[i] ? WorldCellState::Occupied : WorldCellState::Empty;
    }
    return RESULT_OK;
}
Result CALL WriteGrid(StringView owner,StringView id,const WorldBounds* bounds,const std::uint32_t* values,
                      const WorldCellState* states,std::size_t count) {
    if ((owner.size && !owner.data) || !owner.size || !Equal(id,VoxelGrid) || !bounds ||
        bounds->struct_size<sizeof(*bounds) || !values || !states) return RESULT_INVALID_ARGUMENT;
    std::int32_t minimum[3]{},maximum[3]{},expandedMinimum[3]{},expandedMaximum[3]{};
    const double low[]{bounds->minimum.x,bounds->minimum.y,bounds->minimum.z};
    const double high[]{bounds->maximum.x,bounds->maximum.y,bounds->maximum.z};
    std::uint32_t dimensions[3]{},expandedDimensions[3]{};
    std::size_t expected=1,expandedCount=1;
    for (int i=0;i<3;++i) {
        if (!Coordinate(low[i],minimum[i]) || !Coordinate(high[i],maximum[i]) || maximum[i]<=minimum[i] ||
            !AlignChunk(minimum[i],maximum[i],expandedMinimum[i],expandedMaximum[i])) return RESULT_INVALID_ARGUMENT;
        const auto extent=static_cast<std::uint64_t>(static_cast<std::int64_t>(maximum[i])-minimum[i]);
        const auto expanded=static_cast<std::uint64_t>(static_cast<std::int64_t>(expandedMaximum[i])-expandedMinimum[i]);
        if (extent>SIZE_MAX/expected || expanded>MaximumWriteCells/expandedCount) return RESULT_INVALID_ARGUMENT;
        dimensions[i]=static_cast<std::uint32_t>(extent); expected*=dimensions[i];
        expandedDimensions[i]=static_cast<std::uint32_t>(expanded); expandedCount*=expandedDimensions[i];
    }
    if (count!=expected || expandedCount>MaximumWriteCells) return RESULT_INVALID_ARGUMENT;
    for (std::size_t i=0;i<count;++i) {
        if (states[i]<WorldCellState::Unknown || states[i]>WorldCellState::Occupied || values[i]>UINT16_MAX ||
            (states[i]==WorldCellState::Occupied && values[i]==0)) return RESULT_INVALID_ARGUMENT;
    }
    const auto* base=reinterpret_cast<const std::uint8_t*>(GetModuleHandleW(nullptr));
    std::uintptr_t world{};
    if (!ResolveWorld(base,world)) return RESULT_NOT_AVAILABLE;
    std::vector<WorldNative::TerrainCell> expanded(expandedCount);
    std::scoped_lock lock(gridMutex);
    if (!NativeReadRegion(base,world,expandedMinimum,expandedDimensions,expanded)) return RESULT_NOT_FOUND;
    const std::size_t offset[]{
        static_cast<std::size_t>(static_cast<std::int64_t>(minimum[0])-expandedMinimum[0]),
        static_cast<std::size_t>(static_cast<std::int64_t>(minimum[1])-expandedMinimum[1]),
        static_cast<std::size_t>(static_cast<std::int64_t>(minimum[2])-expandedMinimum[2])};
    for (std::size_t z=0;z<dimensions[2];++z) for (std::size_t y=0;y<dimensions[1];++y)
        for (std::size_t x=0;x<dimensions[0];++x) {
            const auto source=(z*dimensions[1]+y)*dimensions[0]+x;
            if (states[source]==WorldCellState::Unknown) continue;
            const auto target=((z+offset[2])*expandedDimensions[1]+y+offset[1])*expandedDimensions[0]+x+offset[0];
            const auto value=states[source]==WorldCellState::Empty ? 0u : values[source];
            expanded[target]={static_cast<std::uint8_t>(value),static_cast<std::uint8_t>(value>>8)};
        }
    CellSpan span{expanded.data(),expanded.size()};
    const auto write=reinterpret_cast<NativeWrite>(const_cast<std::uint8_t*>(base)+WriteVoxelRegionRva);
    return SafeNativeWrite(write,reinterpret_cast<void*>(world),&span,expandedDimensions,expandedMinimum) ?
        RESULT_OK : RESULT_INTERNAL_ERROR;
}
WorldApi service{sizeof(service),WorldApiVersion,nullptr,nullptr,nullptr,nullptr,nullptr,
                 GetGrid,ReadGrid,WriteGrid,GetCursor};
}
Result Initialize(const Api* api) {
    if (!api || serviceRegistration) return !api ? RESULT_INVALID_ARGUMENT : RESULT_CONFLICT;
    const auto* base=reinterpret_cast<const std::uint8_t*>(GetModuleHandleW(nullptr));
    const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    const auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    (void)ReadVoxelRegionSignature; // Kept beside the ABI for offline uniqueness verification.
    (void)WriteVoxelRegionSignature;
    const std::uint8_t original[]{0x41,0x8b,0x96,0xf8,0x02,0,0};
    const std::uint8_t readerOriginal[]{0x44,0x89,0x4c,0x24,0x20,0x4c,0x89,0x44,0x24,0x18};
    const std::uint8_t writerOriginal[]{0x4c,0x8b,0xdc,0x4d,0x89,0x4b,0x20,0x4d,0x89,0x43,0x18};
    if (nt->FileHeader.TimeDateStamp!=0x6a4236c8 || nt->OptionalHeader.SizeOfImage!=0x2da7000 ||
        std::memcmp(base+0x24aa1d,original,sizeof(original)) ||
        std::memcmp(base+ReadVoxelRegionRva,readerOriginal,sizeof(readerOriginal)) ||
        std::memcmp(base+WriteVoxelRegionRva,writerOriginal,sizeof(writerOriginal))) return RESULT_VERSION_MISMATCH;
    auto payload=WorldDiagnostic::Payload(&cursor,WorldDiagnostic::Kind::Cursor);
    payload.insert(payload.end(),std::begin(original),std::end(original));
    payload.push_back(0xe9); const auto returnOffset=payload.size(); payload.insert(payload.end(),4,0);
    const RuntimeRelocation relocation{sizeof(relocation),returnOffset,RUNTIME_RELOCATION_REL32_RETURN};
    constexpr char signature[]="41 8B 96 F8 02 00 00 0F B6 CA 41 0F 10 8E F0 02 00 00 84 D2";
    const RuntimePatchOptions options{sizeof(options),View(signature),0,RUNTIME_PATCH_DETOUR,
        sizeof(original),payload.data(),payload.size(),&relocation,1};
    auto result=RuntimePatches::Create(Owner,&options,&cursorPatch);
    if (result==RESULT_OK) result=RuntimePatches::SetEnabled(Owner,cursorPatch,true);
    if (result!=RESULT_OK) return result;
    const ServiceDescriptor descriptor{sizeof(descriptor),View(WorldServiceId),WorldServiceMajor,WorldServiceMinor,&service};
    result=api->register_service(View(Owner),&descriptor,&serviceRegistration);
    if (result!=RESULT_OK) { InterlockedExchange(&cursor.enabled,0); return result; }
    InterlockedExchange(&cursor.enabled,1);
    Utils::Log(LOG_INFO,"Native world service ready: cursor=yes entities=no grids=voxel-read-write (bounded chunk RMW)." );
    return RESULT_OK;
}
void Shutdown(const Api* api) {
    InterlockedExchange(&cursor.enabled,0);
    if (api && serviceRegistration) api->release_registration(serviceRegistration);
    serviceRegistration=0;
}
bool DiagnosticSnapshot(WorldNative::Cursor& value,std::uint64_t& sequence,
                        std::uint32_t& thread,std::uintptr_t& input) {
    WorldDiagnostic::Record sample{};
    if (!Snapshot(sample) || !sample.sequence) return false;
    value=sample.cursor; sequence=sample.sequence; thread=sample.thread;
    input=static_cast<std::uintptr_t>(sample.arguments[0]); return true;
}
}
