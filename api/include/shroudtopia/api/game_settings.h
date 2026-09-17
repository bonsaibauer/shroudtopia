#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GameSetting {
    size_t struct_size;
    StringView key;
    StringView effective_json;
    StringView pending_json;
    uint8_t has_pending_value;
    uint8_t reserved[7];
} GameSetting;

#ifdef __cplusplus
}
#endif
