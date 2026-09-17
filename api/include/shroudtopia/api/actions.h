#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ActionState {
    ACTION_AVAILABLE = 0,
    ACTION_DISABLED = 1,
    ACTION_RUNNING = 2,
    ACTION_FAILED = 3
} ActionState;

typedef Result (CALL* ActionHandler)(StringView input_json, void* user_data);

typedef struct Action {
    size_t struct_size;
    StringView id;
    StringView title;
    StringView description;
    StringView input_schema_json;
    ActionHandler invoke;
    void* user_data;
} Action;

#ifdef __cplusplus
}
#endif
