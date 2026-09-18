#include "pch.h"
#include "world_engine.h"
#include "world_diagnostic_payload.h"
#include "building_native_layout.h"
#include "runtime_patches.h"
#include "shroudtopia_world.h"
#include "utils.h"
#include <limits>
#include <mutex>
#include <charconv>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <unordered_map>
#include <atomic>

namespace WorldEngine {
namespace {
WorldDiagnostic::Record cursor{};
WorldDiagnostic::Record worldContext{};
RuntimePatch cursorPatch{};
RuntimePatch worldContextPatch{};
WorldDiagnostic::Record objectContext{};
WorldDiagnostic::Record placeEntryContext{};
WorldDiagnostic::Record actorWorldContext{};
WorldDiagnostic::Record propUpdateContext{};
WorldDiagnostic::Record actorPlacementContext{};
WorldDiagnostic::Record buildingDispatchContext{};
WorldDiagnostic::Record placeContext{};
WorldDiagnostic::Record removeContext{};
RuntimePatch objectContextPatch{};
RuntimePatch placeEntryContextPatch{};
RuntimePatch actorWorldContextPatch{};
RuntimePatch propUpdateContextPatch{};
RuntimePatch actorPlacementContextPatch{};
RuntimePatch buildingDispatchContextPatch{};
RuntimePatch placeContextPatch{};
RuntimePatch removeContextPatch{};
Registration serviceRegistration{};
const Api* hostApi{};
constexpr char Owner[]="shroudtopia.world-native";
constexpr char AssetOwner[]="shroudtopia.core";
constexpr char VoxelGrid[]="voxel";
constexpr std::uintptr_t ActiveContextRva=0x273f588;
constexpr std::uintptr_t ReadVoxelRegionRva=0xe819d0;
constexpr char ReadVoxelRegionSignature[]="44 89 4C 24 20 4C 89 44 24 18 48 89 4C 24 08 55 53 56 57 41 54 41 55 41 56 48 8D AC 24 40 FB FF FF 48 81 EC C0 05 00 00 48 8B 85 20 05 00 00 45 8B D1 8B 3A 40 B6 01";
constexpr std::uintptr_t WriteVoxelRegionRva=0xe8cb20;
constexpr char WriteVoxelRegionSignature[]="4C 8B DC 4D 89 4B 20 4D 89 43 18 49 89 53 10 49 89 4B 08 41 54 48 81 EC C0 04 00 00 45 33 E4 49 8B C1";
constexpr std::ptrdiff_t VoxelWorldFromContext=-0x5a8;
constexpr std::size_t MaximumReadCells=65'536; // Raised only after bounded live-read validation.
constexpr std::size_t MaximumWriteCells=65'536; // Bounded chunk-aligned RMW, matching the validated read ceiling.
std::mutex gridMutex;
std::mutex entityMutex;
std::atomic<std::uintptr_t> preferredVoxelWorld{};
std::uintptr_t cachedEntityManager{};
struct PlacementRecipe { std::uint32_t tracking{},feedback{}; float bounds[6]{}; std::uint64_t templateUuid[2]{}; };
std::unordered_map<std::uint32_t,PlacementRecipe> placementRecipes;
std::unordered_map<std::uint64_t,PlacementRecipe> spawnedRecipes;
std::uint64_t placementRecipeSequence{};
bool assetRecipesAttempted{};
bool placementContextLogged{};
bool voxelWorldContextLogged{};
std::uint64_t loggedPlaceEntrySequence{},loggedActorSequence{},loggedBuildingSequence{};
thread_local std::vector<std::string> entityTemplateIds;
struct EntityHeader {
    std::uint8_t padding[0x10]; std::uint32_t id; std::uint32_t unused;
    std::uintptr_t layout,storage,definition; std::uint64_t row,componentCount;
};
struct EntityDefinition { std::uint64_t uuid[2]; std::uintptr_t name; std::uint64_t nameSize; };
bool SnapshotRecord(WorldDiagnostic::Record& source,WorldDiagnostic::Record& output);
struct CellSpan { WorldNative::TerrainCell* data; std::uint64_t size; };
using NativeRead=bool (CALL*)(CellSpan*,const std::uint32_t*,const void*,std::uint32_t,const std::int32_t*);
using NativeWrite=void (CALL*)(void*,CellSpan*,const std::uint32_t*,const std::int32_t*);
bool SafeNativeRead(NativeRead read,CellSpan* span,const std::int32_t* origin,
                    const void* world,const std::uint32_t* dimensions) {
    __try { return read(span,dimensions,world,6,origin); }
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
bool ReadMemory(std::uintptr_t address,void* value,std::size_t size) {
    SIZE_T count{};
    return address && value && size && ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),
        value,size,&count) && count==size;
}
bool ResolveEntityManager(std::uintptr_t& manager) {
    WorldDiagnostic::Record sample{};
    if (!SnapshotRecord(objectContext,sample) || !sample.sequence) return false;
    // ObjectLook resolves the manager synchronously because arguments[4] is a
    // transient lookup context that can already be invalid on the next tick.
    manager=static_cast<std::uintptr_t>(sample.arguments[5]);
    if (manager) return true;
    if (!sample.arguments[4]) return false;
    // Compatibility fallback for records produced by the earlier payload.
    std::uintptr_t root{};
    return ReadPointer(static_cast<std::uintptr_t>(sample.arguments[4]),root) && root &&
        ReadPointer(root+0x30,manager) && manager;
}
bool ReadEntityPointers(std::uintptr_t manager,std::vector<std::uintptr_t>& pointers) {
    std::uintptr_t table{}; std::uint64_t count{};
    if (!manager || !ReadMemory(manager+0x158,&count,sizeof(count)) ||
        !ReadPointer(manager+0x188,table) || !table || !count || count>(1u<<20)) return false;
    pointers.resize(static_cast<std::size_t>(count));
    return ReadMemory(table,pointers.data(),pointers.size()*sizeof(pointers[0]));
}
bool EntityPointers(std::vector<std::uintptr_t>& pointers) {
    if (cachedEntityManager && ReadEntityPointers(cachedEntityManager,pointers)) return true;
    for (int attempt=0;attempt<20;++attempt) {
        std::uintptr_t manager{};
        if (ResolveEntityManager(manager) && ReadEntityPointers(manager,pointers)) {
            cachedEntityManager=manager;
            return true;
        }
        if (attempt!=19) Sleep(5);
    }
    return false;
}
bool Component(const EntityHeader& header,std::uint16_t index,std::uintptr_t& address,std::uint16_t& stride) {
    std::uint64_t bits{}; std::uint16_t offset{};
    if (!header.layout || !header.storage || header.componentCount>1024 ||
        !ReadMemory(header.layout+(index/64)*8,&bits,sizeof(bits)) ||
        !(bits&(std::uint64_t{1}<<(index%64))) ||
        !ReadMemory(header.layout+0x84+index*2,&offset,sizeof(offset)) ||
        !ReadMemory(header.layout+0xa84+index*2,&stride,sizeof(stride)) || !stride || stride>0x1000) return false;
    address=header.storage+offset+static_cast<std::uintptr_t>(static_cast<std::uint32_t>(header.row))*stride;
    return true;
}
bool ReadEntity(std::uintptr_t pointer,EntityHeader& header,WorldNative::Transform& transform,
                std::uint32_t& tracking,EntityDefinition& definition,std::string& name) {
    if (!ReadMemory(pointer,&header,sizeof(header)) || !header.id || !header.definition ||
        !ReadMemory(header.definition,&definition,sizeof(definition)) || !definition.name ||
        !definition.nameSize || definition.nameSize>128) return false;
    std::uintptr_t address{}; std::uint16_t stride{};
    if (!Component(header,125,address,stride) || stride!=sizeof(transform) ||
        !ReadMemory(address,&transform,sizeof(transform)) || !Component(header,566,address,stride) || stride!=4 ||
        !ReadMemory(address,&tracking,sizeof(tracking)) || !tracking) return false;
    name.assign(static_cast<std::size_t>(definition.nameSize),'\0');
    if (!ReadMemory(definition.name,name.data(),name.size())) return false;
    if (const auto end=name.find('\0'); end!=std::string::npos) name.resize(end);
    return !name.empty();
}
void UpdatePlacementRecipe() {
    WorldDiagnostic::Record sample{};
    if (!SnapshotRecord(placeContext,sample) || !sample.sequence || sample.sequence==placementRecipeSequence) return;
    placementRecipeSequence=sample.sequence;
    BuildingNative::Event event{}; std::memcpy(&event,&sample.cursor,sizeof(event));
    PlacementRecipe recipe{event.tracking_or_owner,event.material_feedback};
    std::copy(std::begin(event.volume_min),std::end(event.volume_min),recipe.bounds);
    std::copy(std::begin(event.volume_max),std::end(event.volume_max),recipe.bounds+3);
    if (recipe.tracking) placementRecipes[recipe.tracking]=recipe;
}
struct AssetRecipeScan {
    const Api* api{};
    std::unordered_map<std::uint32_t,PlacementRecipe> recipes;
};
Result CALL CollectAssetRecipe(const AssetId* asset,void* user) {
    auto& scan=*static_cast<AssetRecipeScan*>(user);
    try {
        std::size_t required{};
        auto result=scan.api->get_asset(View(AssetOwner),asset,nullptr,0,&required);
        if (result!=RESULT_OK) return result;
        if (!required || required>4*1024*1024) return RESULT_OK;
        std::string bytes(required,'\0');
        result=scan.api->get_asset(View(AssetOwner),asset,bytes.data(),bytes.size(),&required);
        if (result!=RESULT_OK) return result;
        const auto data=nlohmann::json::parse(bytes.begin(),bytes.begin()+required);
        if (!data.contains("itemId") || !data["itemId"].contains("value") ||
            !data.contains("equipment") || !data["equipment"].is_object()) return RESULT_OK;
        const auto tracking=data["itemId"]["value"].get<std::uint32_t>();
        const auto& equipment=data["equipment"];
        if (!tracking || !equipment.contains("placedEntity") || equipment["placedEntity"].is_null() ||
            !equipment.contains("placementAABBmin") || !equipment.contains("placementAABBmax")) return RESULT_OK;
        PlacementRecipe recipe{tracking};
        const auto read=[&](const char* endpoint,const char* axis) {
            return equipment.at(endpoint).at(axis).get<float>();
        };
        recipe.bounds[0]=read("placementAABBmin","x"); recipe.bounds[1]=read("placementAABBmin","y");
        recipe.bounds[2]=read("placementAABBmin","z"); recipe.bounds[3]=read("placementAABBmax","x");
        recipe.bounds[4]=read("placementAABBmax","y"); recipe.bounds[5]=read("placementAABBmax","z");
        if (equipment.contains("placementColliders") && equipment["placementColliders"].is_array()) {
            for (const auto& collider:equipment["placementColliders"]) {
                if (!collider.contains("dataArray") || !collider["dataArray"].is_array()) continue;
                for (const auto& entry:collider["dataArray"]) {
                    if (!entry.contains("$value")) continue;
                    const auto& value=entry["$value"];
                    if (value.contains("materialFeedbackId") && value["materialFeedbackId"].contains("value")) {
                        recipe.feedback=value["materialFeedbackId"]["value"].get<std::uint32_t>();
                        if (recipe.feedback) break;
                    }
                }
                if (recipe.feedback) break;
            }
        }
        if (!recipe.feedback || !std::all_of(std::begin(recipe.bounds),std::end(recipe.bounds),
            [](float value){ return std::isfinite(value); })) return RESULT_OK;
        scan.recipes[tracking]=recipe;
        return RESULT_OK;
    } catch (...) { return RESULT_OK; }
}
void LoadAssetRecipes() {
    assetRecipesAttempted=true;
    if (!hostApi || !hostApi->list_assets || !hostApi->get_asset) return;
    AssetRecipeScan scan{hostApi};
    const auto result=hostApi->list_assets(View(AssetOwner),View("keen::ItemInfo"),CollectAssetRecipe,&scan);
    if (result!=RESULT_OK) {
        Utils::Log(LOG_WARNING,"World entity recipe catalog unavailable: result=%d.",static_cast<int>(result));
        return;
    }
    std::scoped_lock lock(entityMutex);
    for (const auto& [tracking,recipe]:scan.recipes) placementRecipes.try_emplace(tracking,recipe);
    Utils::Log(LOG_INFO,"World entity recipe catalog ready: %zu placeable item types.",scan.recipes.size());
}
std::string EncodeTemplate(const PlacementRecipe& recipe,const EntityDefinition& definition,const std::string& name) {
    char text[512]{};
    std::uint32_t bits[6]{}; std::memcpy(bits,recipe.bounds,sizeof(bits));
    const auto size=std::snprintf(text,sizeof(text),
        "bau1:%08x:%08x:%08x:%08x:%08x:%08x:%08x:%08x:%016llx:%016llx:%s",
        recipe.tracking,recipe.feedback,bits[0],bits[1],bits[2],bits[3],bits[4],bits[5],
        static_cast<unsigned long long>(definition.uuid[0]),static_cast<unsigned long long>(definition.uuid[1]),name.c_str());
    return size>0 && static_cast<std::size_t>(size)<sizeof(text) ? std::string(text,static_cast<std::size_t>(size)) : std::string{};
}
bool Hex(std::string_view text,std::uint64_t& value) {
    return !text.empty() && std::from_chars(text.data(),text.data()+text.size(),value,16).ec==std::errc{};
}
bool DecodeTemplate(StringView input,PlacementRecipe& recipe,std::string& name) {
    if (!input.data || input.size<5) return false;
    std::string_view text(input.data,input.size);
    if (!text.starts_with("bau1:")) return false;
    std::array<std::string_view,10> fields{}; std::size_t start=5;
    for (auto& field:fields) {
        const auto end=text.find(':',start); if (end==std::string_view::npos) return false;
        field=text.substr(start,end-start); start=end+1;
    }
    std::uint64_t value{};
    if (!Hex(fields[0],value) || value>UINT32_MAX) return false; recipe.tracking=static_cast<std::uint32_t>(value);
    if (!Hex(fields[1],value) || value>UINT32_MAX) return false; recipe.feedback=static_cast<std::uint32_t>(value);
    std::uint32_t bits[6]{};
    for (std::size_t i=0;i<6;++i) { if (!Hex(fields[i+2],value) || value>UINT32_MAX) return false; bits[i]=static_cast<std::uint32_t>(value); }
    std::memcpy(recipe.bounds,bits,sizeof(bits));
    if (!Hex(fields[8],recipe.templateUuid[0]) || !Hex(fields[9],recipe.templateUuid[1]) ||
        !(recipe.templateUuid[0] || recipe.templateUuid[1])) return false;
    name.assign(text.substr(start));
    return recipe.tracking && !name.empty();
}
bool EncodeTransform(const WorldTransform& input,WorldNative::Transform& output) {
    const double position[]{input.position.x,input.position.y,input.position.z};
    for (int i=0;i<3;++i) {
        if (!std::isfinite(position[i])) return false;
        const auto scaled=std::ldexp(position[i],32);
        if (scaled<static_cast<double>(INT64_MIN) || scaled>static_cast<double>(INT64_MAX)) return false;
        output.position[i]=static_cast<std::int64_t>(std::llround(scaled));
    }
    const double rotation[]{input.rotation.x,input.rotation.y,input.rotation.z,input.rotation.w};
    const double scale[]{input.scale.x,input.scale.y,input.scale.z};
    for (int i=0;i<4;++i) { if (!std::isfinite(rotation[i])) return false; output.rotation[i]=static_cast<float>(rotation[i]); }
    for (int i=0;i<3;++i) { if (!std::isfinite(scale[i])) return false; output.scale[i]=static_cast<float>(scale[i]); }
    return true;
}
bool SnapshotRecord(WorldDiagnostic::Record& source,WorldDiagnostic::Record& output);
Result InstallCapture(WorldDiagnostic::Record& record,WorldDiagnostic::Kind kind,const char* signature,
                      std::span<const std::uint8_t> original,RuntimePatch& patch,bool replayBefore=false,
                      void* callback=nullptr,bool entryCallback=false) {
    auto payload=WorldDiagnostic::Payload(&record,kind);
    if (callback) {
        const auto invoke=entryCallback ? WorldDiagnostic::EntryCallback(callback) : WorldDiagnostic::ActorCallback(callback);
        payload.insert(payload.end(),invoke.begin(),invoke.end());
    }
    payload.insert(replayBefore ? payload.begin() : payload.end(),original.begin(),original.end());
    payload.push_back(0xe9); const auto returnOffset=payload.size(); payload.insert(payload.end(),4,0);
    const RuntimeRelocation relocation{sizeof(relocation),returnOffset,RUNTIME_RELOCATION_REL32_RETURN};
    const RuntimePatchOptions options{sizeof(options),View(signature),0,RUNTIME_PATCH_DETOUR,
        original.size(),payload.data(),payload.size(),&relocation,1};
    auto result=RuntimePatches::Create(Owner,&options,&patch);
    if (result==RESULT_OK) result=RuntimePatches::SetEnabled(Owner,patch,true);
    return result;
}
bool ResolveWorld(const std::uint8_t* base,std::uintptr_t& world) {
    const auto valid=[](std::uintptr_t candidate) {
        std::uintptr_t voxelStore{};
        return candidate && ReadPointer(candidate+0x1210,voxelStore) && voxelStore;
    };
    // A successful public grid read identifies the editable world more
    // reliably than the actor hook: that hook also observes preview/layer
    // worlds. Keep using the confirmed world for the rest of this process.
    const auto preferred=preferredVoxelWorld.load(std::memory_order_acquire);
    if (valid(preferred)) { world=preferred; return true; }
    if (preferred) preferredVoxelWorld.store(0,std::memory_order_release);
    WorldDiagnostic::Record sample{};
    if (SnapshotRecord(actorWorldContext,sample) && sample.sequence && sample.arguments[5]) {
        const auto candidate=static_cast<std::uintptr_t>(sample.arguments[5]);
        if (valid(candidate)) { world=candidate; return true; }
    }
    if (SnapshotRecord(actorPlacementContext,sample) && sample.sequence && sample.arguments[5]) {
        const auto candidate=static_cast<std::uintptr_t>(sample.arguments[5]);
        if (valid(candidate)) { world=candidate; return true; }
    }
    // Region reads also run on temporary/preview worlds. Never promote their
    // last argument to the editable world, or readback merely verifies a copy.
    if (SnapshotRecord(worldContext,sample) && sample.sequence && sample.arguments[0]) {
        const auto candidate=static_cast<std::uintptr_t>(sample.arguments[0]);
        if (valid(candidate)) { world=candidate; return true; }
    }
    std::uintptr_t singleton{},context{};
    if (!ReadPointer(reinterpret_cast<std::uintptr_t>(base)+ActiveContextRva,singleton) || !singleton ||
        !ReadPointer(singleton+0xc8,context) || !context ||
        context<static_cast<std::uintptr_t>(-VoxelWorldFromContext)) return false;
    world=context+VoxelWorldFromContext;
    if (!valid(world)) return false;
    return true;
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
bool SnapshotRecord(WorldDiagnostic::Record& source,WorldDiagnostic::Record& output) {
    if (InterlockedCompareExchange(&source.lock,1,0)!=0) return false;
    std::memcpy(&output,&source,sizeof(output));
    InterlockedExchange(&source.lock,0);
    return true;
}
bool Snapshot(WorldDiagnostic::Record& output) { return SnapshotRecord(cursor,output); }
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
bool InBounds(const WorldVec3& value,const WorldBounds& bounds) {
    return value.x>=bounds.minimum.x && value.y>=bounds.minimum.y && value.z>=bounds.minimum.z &&
        value.x<bounds.maximum.x && value.y<bounds.maximum.y && value.z<bounds.maximum.z;
}
bool EntityIntersectsBounds(const WorldTransform& transform,const PlacementRecipe& recipe,const WorldBounds& bounds) {
    const double localCenter[]{
        (recipe.bounds[0]+recipe.bounds[3])*.5,
        (recipe.bounds[1]+recipe.bounds[4])*.5,
        (recipe.bounds[2]+recipe.bounds[5])*.5};
    const double localHalf[]{
        std::abs((recipe.bounds[3]-recipe.bounds[0])*.5*transform.scale.x),
        std::abs((recipe.bounds[4]-recipe.bounds[1])*.5*transform.scale.y),
        std::abs((recipe.bounds[5]-recipe.bounds[2])*.5*transform.scale.z)};
    const double x=transform.rotation.x,y=transform.rotation.y,z=transform.rotation.z,w=transform.rotation.w;
    const double norm=x*x+y*y+z*z+w*w;
    if (!std::isfinite(norm) || norm<1e-12) return InBounds(transform.position,bounds);
    const double s=2.0/norm;
    const double rotation[3][3]{
        {1-s*(y*y+z*z),s*(x*y-z*w),s*(x*z+y*w)},
        {s*(x*y+z*w),1-s*(x*x+z*z),s*(y*z-x*w)},
        {s*(x*z-y*w),s*(y*z+x*w),1-s*(x*x+y*y)}};
    const double scaledCenter[]{localCenter[0]*transform.scale.x,localCenter[1]*transform.scale.y,localCenter[2]*transform.scale.z};
    double center[3]{transform.position.x,transform.position.y,transform.position.z},half[3]{};
    for (int row=0;row<3;++row) {
        for (int column=0;column<3;++column) {
            center[row]+=rotation[row][column]*scaledCenter[column];
            half[row]+=std::abs(rotation[row][column])*localHalf[column];
        }
    }
    // Some placeables expose a point-sized placement AABB. Treat one voxel as
    // their selection footprint so a pivot slightly below the ground still
    // belongs to the visibly selected cell.
    if (half[0]+half[1]+half[2]<1e-6) half[0]=half[1]=half[2]=.25;
    return center[0]+half[0]>=bounds.minimum.x && center[0]-half[0]<bounds.maximum.x &&
        center[1]+half[1]>=bounds.minimum.y && center[1]-half[1]<bounds.maximum.y &&
        center[2]+half[2]>=bounds.minimum.z && center[2]-half[2]<bounds.maximum.z;
}
Result CALL Query(StringView owner,const WorldBounds* bounds,WorldEntity* output,std::size_t capacity,std::size_t* actual) {
    if ((owner.size && !owner.data) || !owner.size || !bounds || bounds->struct_size<sizeof(*bounds) || !actual ||
        (!output && capacity)) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(entityMutex);
    UpdatePlacementRecipe();
    std::vector<std::uintptr_t> pointers;
    if (!EntityPointers(pointers)) {
        Utils::Log(LOG_WARNING,"World entity query unavailable: no valid entity-manager snapshot.");
        return RESULT_NOT_AVAILABLE;
    }
    struct Found { EntityHeader header; EntityDefinition definition; WorldTransform transform; PlacementRecipe recipe; std::string name; };
    std::vector<Found> found;
    std::unordered_set<std::uint32_t> seen;
    std::size_t readable{},withRecipe{},outside{};
    for (const auto pointer:pointers) {
        EntityHeader header{}; EntityDefinition definition{}; WorldNative::Transform native{}; std::uint32_t tracking{}; std::string name;
        if (!pointer || !ReadEntity(pointer,header,native,tracking,definition,name) || !seen.insert(header.id).second) continue;
        ++readable;
        const auto recipe=placementRecipes.find(tracking);
        if (recipe==placementRecipes.end()) continue;
        ++withRecipe;
        WorldTransform transform{sizeof(transform)};
        if (!WorldNative::DecodeTransform(native,transform) || !EntityIntersectsBounds(transform,recipe->second,*bounds)) {
            ++outside; continue;
        }
        found.push_back({header,definition,transform,recipe->second,std::move(name)});
    }
    *actual=found.size();
    Utils::Log(LOG_DEBUG,"World entity query: candidates=%zu readable=%zu placeable=%zu outside=%zu matched=%zu recipes=%zu capacity=%zu.",
        pointers.size(),readable,withRecipe,outside,found.size(),placementRecipes.size(),capacity);
    if (!output) return RESULT_OK;
    if (capacity<found.size()) return RESULT_INVALID_ARGUMENT;
    entityTemplateIds.clear(); entityTemplateIds.reserve(found.size());
    for (std::size_t i=0;i<found.size();++i) {
        if (output[i].struct_size<sizeof(WorldEntity)) return RESULT_INVALID_ARGUMENT;
        entityTemplateIds.push_back(EncodeTemplate(found[i].recipe,found[i].definition,found[i].name));
        if (entityTemplateIds.back().empty()) return RESULT_INTERNAL_ERROR;
        output[i]={sizeof(WorldEntity),{found[i].header.id,0,0},View(entityTemplateIds.back().c_str()),
            found[i].transform,WorldEntityKind::Prop};
    }
    return RESULT_OK;
}
Result FindEntity(WorldEntityHandle handle,EntityHeader& header,WorldNative::Transform& transform,
                  std::uint32_t& tracking,EntityDefinition& definition,std::string& name) {
    if (!handle.id || handle.id>UINT32_MAX) return RESULT_INVALID_ARGUMENT;
    std::vector<std::uintptr_t> pointers;
    if (!EntityPointers(pointers)) return RESULT_NOT_AVAILABLE;
    std::unordered_set<std::uintptr_t> seen;
    for (const auto pointer:pointers) {
        if (!pointer || !seen.insert(pointer).second || !ReadMemory(pointer,&header,sizeof(header)) || header.id!=handle.id) continue;
        return ReadEntity(pointer,header,transform,tracking,definition,name) ? RESULT_OK : RESULT_NOT_FOUND;
    }
    return RESULT_NOT_FOUND;
}
Result CALL GetTransform(StringView owner,WorldEntityHandle handle,WorldTransform* output) {
    if ((owner.size && !owner.data) || !owner.size || !output || output->struct_size<sizeof(*output)) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(entityMutex);
    EntityHeader header{}; EntityDefinition definition{}; WorldNative::Transform native{}; std::uint32_t tracking{}; std::string name;
    const auto result=FindEntity(handle,header,native,tracking,definition,name);
    return result==RESULT_OK && WorldNative::DecodeTransform(native,*output) ? RESULT_OK : result==RESULT_OK ? RESULT_INTERNAL_ERROR : result;
}
using NativePlace=void (CALL*)(void*,const WorldNative::Transform*,const float*,std::uint32_t,std::uint32_t);
using NativeRemove=void (CALL*)(void*,const WorldNative::Transform*,const float*,std::uint32_t);
using NativeFinishBuilding=void (CALL*)(void*,void*,std::uint32_t,bool);
bool SafePlace(NativePlace function,void* context,const WorldNative::Transform* transform,const float* bounds,
               std::uint32_t tracking,std::uint32_t feedback) {
    __try { function(context,transform,bounds,tracking,feedback); return true; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool SafeRemove(NativeRemove function,void* context,const WorldNative::Transform* transform,const float* bounds,
                std::uint32_t tracking);
struct NativePlacementContext {
    std::uintptr_t root{};
    std::uint8_t beforePlaceQueue[0xa8]{};
    std::uintptr_t placeQueue{};
    std::uint8_t beforeRemoveQueue[8]{};
    std::uintptr_t removeQueue{};
    std::uint8_t beforeOwner[0x6c]{};
    std::uint32_t owner{};
};
static_assert(offsetof(NativePlacementContext,placeQueue)==0xb0 &&
              offsetof(NativePlacementContext,removeQueue)==0xc0 &&
              offsetof(NativePlacementContext,owner)==0x134);
struct PendingPlacement {
    volatile LONG phase{}; // 0 idle, 1 ready, 2 executing on engine thread, 3 complete
    WorldNative::Transform transform{};
    PlacementRecipe recipe{};
    bool remove{};
    Result result{RESULT_NOT_AVAILABLE};
    DWORD thread{};
    std::uint32_t actorIndex{};
    std::uint32_t createdId{}; // Deferred creation token, NOT a live entity handle.
    std::uintptr_t world{},placeQueue{},removeQueue{};
} pendingPlacement;
// 3E2500 calls 3E1120 to create the entity BEFORE emitting the 3EBB70
// feedback event. Emitting that event alone only produces building effects.
using NativeCreateProp=std::uint32_t (CALL*)(void*,const std::uint64_t*,const float*,const float*,
                                           std::uint32_t,std::uint32_t,const std::uint64_t*);
bool SafeCreateProp(void* executionView,const PlacementRecipe& recipe,const WorldNative::Transform& transform,
                    std::uint32_t& createdId) {
    const auto base=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    std::uintptr_t context[1]{reinterpret_cast<std::uintptr_t>(executionView)};
    float position[4]{};
    for (int i=0;i<3;++i) position[i]=static_cast<float>(std::ldexp(static_cast<double>(transform.position[i]),-32));
    const std::uint64_t auxiliary[2]{};
    __try {
        createdId=reinterpret_cast<NativeCreateProp>(base+0x3e1120)(context,recipe.templateUuid,position,
            transform.rotation,recipe.tracking,0,auxiliary);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
void CALL ActorCreateTick(void* executionView,void* actorFrame) {
    if (InterlockedCompareExchange(&pendingPlacement.phase,0,0)!=1 || pendingPlacement.remove) return;
    std::uintptr_t serviceView{},world{};
    const auto frame=reinterpret_cast<std::uintptr_t>(actorFrame);
    // Run within the same prop update system as normal placement, before its
    // input-event filter. A voxel-system command buffer is not interchangeable.
    if (!ReadPointer(frame+0x20,serviceView) || !serviceView || !ReadPointer(serviceView+8,world) || !world) return;
    std::uintptr_t root{},owners{}; std::uint32_t row{},owner{};
    const auto view=reinterpret_cast<std::uintptr_t>(executionView);
    if (!ReadPointer(view,root) || !root || !ReadMemory(view+8,&row,sizeof(row)) || !row || row>100000 ||
        !ReadPointer(root+0x7e0,owners) || !owners || !ReadMemory(owners+(row-1)*16,&owner,sizeof(owner)) || !owner) return;
    if (InterlockedCompareExchange(&pendingPlacement.phase,2,1)!=1) return;
    pendingPlacement.createdId=0;
    pendingPlacement.result=SafeCreateProp(executionView,pendingPlacement.recipe,pendingPlacement.transform,
        pendingPlacement.createdId) ? (pendingPlacement.createdId ? RESULT_OK : RESULT_NOT_AVAILABLE) : RESULT_INTERNAL_ERROR;
    pendingPlacement.thread=GetCurrentThreadId();
    pendingPlacement.actorIndex=owner;
    pendingPlacement.world=world;
    InterlockedExchange(&pendingPlacement.phase,3);
}
void CALL ActorPlacementTick(void* executionView,void* actorFrame) {
    // Creation is handled by the continuous actor update, with a real entity
    // factory call; the event-only path must never claim a creation request.
    if (!pendingPlacement.remove) return;
    const auto frame=reinterpret_cast<std::uintptr_t>(actorFrame);
    std::uintptr_t serviceView{},actorWorld{};
    if (!ReadPointer(frame+0x20,serviceView) || !serviceView ||
        !ReadPointer(serviceView+8,actorWorld) || !actorWorld) return;
    const auto preferred=preferredVoxelWorld.load(std::memory_order_acquire);
    if (preferred && actorWorld!=preferred) return;
    if (InterlockedCompareExchange(&pendingPlacement.phase,2,1)!=1) return;
    // 282965 is reached after the game has built its complete 0x13a-byte prop
    // context at frame+260, including the publish state at +D8/+E0.
    auto* const native=reinterpret_cast<NativePlacementContext*>(frame+0x260);
    std::uintptr_t placeQueue{},removeQueue{}; std::uint32_t index{};
    auto result=RESULT_NOT_AVAILABLE;
    const bool queueReady=pendingPlacement.remove
        ? ReadPointer(reinterpret_cast<std::uintptr_t>(native)+0xc0,removeQueue) && removeQueue
        : ReadPointer(reinterpret_cast<std::uintptr_t>(native)+0xb0,placeQueue) && placeQueue;
    std::uintptr_t root{},publishState{},publishCommands{};
    ReadPointer(reinterpret_cast<std::uintptr_t>(native),root);
    ReadPointer(reinterpret_cast<std::uintptr_t>(native)+0xd8,publishState);
    ReadPointer(reinterpret_cast<std::uintptr_t>(native)+0xe0,publishCommands);
    if (root==reinterpret_cast<std::uintptr_t>(executionView) && queueReady && publishState && publishCommands &&
        ReadMemory(reinterpret_cast<std::uintptr_t>(native)+0x134,&index,sizeof(index)) && index) {
        const auto base=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
        const WorldNative::PlacementBounds bounds(pendingPlacement.recipe.bounds);
        if (pendingPlacement.remove)
            result=SafeRemove(reinterpret_cast<NativeRemove>(base+0x3ebcb0),native,&pendingPlacement.transform,
                bounds.minimum,pendingPlacement.recipe.feedback) ? RESULT_OK : RESULT_INTERNAL_ERROR;
        else
            result=SafePlace(reinterpret_cast<NativePlace>(base+0x3ebb70),native,&pendingPlacement.transform,
                bounds.minimum,pendingPlacement.recipe.feedback,pendingPlacement.recipe.tracking) ? RESULT_OK : RESULT_INTERNAL_ERROR;
        if (result==RESULT_OK) {
            std::uint32_t buildingEvent{};
            ReadMemory(base+0x1402264,&buildingEvent,sizeof(buildingEvent));
            __try {
                reinterpret_cast<NativeFinishBuilding>(base+0x3e77f0)(native,nullptr,buildingEvent,true);
            } __except(EXCEPTION_EXECUTE_HANDLER) { result=RESULT_INTERNAL_ERROR; }
        }
    }
    pendingPlacement.result=result;
    pendingPlacement.thread=GetCurrentThreadId();
    pendingPlacement.actorIndex=index;
    pendingPlacement.world=actorWorld;
    pendingPlacement.placeQueue=placeQueue;
    pendingPlacement.removeQueue=removeQueue;
    InterlockedExchange(&pendingPlacement.phase,3);
}
void CALL BuildingDispatchTick(void* context,void*) {
    if (InterlockedCompareExchange(&pendingPlacement.phase,2,1)!=1) return;
    const auto native=reinterpret_cast<std::uintptr_t>(context);
    std::uintptr_t placeQueue{},removeQueue{}; std::uint32_t owner{};
    ReadPointer(native+0xb0,placeQueue);
    ReadPointer(native+0xc0,removeQueue);
    ReadMemory(native+0x134,&owner,sizeof(owner));
    auto result=RESULT_NOT_AVAILABLE;
    const auto queue=pendingPlacement.remove ? removeQueue : placeQueue;
    if (native && queue && owner) {
        const auto base=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
        const WorldNative::PlacementBounds bounds(pendingPlacement.recipe.bounds);
        if (pendingPlacement.remove)
            result=SafeRemove(reinterpret_cast<NativeRemove>(base+0x3ebcb0),context,&pendingPlacement.transform,
                bounds.minimum,pendingPlacement.recipe.feedback) ? RESULT_OK : RESULT_INTERNAL_ERROR;
        else
            result=SafePlace(reinterpret_cast<NativePlace>(base+0x3ebb70),context,&pendingPlacement.transform,
                bounds.minimum,pendingPlacement.recipe.feedback,pendingPlacement.recipe.tracking) ? RESULT_OK : RESULT_INTERNAL_ERROR;
    }
    pendingPlacement.result=result;
    pendingPlacement.thread=GetCurrentThreadId();
    pendingPlacement.actorIndex=owner;
    pendingPlacement.world=preferredVoxelWorld.load(std::memory_order_acquire);
    pendingPlacement.placeQueue=placeQueue;
    pendingPlacement.removeQueue=removeQueue;
    InterlockedExchange(&pendingPlacement.phase,3);
}
Result QueuePlacement(const WorldNative::Transform& transform,const PlacementRecipe& recipe) {
    pendingPlacement.transform=transform;
    pendingPlacement.recipe=recipe;
    pendingPlacement.remove=false;
    pendingPlacement.result=RESULT_NOT_AVAILABLE;
    pendingPlacement.thread=0;
    pendingPlacement.createdId=0;
    pendingPlacement.actorIndex=0; pendingPlacement.world=0;
    pendingPlacement.placeQueue=0; pendingPlacement.removeQueue=0;
    InterlockedExchange(&pendingPlacement.phase,1);
    const auto deadline=GetTickCount64()+3000;
    for (;;) {
        const auto phase=InterlockedCompareExchange(&pendingPlacement.phase,0,0);
        if (phase==3) {
            const auto result=pendingPlacement.result;
            Utils::Log(result==RESULT_OK ? LOG_INFO : LOG_WARNING,
                "Native prop command queued: token=%u result=%d template=%016llx:%016llx; live entity verification pending.",
                pendingPlacement.createdId,static_cast<int>(result),
                static_cast<unsigned long long>(recipe.templateUuid[0]),static_cast<unsigned long long>(recipe.templateUuid[1]));
            Utils::Log(result==RESULT_OK ? LOG_INFO : LOG_WARNING,
                "World entity placement dispatched: thread=%lu actor=%u world=%p queue=%p result=%d tracking=%08x feedback=%08x.",
                pendingPlacement.thread,pendingPlacement.actorIndex,reinterpret_cast<void*>(pendingPlacement.world),
                reinterpret_cast<void*>(pendingPlacement.placeQueue),static_cast<int>(result),recipe.tracking,recipe.feedback);
            InterlockedExchange(&pendingPlacement.phase,0);
            return result;
        }
        if (GetTickCount64()>=deadline && InterlockedCompareExchange(&pendingPlacement.phase,0,1)==1) {
            Utils::Log(LOG_WARNING,"World entity placement cancelled: building update did not consume request.");
            return RESULT_NOT_AVAILABLE;
        }
        Sleep(1);
    }
}
Result QueueRemoval(const WorldNative::Transform& transform,const PlacementRecipe& recipe) {
    pendingPlacement.transform=transform;
    pendingPlacement.recipe=recipe;
    pendingPlacement.remove=true;
    pendingPlacement.result=RESULT_NOT_AVAILABLE;
    pendingPlacement.thread=0;
    pendingPlacement.actorIndex=0; pendingPlacement.world=0;
    pendingPlacement.placeQueue=0; pendingPlacement.removeQueue=0;
    InterlockedExchange(&pendingPlacement.phase,1);
    const auto deadline=GetTickCount64()+3000;
    for (;;) {
        const auto phase=InterlockedCompareExchange(&pendingPlacement.phase,0,0);
        if (phase==3) {
            const auto result=pendingPlacement.result;
            Utils::Log(result==RESULT_OK ? LOG_INFO : LOG_WARNING,
                "World entity removal dispatched: thread=%lu actor=%u world=%p queue=%p result=%d tracking=%08x feedback=%08x.",
                pendingPlacement.thread,pendingPlacement.actorIndex,reinterpret_cast<void*>(pendingPlacement.world),
                reinterpret_cast<void*>(pendingPlacement.removeQueue),static_cast<int>(result),recipe.tracking,recipe.feedback);
            InterlockedExchange(&pendingPlacement.phase,0);
            return result;
        }
        if (GetTickCount64()>=deadline && InterlockedCompareExchange(&pendingPlacement.phase,0,1)==1) {
            Utils::Log(LOG_WARNING,"World entity removal cancelled: building update did not consume request.");
            return RESULT_NOT_AVAILABLE;
        }
        Sleep(1);
    }
}
bool SafeRemove(NativeRemove function,void* context,const WorldNative::Transform* transform,const float* bounds,std::uint32_t tracking) {
    __try { function(context,transform,bounds,tracking); return true; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
Result CALL Spawn(StringView owner,StringView templateId,const WorldTransform* input,WorldEntityHandle* output) {
    if ((owner.size && !owner.data) || !owner.size || !input || input->struct_size<sizeof(*input) || !output)
        return RESULT_INVALID_ARGUMENT;
    PlacementRecipe recipe{}; std::string name;
    if (!DecodeTemplate(templateId,recipe,name)) return RESULT_INVALID_ARGUMENT;
    if (input->scale.x!=1 || input->scale.y!=1 || input->scale.z!=1) {
        Utils::Log(LOG_WARNING,"Native prop creation requires unit scale; refusing an incorrectly scaled copy.");
        return RESULT_NOT_AVAILABLE;
    }
    WorldNative::Transform native{};
    if (!EncodeTransform(*input,native)) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(entityMutex);
    WorldDiagnostic::Record context{};
    if (!SnapshotRecord(propUpdateContext,context) || !context.sequence || !context.arguments[5]) {
        Utils::Log(LOG_WARNING,"World entity spawn unavailable: no active prop update context.");
        return RESULT_NOT_AVAILABLE;
    }
    std::unordered_set<std::uint32_t> before;
    std::vector<std::uintptr_t> pointers;
    if (!EntityPointers(pointers)) return RESULT_NOT_AVAILABLE;
    for (const auto pointer:pointers) { EntityHeader header{}; if (pointer && ReadMemory(pointer,&header,sizeof(header)) && header.id) before.insert(header.id); }
    const auto dispatched=QueuePlacement(native,recipe);
    if (dispatched!=RESULT_OK) return dispatched;
    for (int attempt=0;attempt<200;++attempt) {
        Sleep(10); pointers.clear(); if (!EntityPointers(pointers)) continue;
        std::unordered_set<std::uintptr_t> seen;
        for (const auto pointer:pointers) {
            EntityHeader header{}; EntityDefinition definition{}; WorldNative::Transform candidate{}; std::uint32_t tracking{}; std::string candidateName;
            if (!pointer || !seen.insert(pointer).second || !ReadEntity(pointer,header,candidate,tracking,definition,candidateName) ||
                before.contains(header.id) || tracking!=recipe.tracking || candidateName!=name ||
                std::llabs(candidate.position[0]-native.position[0])>(1LL<<24) ||
                std::llabs(candidate.position[1]-native.position[1])>(1LL<<24) ||
                std::llabs(candidate.position[2]-native.position[2])>(1LL<<24)) continue;
            *output={header.id,0,0}; spawnedRecipes[header.id]=recipe; placementRecipes[recipe.tracking]=recipe;
            Utils::Log(LOG_INFO,"Native prop verified in entity manager: entity=%u tracking=%08x.",header.id,tracking);
            return RESULT_OK;
        }
    }
    return RESULT_NOT_FOUND;
}
Result CALL Destroy(StringView owner,WorldEntityHandle handle) {
    if ((owner.size && !owner.data) || !owner.size || !handle.id) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(entityMutex);
    EntityHeader header{}; EntityDefinition definition{}; WorldNative::Transform native{}; std::uint32_t tracking{}; std::string name;
    auto result=FindEntity(handle,header,native,tracking,definition,name);
    if (result!=RESULT_OK) return result;
    UpdatePlacementRecipe();
    auto recipe=spawnedRecipes.find(handle.id);
    const auto known=placementRecipes.find(tracking);
    if (recipe==spawnedRecipes.end() && known==placementRecipes.end()) return RESULT_NOT_AVAILABLE;
    const auto& value=recipe!=spawnedRecipes.end() ? recipe->second : known->second;
    const auto dispatched=QueueRemoval(native,value);
    if (dispatched!=RESULT_OK) return dispatched;
    for (int attempt=0;attempt<200;++attempt) {
        Sleep(10); EntityHeader check{}; EntityDefinition def{}; WorldNative::Transform transform{}; std::uint32_t item{}; std::string label;
        if (FindEntity(handle,check,transform,item,def,label)==RESULT_NOT_FOUND) { spawnedRecipes.erase(handle.id); return RESULT_OK; }
    }
    return RESULT_INTERNAL_ERROR;
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
    if (!ResolveWorld(base,world)) {
        Utils::Log(LOG_WARNING,"Voxel read unavailable: no valid world snapshot.");
        return RESULT_NOT_AVAILABLE;
    }
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
    std::uintptr_t unset{};
    if (preferredVoxelWorld.compare_exchange_strong(unset,world,std::memory_order_acq_rel))
        Utils::Log(LOG_INFO,"Editable voxel world pinned after successful grid read: %p.",reinterpret_cast<void*>(world));
    for (std::size_t i=0;i<count;++i) {
        values[i]=WorldNative::EncodeCell(cells[i]);
        states[i]=values[i] ? WorldCellState::Occupied : WorldCellState::Empty;
    }
    const auto occupied=std::count_if(values,values+count,[](auto value){ return value!=0; });
    Utils::Log(LOG_INFO,"Voxel capture: world=%p origin=%d,%d,%d dimensions=%ux%ux%u occupied=%zu total=%zu.",
        reinterpret_cast<void*>(world),minimum[0],minimum[1],minimum[2],dimensions[0],dimensions[1],dimensions[2],
        static_cast<std::size_t>(occupied),count);
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
            !AlignChunk(minimum[i],maximum[i],expandedMinimum[i],expandedMaximum[i])) {
            Utils::Log(LOG_WARNING,"Voxel write rejected: invalid or unaligned bounds on axis %d.",i);
            return RESULT_INVALID_ARGUMENT;
        }
        const auto extent=static_cast<std::uint64_t>(static_cast<std::int64_t>(maximum[i])-minimum[i]);
        const auto expanded=static_cast<std::uint64_t>(static_cast<std::int64_t>(expandedMaximum[i])-expandedMinimum[i]);
        if (extent>SIZE_MAX/expected || expanded>MaximumWriteCells/expandedCount) {
            Utils::Log(LOG_WARNING,"Voxel write rejected: expanded region exceeds %zu cells on axis %d.",MaximumWriteCells,i);
            return RESULT_INVALID_ARGUMENT;
        }
        dimensions[i]=static_cast<std::uint32_t>(extent); expected*=dimensions[i];
        expandedDimensions[i]=static_cast<std::uint32_t>(expanded); expandedCount*=expandedDimensions[i];
    }
    Utils::Log(LOG_DEBUG,"Voxel write request: cells=%zu expected=%zu dimensions=%ux%ux%u expanded=%ux%ux%u (%zu).",
        count,expected,dimensions[0],dimensions[1],dimensions[2],
        expandedDimensions[0],expandedDimensions[1],expandedDimensions[2],expandedCount);
    if (count!=expected || expandedCount>MaximumWriteCells) {
        Utils::Log(LOG_WARNING,"Voxel write rejected: cell count mismatch or expanded limit exceeded.");
        return RESULT_INVALID_ARGUMENT;
    }
    for (std::size_t i=0;i<count;++i) {
        if (states[i]<WorldCellState::Unknown || states[i]>WorldCellState::Occupied || values[i]>UINT16_MAX ||
            (states[i]==WorldCellState::Occupied && values[i]==0)) {
            Utils::Log(LOG_WARNING,"Voxel write rejected: invalid cell at index %zu (state=%u value=%u).",
                i,static_cast<unsigned>(states[i]),values[i]);
            return RESULT_INVALID_ARGUMENT;
        }
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
    Utils::Log(LOG_DEBUG,"Voxel write dispatch: world=%p thread=%lu origin=%d,%d,%d.",
        reinterpret_cast<void*>(world),GetCurrentThreadId(),expandedMinimum[0],expandedMinimum[1],expandedMinimum[2]);
    if (!SafeNativeWrite(write,reinterpret_cast<void*>(world),&span,expandedDimensions,expandedMinimum)) {
        Utils::Log(LOG_ERROR,"Voxel write faulted; region may be partially changed.");
        return RESULT_INTERNAL_ERROR;
    }
    std::vector<WorldNative::TerrainCell> verified(expandedCount);
    if (!NativeReadRegion(base,world,expandedMinimum,expandedDimensions,verified)) {
        Utils::Log(LOG_ERROR,"Voxel write readback unavailable; physics and persistence are unverified.");
        return RESULT_INTERNAL_ERROR;
    }
    for (std::size_t i=0;i<expandedCount;++i) {
        if (WorldNative::EncodeCell(verified[i])!=WorldNative::EncodeCell(expanded[i])) {
            Utils::Log(LOG_ERROR,"Voxel write readback mismatch: index=%zu expected=%u actual=%u.",
                i,WorldNative::EncodeCell(expanded[i]),WorldNative::EncodeCell(verified[i]));
            return RESULT_INTERNAL_ERROR;
        }
    }
    Utils::Log(LOG_INFO,"Voxel write data verified: %zu cells including chunk border. Collision/persistence NOT verified.",expandedCount);
    return RESULT_OK;
}
WorldApi service{sizeof(service),WorldApiVersion,Query,GetTransform,Spawn,Destroy,nullptr,
                 GetGrid,ReadGrid,WriteGrid,GetCursor};
}
Result Initialize(const Api* api) {
    if (!api || serviceRegistration) return !api ? RESULT_INVALID_ARGUMENT : RESULT_CONFLICT;
    hostApi=api;
    const auto* base=reinterpret_cast<const std::uint8_t*>(GetModuleHandleW(nullptr));
    const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    const auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    (void)ReadVoxelRegionSignature; // Kept beside the ABI for offline uniqueness verification.
    (void)WriteVoxelRegionSignature;
    const std::uint8_t original[]{0x41,0x8b,0x96,0xf8,0x02,0,0};
    const std::uint8_t readerOriginal[]{0x44,0x89,0x4c,0x24,0x20,0x4c,0x89,0x44,0x24,0x18};
    const std::uint8_t writerOriginal[]{0x4c,0x8b,0xdc,0x4d,0x89,0x4b,0x20,0x4d,0x89,0x43,0x18};
    const std::uint8_t objectOriginal[]{0x48,0x81,0xc3,0x88,0,0,0};
    const std::uint8_t placeEntryOriginal[]{0x48,0x89,0x6c,0x24,0x18};
    const std::uint8_t actorWorldOriginal[]{0x48,0x8b,0x45,0x10,0x48,0x8b,0x55,0x48,0x48,0x85,0xc0};
    const std::uint8_t placeOriginal[]{0x41,0x8b,0x86,0x34,0x01,0,0,0x89,0x43,0x44};
    const std::uint8_t removeOriginal[]{0x41,0x8b,0x86,0x34,0x01,0,0,0x89,0x43,0x40};
    const std::uint8_t dispatchOriginal[]{0x48,0x89,0x5c,0x24,0x10};
    const std::uint8_t createOriginal[]{0x48,0x89,0x5c,0x24,0x20,0x55,0x56,0x57,0x41,0x56,0x41,0x57};
    if (nt->FileHeader.TimeDateStamp!=0x6a4236c8 || nt->OptionalHeader.SizeOfImage!=0x2da7000 ||
        std::memcmp(base+0x24aa1d,original,sizeof(original)) ||
        std::memcmp(base+ReadVoxelRegionRva,readerOriginal,sizeof(readerOriginal)) ||
        std::memcmp(base+WriteVoxelRegionRva,writerOriginal,sizeof(writerOriginal)) ||
        std::memcmp(base+0x23a159,objectOriginal,sizeof(objectOriginal)) ||
        std::memcmp(base+0x3ebb70,placeEntryOriginal,sizeof(placeEntryOriginal)) ||
        std::memcmp(base+0x3e77f0,dispatchOriginal,sizeof(dispatchOriginal)) ||
        std::memcmp(base+0x3e1120,createOriginal,sizeof(createOriginal)) ||
        std::memcmp(base+0x3ebc82,placeOriginal,sizeof(placeOriginal)) ||
        std::memcmp(base+0x3ebdb2,removeOriginal,sizeof(removeOriginal))) return RESULT_VERSION_MISMATCH;
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
    const std::uint8_t worldOriginal[]{0x4c,0x89,0x4c,0x24,0x20,0x4c,0x89,0x44,0x24,0x18};
    // Production capture is deliberately register-only: never dereference transient
    // writer descriptors from a permanent hook.
    auto worldPayload=WorldDiagnostic::Payload(&worldContext,WorldDiagnostic::Kind::WorldContext);
    worldPayload.insert(worldPayload.end(),std::begin(worldOriginal),std::end(worldOriginal));
    worldPayload.push_back(0xe9); const auto worldReturnOffset=worldPayload.size(); worldPayload.insert(worldPayload.end(),4,0);
    const RuntimeRelocation worldRelocation{sizeof(worldRelocation),worldReturnOffset,RUNTIME_RELOCATION_REL32_RETURN};
    constexpr char worldSignature[]="4C 89 4C 24 20 4C 89 44 24 18 48 89 54 24 10 48 89 4C 24 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 18 FF FF FF";
    const RuntimePatchOptions worldOptions{sizeof(worldOptions),View(worldSignature),0,RUNTIME_PATCH_DETOUR,
        sizeof(worldOriginal),worldPayload.data(),worldPayload.size(),&worldRelocation,1};
    result=RuntimePatches::Create(Owner,&worldOptions,&worldContextPatch);
    if (result==RESULT_OK) result=RuntimePatches::SetEnabled(Owner,worldContextPatch,true);
    if (result!=RESULT_OK) { InterlockedExchange(&cursor.enabled,0); return result; }
    if (result==RESULT_OK) result=InstallCapture(objectContext,WorldDiagnostic::Kind::ObjectLook,
        "48 81 C3 88 00 00 00 48 81 FB 80 08 00 00 0F 82 66 FE FF FF",
        objectOriginal,objectContextPatch);
    // Capture the actual placement helper's input. 3ECAF0 is a spatial query
    // with a different input layout and must never supply a placement context.
    if (result==RESULT_OK) result=InstallCapture(placeEntryContext,WorldDiagnostic::Kind::PlacementContext,
        "48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 48 8B B1 B0 00 00 00",
        placeEntryOriginal,placeEntryContextPatch);
    if (result==RESULT_OK) result=InstallCapture(actorWorldContext,WorldDiagnostic::Kind::ActorWorldContext,
        "48 8B 45 10 48 8B 55 48 48 85 C0 74 03 F0 FF 00 48 8B 4D 08 8B 49 30",
        actorWorldOriginal,actorWorldContextPatch);
    const std::uint8_t propUpdateOriginal[]{0x33,0xd2,0x48,0x8d,0x4c,0x24,0x30};
    if (result==RESULT_OK) result=InstallCapture(propUpdateContext,WorldDiagnostic::Kind::PropUpdateContext,
        "33 D2 48 8D 4C 24 30 E8 4E AA 52 00 48 8D 54 24 30 49 8B CF",
        propUpdateOriginal,propUpdateContextPatch,false,reinterpret_cast<void*>(&ActorCreateTick));
    const std::uint8_t actorOriginal[]{0x48,0x8b,0x45,0x10,0x48,0x89,0x85,0x88,0x02,0x00,0x00};
    if (result==RESULT_OK) result=InstallCapture(actorPlacementContext,WorldDiagnostic::Kind::ActorPlacementContext,
        "48 8B 45 10 48 89 85 88 02 00 00 48 8B 45 30 48 89 85 70 03 00 00",
        actorOriginal,actorPlacementContextPatch,false,reinterpret_cast<void*>(&ActorPlacementTick));
    if (result==RESULT_OK) result=InstallCapture(buildingDispatchContext,WorldDiagnostic::Kind::WorldContext,
        "48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 20 4C 8B F1 48 8B 89 D8 00 00 00",
        dispatchOriginal,buildingDispatchContextPatch);
    if (result==RESULT_OK) result=InstallCapture(placeContext,WorldDiagnostic::Kind::BuildingEvent,
        "41 8B 86 34 01 00 00 89 43 44 48 8B 5C 24 48",placeOriginal,placeContextPatch,true);
    if (result==RESULT_OK) result=InstallCapture(removeContext,WorldDiagnostic::Kind::BuildingEvent,
        "41 8B 86 34 01 00 00 89 43 40 48 8B 5C 24 48",removeOriginal,removeContextPatch,true);
    if (result!=RESULT_OK) { InterlockedExchange(&cursor.enabled,0); InterlockedExchange(&worldContext.enabled,0); return result; }
    const ServiceDescriptor descriptor{sizeof(descriptor),View(WorldServiceId),WorldServiceMajor,WorldServiceMinor,&service};
    result=api->register_service(View(Owner),&descriptor,&serviceRegistration);
    if (result!=RESULT_OK) { InterlockedExchange(&cursor.enabled,0); return result; }
    InterlockedExchange(&worldContext.enabled,1);
    InterlockedExchange(&objectContext.enabled,1);
    InterlockedExchange(&placeEntryContext.enabled,1);
    InterlockedExchange(&actorWorldContext.enabled,1);
    InterlockedExchange(&propUpdateContext.enabled,1);
    InterlockedExchange(&actorPlacementContext.enabled,1);
    InterlockedExchange(&buildingDispatchContext.enabled,1);
    InterlockedExchange(&placeContext.enabled,1);
    InterlockedExchange(&removeContext.enabled,1);
    InterlockedExchange(&cursor.enabled,1);
    Utils::Log(LOG_INFO,"Native world service ready: cursor=yes world-context=writer-hook grids=voxel-read-write (bounded chunk RMW)." );
    return RESULT_OK;
}
void Tick() {
    if (!assetRecipesAttempted) LoadAssetRecipes();
    std::scoped_lock lock(entityMutex);
    UpdatePlacementRecipe();
    if (!voxelWorldContextLogged) {
        const auto* base=reinterpret_cast<const std::uint8_t*>(GetModuleHandleW(nullptr));
        std::uintptr_t world{};
        if (ResolveWorld(base,world)) {
            voxelWorldContextLogged=true;
            Utils::Log(LOG_INFO,"Editable voxel world context ready: world=%p.",reinterpret_cast<void*>(world));
        }
    }
    if (!placementContextLogged) {
        WorldDiagnostic::Record sample{};
        if (SnapshotRecord(actorPlacementContext,sample) && sample.sequence && sample.arguments[1] &&
            sample.arguments[2] && sample.arguments[4]) {
            placementContextLogged=true;
            Utils::Log(LOG_INFO,"World entity placement context ready: root=%p placeQueue=%p removeQueue=%p owner=%u.",
                reinterpret_cast<void*>(sample.arguments[1]),reinterpret_cast<void*>(sample.arguments[2]),
                reinterpret_cast<void*>(sample.arguments[3]),static_cast<unsigned>(sample.arguments[4]));
        }
    }
    const auto report=[](const char* label,WorldDiagnostic::Record& record,std::uint64_t& last) {
        WorldDiagnostic::Record sample{};
        if (!SnapshotRecord(record,sample) || !sample.sequence || sample.sequence==last) return;
        last=sample.sequence;
        Utils::Log(LOG_DEBUG,
            "%s context sample: seq=%llu thread=%u a0=%p a1=%p a2=%p a3=%p owner=%llu world=%p.",
            label,static_cast<unsigned long long>(sample.sequence),sample.thread,
            reinterpret_cast<void*>(sample.arguments[0]),reinterpret_cast<void*>(sample.arguments[1]),
            reinterpret_cast<void*>(sample.arguments[2]),reinterpret_cast<void*>(sample.arguments[3]),
            static_cast<unsigned long long>(sample.arguments[4]),reinterpret_cast<void*>(sample.arguments[5]));
    };
    report("Actor",actorPlacementContext,loggedActorSequence);
    report("Native place entry",placeEntryContext,loggedPlaceEntrySequence);
    report("Building update",buildingDispatchContext,loggedBuildingSequence);
}
void Shutdown(const Api* api) {
    InterlockedExchange(&cursor.enabled,0);
    InterlockedExchange(&worldContext.enabled,0);
    InterlockedExchange(&objectContext.enabled,0);
    InterlockedExchange(&placeEntryContext.enabled,0);
    InterlockedExchange(&actorWorldContext.enabled,0);
    InterlockedExchange(&propUpdateContext.enabled,0);
    InterlockedExchange(&actorPlacementContext.enabled,0);
    InterlockedExchange(&buildingDispatchContext.enabled,0);
    InterlockedExchange(&placeContext.enabled,0);
    InterlockedExchange(&removeContext.enabled,0);
    if (api && serviceRegistration) api->release_registration(serviceRegistration);
    serviceRegistration=0;
    std::scoped_lock lock(entityMutex,gridMutex);
    placementRecipes.clear();
    spawnedRecipes.clear();
    placementRecipeSequence=0;
    placementContextLogged=false;
    voxelWorldContextLogged=false;
    loggedPlaceEntrySequence=loggedActorSequence=loggedBuildingSequence=0;
    cachedEntityManager=0;
    assetRecipesAttempted=false;
    hostApi=nullptr;
}
bool DiagnosticSnapshot(WorldNative::Cursor& value,std::uint64_t& sequence,
                        std::uint32_t& thread,std::uintptr_t& input) {
    WorldDiagnostic::Record sample{};
    if (!Snapshot(sample) || !sample.sequence) return false;
    value=sample.cursor; sequence=sample.sequence; thread=sample.thread;
    input=static_cast<std::uintptr_t>(sample.arguments[0]); return true;
}
bool DiagnosticWorldSnapshot(std::uintptr_t& world,std::uint64_t& sequence,
                             std::uint32_t& thread,std::array<std::uint64_t,6>& arguments,
                             WorldNative::Cursor& descriptor) {
    WorldDiagnostic::Record sample{};
    if (!SnapshotRecord(worldContext,sample) || !sample.sequence || !sample.arguments[0]) return false;
    world=static_cast<std::uintptr_t>(sample.arguments[0]); sequence=sample.sequence; thread=sample.thread;
    std::copy(std::begin(sample.arguments),std::end(sample.arguments),arguments.begin());
    descriptor=sample.cursor;
    return true;
}
bool DiagnosticObjectSnapshot(WorldDiagnostic::Record& output) {
    return SnapshotRecord(objectContext,output) && output.sequence;
}
bool DiagnosticBuildingSnapshot(bool remove,WorldDiagnostic::Record& output) {
    return SnapshotRecord(remove ? removeContext : placeContext,output) && output.sequence;
}
}
