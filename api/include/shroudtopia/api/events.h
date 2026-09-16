#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ST_Event {
    size_t struct_size;
    ST_StringView event_id;
    const void* payload;
    size_t payload_size;
} ST_Event;

typedef ST_Result (ST_CALL* ST_EventCallback)(const ST_Event* event_data, void* user_data);
typedef struct ST_EventSubscription {
    size_t struct_size;
    ST_StringView event_id;
    ST_EventCallback callback;
    void* user_data;
} ST_EventSubscription;

#ifdef __cplusplus
}
#endif
