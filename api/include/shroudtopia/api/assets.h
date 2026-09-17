#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASSETS_SERVICE_ID "shroudtopia.assets"
#define ASSETS_SERVICE_VERSION_MAJOR 2u
#define ASSETS_SERVICE_VERSION_MINOR 1u

typedef struct AssetId {
    size_t struct_size;
    StringView guid;
    StringView type_name;
    uint32_t part;
} AssetId;

typedef Result (CALL* AssetVisitor)(
    const AssetId* asset, void* user_data);

typedef struct AssetsApi {
    size_t struct_size;
    Result (CALL* list)(StringView owner_id, StringView type_name,
        AssetVisitor visitor, void* user_data);
    Result (CALL* get)(StringView owner_id,
        const AssetId* asset, char* buffer, size_t capacity, size_t* required_size);
    Result (CALL* update)(StringView owner_id,
        const AssetId* asset, StringView json);
    Result (CALL* create)(StringView owner_id, StringView type_name,
        StringView json, AssetVisitor visitor, void* user_data);
    Result (CALL* reset)(StringView owner_id);
    Result (CALL* save)(StringView owner_id);
    /* Asset API 2.1. Existing JSON-pointer path, typed JSON value.
       Disjoint fields compose; overlapping owners return CONFLICT. */
    Result (CALL* set)(StringView owner_id,
        const AssetId* asset, StringView path, StringView json);
} AssetsApi;

#ifdef __cplusplus
}
#endif
