#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST_WORLD_SERVICE_ID "shroudtopia.world"
#define ST_WORLD_SERVICE_VERSION_MAJOR 1u
#define ST_WORLD_SERVICE_VERSION_MINOR 1u

typedef struct ST_Vec3d { double x, y, z; } ST_Vec3d;
typedef struct ST_Quaterniond { double x, y, z, w; } ST_Quaterniond;
typedef struct ST_TransformV1 {
    size_t struct_size;
    ST_Vec3d position;
    ST_Quaterniond rotation;
    ST_Vec3d scale;
} ST_TransformV1;

typedef struct ST_AabbV1 {
    size_t struct_size;
    ST_Vec3d minimum;
    ST_Vec3d maximum;
} ST_AabbV1;

typedef struct ST_EntityHandleV1 {
    uint64_t id;
    uint32_t generation;
    uint32_t session;
} ST_EntityHandleV1;

typedef enum ST_EntityKindV1 {
    ST_ENTITY_UNKNOWN = 0,
    ST_ENTITY_PROP = 1,
    ST_ENTITY_OTHER = 2
} ST_EntityKindV1;

typedef struct ST_EntitySnapshotV1 {
    size_t struct_size;
    ST_EntityHandleV1 handle;
    ST_StringView template_id;
    ST_TransformV1 transform;
    /* World 1.1: PROP must identify a supported static template instance.
       NPCs/players and unsupported dynamic objects are OTHER, never PROP. */
    ST_EntityKindV1 kind;
} ST_EntitySnapshotV1;

typedef enum ST_CoverageV1 {
    ST_COVERAGE_UNKNOWN = 0,
    ST_COVERAGE_EMPTY = 1,
    ST_COVERAGE_OCCUPIED = 2
} ST_CoverageV1;

typedef struct ST_GridSpecV1 {
    size_t struct_size;
    ST_StringView grid_id;
    ST_Vec3d origin;
    ST_Vec3d cell_size;
    uint32_t chunk_size_x;
    uint32_t chunk_size_y;
    uint32_t chunk_size_z;
} ST_GridSpecV1;

typedef struct ST_WorldApiV1 {
    size_t struct_size;
    uint32_t api_version;
    /* Regions are half-open world-space bounds. OK means complete coverage;
       an unloaded/unknown region must return an error, never a partial OK.
       Template strings remain valid until the next world API call on this thread.
       A capacity query performs no writes; callers initialize output struct sizes. */
    ST_Result (ST_CALL* query_entities)(ST_StringView owner_id, const ST_AabbV1* region,
        ST_EntitySnapshotV1* entities, size_t capacity, size_t* required_count);
    ST_Result (ST_CALL* get_entity_transform)(ST_StringView owner_id, ST_EntityHandleV1 entity, ST_TransformV1* transform);
    ST_Result (ST_CALL* spawn_entity)(ST_StringView owner_id, ST_StringView template_id,
        const ST_TransformV1* transform, ST_EntityHandleV1* entity);
    ST_Result (ST_CALL* destroy_entity)(ST_StringView owner_id, ST_EntityHandleV1 entity);
    ST_Result (ST_CALL* set_entity_transform)(ST_StringView owner_id, ST_EntityHandleV1 entity, const ST_TransformV1* transform);
    ST_Result (ST_CALL* get_grid_spec)(ST_StringView owner_id, ST_StringView grid_id, ST_GridSpecV1* grid);
    /* Grid bounds must align with grid origin/cell size. X varies fastest.
       UNKNOWN is not air. Writers leave UNKNOWN cells untouched. A failed
       write may be partial; callers must preserve recovery data. */
    ST_Result (ST_CALL* read_grid_region)(ST_StringView owner_id, ST_StringView grid_id,
        const ST_AabbV1* region, uint32_t* values, ST_CoverageV1* coverage, size_t capacity, size_t* required_count);
    ST_Result (ST_CALL* write_grid_region)(ST_StringView owner_id, ST_StringView grid_id,
        const ST_AabbV1* region, const uint32_t* values, const ST_CoverageV1* coverage, size_t count);
} ST_WorldApiV1;

#ifdef __cplusplus
}
#endif
