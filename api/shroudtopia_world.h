#pragma once

#include "shroudtopia.h"
#include <cstddef>
#include <cstdint>

// Public optional world contract. Game-specific addresses and layouts stay in
// the loader. Null operations are unavailable, never simulated successes.
struct WorldVec3 { double x{}, y{}, z{}; };
struct WorldQuaternion { double x{}, y{}, z{}, w{1}; };
struct WorldTransform { std::size_t struct_size{}; WorldVec3 position{}; WorldQuaternion rotation{}; WorldVec3 scale{1,1,1}; };
struct WorldBounds { std::size_t struct_size{}; WorldVec3 minimum{}, maximum{}; };
struct WorldEntityHandle { std::uint64_t id{}, generation{}, reserved{}; };
enum class WorldEntityKind : std::uint32_t { Other, Prop };
struct WorldEntity { std::size_t struct_size{}; WorldEntityHandle handle{}; StringView template_id{}; WorldTransform transform{}; WorldEntityKind kind{WorldEntityKind::Other}; };
enum class WorldCellState : std::uint32_t { Unknown, Empty, Occupied };
struct WorldGridSpec { std::size_t struct_size{}; StringView id{}; WorldVec3 origin{}, cell_size{}; std::size_t maximum_x{}, maximum_y{}, maximum_z{}; };
struct WorldGridRegion { std::size_t struct_size{}; WorldBounds bounds{}; std::int64_t minimum[3]{}; std::size_t dimensions[3]{}; std::size_t cell_count{}; };
using QueryEntities = Result (CALL*)(StringView, const WorldBounds*, WorldEntity*, std::size_t, std::size_t*);
using GetEntityTransform = Result (CALL*)(StringView, WorldEntityHandle, WorldTransform*);
using SpawnEntity = Result (CALL*)(StringView, StringView, const WorldTransform*, WorldEntityHandle*);
using DestroyEntity = Result (CALL*)(StringView, WorldEntityHandle);
using GetGridSpec = Result (CALL*)(StringView, StringView, WorldGridSpec*);
using ReadGridRegion = Result (CALL*)(StringView, StringView, const WorldBounds*, std::uint32_t*, WorldCellState*, std::size_t, std::size_t*);
using WriteGridRegion = Result (CALL*)(StringView, StringView, const WorldBounds*, const std::uint32_t*, const WorldCellState*, std::size_t);
// Runtime object identity is not a template/asset UUID.
struct GameObjectId { std::uint64_t value{}; std::uint32_t type{}; std::uint32_t reserved{}; };
struct CursorSnapshot {
    std::size_t struct_size{};
    std::uint64_t sequence{};
    WorldTransform primary{}, secondary{};
    GameObjectId selected_object{};
    std::uint8_t primary_flags{}, secondary_flags{}, material{}, attached_to_prop{};
};
using GetWorldCursor = Result (CALL*)(StringView, CursorSnapshot*);
struct WorldApi {
    std::size_t struct_size{};
    std::uint32_t version{};
    QueryEntities query_entities{};
    GetEntityTransform get_entity_transform{};
    SpawnEntity spawn_entity{};
    DestroyEntity destroy_entity{};
    void* reserved{};
    GetGridSpec get_grid_spec{};
    ReadGridRegion read_grid_region{};
    WriteGridRegion write_grid_region{};
    GetWorldCursor get_cursor{};
};
constexpr char WorldServiceId[] = "shroudtopia.world";
constexpr std::uint32_t WorldServiceMajor = 1, WorldServiceMinor = 1, WorldApiVersion = 1;
