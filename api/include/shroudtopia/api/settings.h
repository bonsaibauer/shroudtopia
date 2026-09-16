#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ST_SettingsDescriptor {
    size_t struct_size;
    ST_StringView settings_id;
    ST_StringView schema_json;
    ST_StringView defaults_json;
} ST_SettingsDescriptor;

#ifdef __cplusplus
}
#endif
