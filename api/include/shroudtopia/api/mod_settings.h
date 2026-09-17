#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ModSettingsDescriptor {
    size_t struct_size;
    StringView mod_settings_id;
    StringView schema_json;
    StringView defaults_json;
} ModSettingsDescriptor;

#ifdef __cplusplus
}
#endif
