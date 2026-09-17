#pragma once

#include "shroudtopia/api/capabilities.h"
#include "shroudtopia/api/commands.h"
#include "shroudtopia/api/actions.h"
#include "shroudtopia/api/events.h"
#include "shroudtopia/api/log.h"
#include "shroudtopia/api/services.h"
#include "shroudtopia/api/mod_settings.h"
#include "shroudtopia/api/game_settings.h"
#include "shroudtopia/api/assets.h"
#include "shroudtopia/api/patches.h"
#include "shroudtopia/api/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Api {
    size_t struct_size;
    uint32_t api_version;
    const AssetsApi* assets;
    const PatchesApi* patches;
    const UiApi* ui;
    const LogApi* logs;
    Result (CALL* register_service)(StringView owner_id, const ServiceDescriptor* descriptor, Registration* registration);
    Result (CALL* find_service)(const ServiceRequest* request, const void** interface_pointer);
    Result (CALL* subscribe_event)(StringView owner_id, const EventSubscription* subscription, Registration* registration);
    Result (CALL* publish_event)(StringView owner_id, const Event* event_data);
    Result (CALL* register_command)(StringView owner_id, const CommandDescriptor* descriptor, Registration* registration);
    Result (CALL* invoke_command)(StringView owner_id, StringView command_id, StringView arguments);
    Result (CALL* register_mod_settings)(StringView owner_id, const ModSettingsDescriptor* descriptor, Registration* registration);
    Result (CALL* register_action)(StringView owner_id, const Action* action, Registration* registration);
    Result (CALL* invoke_action)(StringView owner_id, StringView action_id, StringView input_json);
    Result (CALL* get_action_state)(StringView action_id, ActionState* state);
    Result (CALL* release_registration)(Registration registration);
    Result (CALL* release_owner)(StringView owner_id);
    Result (CALL* query_capability)(StringView capability_id, CapabilityInfo* information);
    Result (CALL* check_permission)(StringView owner_id, StringView permission_id, uint8_t* allowed);
    Result (CALL* log)(StringView owner_id, LogLevel level, StringView message);
    Result (CALL* get_mod_setting_bool)(StringView owner_id, StringView key, uint8_t fallback, uint8_t* value);
    Result (CALL* get_mod_setting_number)(StringView owner_id, StringView key, double fallback, double* value);
    Result (CALL* get_game_setting)(StringView key, char* buffer, size_t capacity, size_t* required_size);
    Result (CALL* stage_game_setting)(StringView key, StringView value_json);
    Result (CALL* reset_game_setting)(StringView key);
} Api;

#ifdef __cplusplus
}
#endif
