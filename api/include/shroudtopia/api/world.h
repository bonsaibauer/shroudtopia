#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WORLD_SERVICE_ID "shroudtopia.world"
#define WORLD_SERVICE_VERSION_MAJOR 2u
#define WORLD_SERVICE_VERSION_MINOR 1u

typedef struct Vec3d { double x, y, z; } Vec3d;
typedef struct Quaterniond { double x, y, z, w; } Quaterniond;
typedef struct Transform {
    size_t struct_size;
    Vec3d position;
    Quaterniond rotation;
    Vec3d scale;
} Transform;

typedef struct Aabb {
    size_t struct_size;
    Vec3d minimum;
    Vec3d maximum;
} Aabb;

typedef struct EntityId {
    uint64_t id;
    uint32_t generation;
    uint32_t session;
} EntityId;

typedef enum EntityKind {
    ENTITY_UNKNOWN = 0,
    ENTITY_PROP = 1,
    ENTITY_OTHER = 2
} EntityKind;

typedef struct Entity {
    size_t struct_size;
    EntityId id;
    StringView template_id;
    Transform transform;
    /* World 2.1: PROP must identify a supported static template instance.
       NPCs/players and unsupported dynamic objects are OTHER, never PROP. */
    EntityKind kind;
} Entity;

typedef enum Coverage {
    COVERAGE_UNKNOWN = 0,
    COVERAGE_EMPTY = 1,
    COVERAGE_OCCUPIED = 2
} Coverage;

typedef struct Grid {
    size_t struct_size;
    StringView id;
    Vec3d origin;
    Vec3d cell_size;
    uint32_t chunk_size_x;
    uint32_t chunk_size_y;
    uint32_t chunk_size_z;
} Grid;

typedef struct WorldApi {
    size_t struct_size;
    uint32_t api_version;
    /* Regions are half-open world-space bounds. OK means complete coverage;
       an unloaded/unknown region must return an error, never a partial OK.
       Template strings remain valid until the next world API call on this thread.
       A capacity query performs no writes; callers initialize output struct sizes. */
    Result (CALL* list_entities)(StringView owner_id, const Aabb* region,
        Entity* entities, size_t capacity, size_t* required_count);
    Result (CALL* get_entity)(StringView owner_id, EntityId entity, Transform* transform);
    Result (CALL* create_entity)(StringView owner_id, StringView template_id,
        const Transform* transform, EntityId* entity);
    Result (CALL* remove_entity)(StringView owner_id, EntityId entity);
    Result (CALL* update_entity)(StringView owner_id, EntityId entity, const Transform* transform);
    Result (CALL* get_grid)(StringView owner_id, StringView grid_id, Grid* grid);
    /* Grid bounds must align with grid origin/cell size. X varies fastest.
       UNKNOWN is not air. Writers leave UNKNOWN cells untouched. A failed
       write may be partial; callers must preserve recovery data. */
    Result (CALL* get_grid_region)(StringView owner_id, StringView grid_id,
        const Aabb* region, uint32_t* values, Coverage* coverage, size_t capacity, size_t* required_count);
    Result (CALL* update_grid_region)(StringView owner_id, StringView grid_id,
        const Aabb* region, const uint32_t* values, const Coverage* coverage, size_t count);
} WorldApi;

#ifdef __cplusplus
}
#endif
