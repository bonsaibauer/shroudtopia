#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RUNTIME_PATCHES_SERVICE_ID "shroudtopia.runtime.patches"
#define RUNTIME_PATCHES_VERSION_MAJOR 2u
#define RUNTIME_PATCHES_VERSION_MINOR 0u

typedef uint64_t RuntimePatch;

typedef enum RuntimePatchKind {
    RUNTIME_PATCH_DIRECT = 0,
    RUNTIME_PATCH_DETOUR = 1
} RuntimePatchKind;

typedef enum RuntimeRelocationKind {
    RUNTIME_RELOCATION_REL32_RETURN = 1
} RuntimeRelocationKind;

typedef struct RuntimeRelocation {
    size_t struct_size;
    size_t payload_offset;
    RuntimeRelocationKind kind;
} RuntimeRelocation;

typedef struct RuntimePatchOptions {
    size_t struct_size;
    StringView signature;
    int64_t match_offset;
    RuntimePatchKind kind;
    size_t overwrite_size;
    const uint8_t* payload;
    size_t payload_size;
    const RuntimeRelocation* relocations;
    size_t relocation_count;
} RuntimePatchOptions;

typedef struct RuntimePatchState {
    size_t struct_size;
    uint8_t enabled;
    uint8_t reserved[7];
} RuntimePatchState;

typedef struct PatchesApi {
    size_t struct_size;
    Result (CALL* create)(
        StringView owner_id, const RuntimePatchOptions* descriptor, RuntimePatch* patch);
    Result (CALL* set_enabled)(StringView owner_id, RuntimePatch patch, uint8_t enabled);
    Result (CALL* get_state)(StringView owner_id, RuntimePatch patch, RuntimePatchState* state);
    Result (CALL* release)(StringView owner_id, RuntimePatch patch);
} PatchesApi;

#ifdef __cplusplus
}
#endif
