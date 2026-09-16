#pragma once

#include "shroudtopia/api/capabilities.h"
#include "shroudtopia/api/commands.h"
#include "shroudtopia/api/events.h"
#include "shroudtopia/api/logging.h"
#include "shroudtopia/api/services.h"
#include "shroudtopia/api/settings.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ST_HostApiV1 {
    size_t struct_size;
    uint32_t abi_version;
    ST_Result (ST_CALL* register_service)(ST_StringView owner_id, const ST_ServiceDescriptor* descriptor, ST_Registration* registration);
    ST_Result (ST_CALL* find_service)(const ST_ServiceRequest* request, const void** interface_pointer);
    ST_Result (ST_CALL* subscribe_event)(ST_StringView owner_id, const ST_EventSubscription* subscription, ST_Registration* registration);
    ST_Result (ST_CALL* publish_event)(ST_StringView owner_id, const ST_Event* event_data);
    ST_Result (ST_CALL* register_command)(ST_StringView owner_id, const ST_CommandDescriptor* descriptor, ST_Registration* registration);
    ST_Result (ST_CALL* execute_command)(ST_StringView owner_id, ST_StringView command_id, ST_StringView arguments);
    ST_Result (ST_CALL* register_settings)(ST_StringView owner_id, const ST_SettingsDescriptor* descriptor, ST_Registration* registration);
    ST_Result (ST_CALL* release_registration)(ST_Registration registration);
    ST_Result (ST_CALL* release_owner)(ST_StringView owner_id);
    ST_Result (ST_CALL* query_capability)(ST_StringView capability_id, ST_CapabilityInfoV1* information);
    ST_Result (ST_CALL* check_permission)(ST_StringView owner_id, ST_StringView permission_id, uint8_t* allowed);
    ST_Result (ST_CALL* log)(ST_StringView owner_id, ST_LogLevel level, ST_StringView message);
    ST_Result (ST_CALL* get_setting_bool)(ST_StringView owner_id, ST_StringView key, uint8_t fallback, uint8_t* value);
    /* Optional tail; check struct_size before calling on an older loader. */
    ST_Result (ST_CALL* get_setting_number)(ST_StringView owner_id, ST_StringView key, double fallback, double* value);
} ST_HostApiV1;

#ifdef __cplusplus
}
#endif
