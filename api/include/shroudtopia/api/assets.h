#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST_ASSETS_SERVICE_ID "shroudtopia.assets"
#define ST_ASSETS_SERVICE_VERSION_MAJOR 1u
#define ST_ASSETS_SERVICE_VERSION_MINOR 1u

typedef struct ST_AssetResourceKeyV1 {
    size_t struct_size;
    ST_StringView guid;
    ST_StringView type_name;
    uint32_t part;
} ST_AssetResourceKeyV1;

typedef ST_Result (ST_CALL* ST_AssetResourceVisitorV1)(
    const ST_AssetResourceKeyV1* resource, void* user_data);

typedef struct ST_AssetsApiV1 {
    size_t struct_size;
    ST_Result (ST_CALL* visit_resources)(ST_StringView owner_id, ST_StringView type_name,
        ST_AssetResourceVisitorV1 visitor, void* user_data);
    ST_Result (ST_CALL* read_resource_json)(ST_StringView owner_id,
        const ST_AssetResourceKeyV1* resource, char* buffer, size_t capacity, size_t* required_size);
    ST_Result (ST_CALL* replace_resource_json)(ST_StringView owner_id,
        const ST_AssetResourceKeyV1* resource, ST_StringView json);
    ST_Result (ST_CALL* create_resource_json)(ST_StringView owner_id, ST_StringView type_name,
        ST_StringView json, ST_AssetResourceVisitorV1 visitor, void* user_data);
    ST_Result (ST_CALL* discard_changes)(ST_StringView owner_id);
    ST_Result (ST_CALL* flush)(ST_StringView owner_id);
    /* Added in service 1.1. Existing JSON-pointer path, typed JSON value.
       Disjoint fields compose; overlapping owners return ALREADY_EXISTS. */
    ST_Result (ST_CALL* set_resource_field_json)(ST_StringView owner_id,
        const ST_AssetResourceKeyV1* resource, ST_StringView path, ST_StringView json);
} ST_AssetsApiV1;

#ifdef __cplusplus
}
#endif
