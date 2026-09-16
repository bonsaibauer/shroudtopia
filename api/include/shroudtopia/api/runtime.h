#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST_RUNTIME_PATCHES_SERVICE_ID "shroudtopia.runtime.patches"
#define ST_RUNTIME_PATCHES_VERSION_MAJOR 1u
#define ST_RUNTIME_PATCHES_VERSION_MINOR 0u

typedef uint64_t ST_RuntimePatch;

typedef enum ST_RuntimePatchKindV1 {
    ST_RUNTIME_PATCH_DIRECT = 0,
    ST_RUNTIME_PATCH_DETOUR = 1
} ST_RuntimePatchKindV1;

typedef enum ST_RuntimeRelocationKindV1 {
    ST_RUNTIME_RELOCATION_REL32_RETURN = 1
} ST_RuntimeRelocationKindV1;

typedef struct ST_RuntimeRelocationV1 {
    size_t struct_size;
    size_t payload_offset;
    ST_RuntimeRelocationKindV1 kind;
} ST_RuntimeRelocationV1;

typedef struct ST_RuntimePatchDescriptorV1 {
    size_t struct_size;
    ST_StringView signature;
    int64_t match_offset;
    ST_RuntimePatchKindV1 kind;
    size_t overwrite_size;
    const uint8_t* payload;
    size_t payload_size;
    const ST_RuntimeRelocationV1* relocations;
    size_t relocation_count;
} ST_RuntimePatchDescriptorV1;

typedef struct ST_RuntimePatchStateV1 {
    size_t struct_size;
    uint8_t enabled;
    uint8_t reserved[7];
} ST_RuntimePatchStateV1;

typedef struct ST_RuntimePatchesApiV1 {
    size_t struct_size;
    ST_Result (ST_CALL* create)(
        ST_StringView owner_id, const ST_RuntimePatchDescriptorV1* descriptor, ST_RuntimePatch* patch);
    ST_Result (ST_CALL* set_enabled)(ST_StringView owner_id, ST_RuntimePatch patch, uint8_t enabled);
    ST_Result (ST_CALL* get_state)(ST_StringView owner_id, ST_RuntimePatch patch, ST_RuntimePatchStateV1* state);
    ST_Result (ST_CALL* release)(ST_StringView owner_id, ST_RuntimePatch patch);
} ST_RuntimePatchesApiV1;

#ifdef __cplusplus
}
#endif
