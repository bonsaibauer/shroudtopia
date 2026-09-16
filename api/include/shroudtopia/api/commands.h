#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ST_Result (ST_CALL* ST_CommandCallback)(ST_StringView arguments, void* user_data);
typedef struct ST_CommandDescriptor {
    size_t struct_size;
    ST_StringView command_id;
    ST_StringView description;
    ST_CommandCallback callback;
    void* user_data;
} ST_CommandDescriptor;

#ifdef __cplusplus
}
#endif
