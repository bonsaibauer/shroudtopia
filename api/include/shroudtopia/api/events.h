#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Event {
    size_t struct_size;
    StringView event_id;
    const void* payload;
    size_t payload_size;
} Event;

typedef Result (CALL* EventCallback)(const Event* event_data, void* user_data);
typedef struct EventSubscription {
    size_t struct_size;
    StringView event_id;
    EventCallback callback;
    void* user_data;
} EventSubscription;

#ifdef __cplusplus
}
#endif
