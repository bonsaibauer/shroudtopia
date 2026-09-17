#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef Result (CALL* CommandCallback)(StringView arguments, void* user_data);
typedef struct CommandDescriptor {
    size_t struct_size;
    StringView command_id;
    StringView description;
    CommandCallback callback;
    void* user_data;
} CommandDescriptor;

#ifdef __cplusplus
}
#endif
