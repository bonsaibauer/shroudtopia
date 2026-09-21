#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(SHROUDTOPIA_EXPORTS)
#    define API_EXPORT __declspec(dllexport)
#  else
#    define API_EXPORT __declspec(dllimport)
#  endif
#  define CALL __cdecl
#else
#  define API_EXPORT
#  define CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define API_VERSION UINT32_C(0x00010002)

typedef struct StringView { const char* data; size_t size; } StringView;
typedef uint64_t Registration;

typedef enum Result {
    RESULT_OK = 0,
    RESULT_INVALID_ARGUMENT = 1,
    RESULT_CONFLICT = 2,
    RESULT_NOT_FOUND = 3,
    RESULT_VERSION_MISMATCH = 4,
    RESULT_PERMISSION_DENIED = 5,
    RESULT_NOT_AVAILABLE = 6,
    RESULT_CALLBACK_FAILED = 7,
    RESULT_INTERNAL_ERROR = 8
} Result;

typedef enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4
} LogLevel;

typedef enum LogSource { LOG_SOURCE_LOADER = 0, LOG_SOURCE_GAME = 1 } LogSource;

typedef struct CapabilityInfo {
    size_t struct_size;
    uint32_t version_major;
    uint32_t version_minor;
    uint64_t flags;
    uint8_t available;
    uint8_t reserved[7];
} CapabilityInfo;

#define CAPABILITY_LIFECYCLE_NATIVE "shroudtopia.lifecycle.native"
#define CAPABILITY_REGISTRY_SERVICES "shroudtopia.registry.services"
#define CAPABILITY_REGISTRY_EVENTS "shroudtopia.registry.events"
#define CAPABILITY_REGISTRY_COMMANDS "shroudtopia.registry.commands"
#define CAPABILITY_ASSETS_READ "shroudtopia.assets.read"
#define CAPABILITY_ASSETS_WRITE "shroudtopia.assets.write"
#define CAPABILITY_RUNTIME_PATCHES "shroudtopia.runtime.patches"
#define CAPABILITY_UI_PAGES "shroudtopia.ui.pages"

typedef struct ServiceDescriptor {
    size_t struct_size;
    StringView contract_id;
    uint32_t version_major;
    uint32_t version_minor;
    const void* interface_pointer;
} ServiceDescriptor;

typedef struct ServiceRequest {
    size_t struct_size;
    StringView contract_id;
    uint32_t version_major;
    uint32_t version_minor;
} ServiceRequest;

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

typedef Result (CALL* CommandCallback)(StringView arguments, void* user_data);
typedef struct CommandDescriptor {
    size_t struct_size;
    StringView command_id;
    StringView description;
    CommandCallback callback;
    void* user_data;
} CommandDescriptor;

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

typedef uint64_t RuntimePatch;
typedef enum RuntimePatchKind {
    RUNTIME_PATCH_DIRECT = 0,
    RUNTIME_PATCH_DETOUR = 1
} RuntimePatchKind;
typedef enum RuntimeRelocationKind {
    RUNTIME_RELOCATION_REL32_RETURN = 1
} RuntimeRelocationKind;
typedef struct RuntimeRelocation {
    size_t struct_size;
    size_t payload_offset;
    RuntimeRelocationKind kind;
} RuntimeRelocation;
typedef struct RuntimePatchOptions {
    size_t struct_size;
    StringView signature;
    int64_t match_offset;
    RuntimePatchKind kind;
    size_t overwrite_size;
    const uint8_t* payload;
    size_t payload_size;
    const RuntimeRelocation* relocations;
    size_t relocation_count;
} RuntimePatchOptions;
typedef struct RuntimePatchState {
    size_t struct_size;
    uint8_t enabled;
    uint8_t reserved[7];
} RuntimePatchState;

typedef struct AssetId {
    size_t struct_size;
    StringView guid;
    StringView type_name;
    uint32_t part;
} AssetId;
typedef Result (CALL* AssetVisitor)(const AssetId* asset, void* user_data);

typedef uint64_t TextWindow;
typedef struct TextWindowOptions {
    size_t struct_size;
    StringView title;
    const StringView* tabs;
    size_t tab_count;
    uint32_t toggle_key;
} TextWindowOptions;
typedef enum TextWindowStatus {
    TEXT_PENDING = 0,
    TEXT_READY = 1,
    TEXT_FAILED = 2
} TextWindowStatus;

typedef enum UiControlType {
    UI_CONTROL_TEXT = 0,
    UI_CONTROL_SEPARATOR = 1,
    UI_CONTROL_BOOL = 2,
    UI_CONTROL_NUMBER = 3,
    UI_CONTROL_BUTTON = 4,
    UI_CONTROL_STATUS = 5
} UiControlType;

typedef enum UiStatusTone {
    UI_STATUS_NEUTRAL = 0,
    UI_STATUS_SUCCESS = 1,
    UI_STATUS_WARNING = 2,
    UI_STATUS_ERROR = 3
} UiStatusTone;

typedef struct UiControlDescriptor {
    size_t struct_size;
    StringView id;
    StringView label;
    StringView description;
    UiControlType type;
    uint32_t flags;
    double minimum;
    double maximum;
    double step;
    uint8_t bool_value;
    uint8_t reserved[7];
    double number_value;
    UiStatusTone status_tone;
} UiControlDescriptor;

typedef struct UiTabDescriptor {
    size_t struct_size;
    StringView id;
    StringView title;
    const UiControlDescriptor* controls;
    size_t control_count;
} UiTabDescriptor;

typedef Result (CALL* UiControlCallback)(StringView control_id,
    uint8_t bool_value, double number_value, void* user_data);

typedef struct UiRenderContext {
    size_t struct_size;
    void (CALL* text)(StringView text);
    void (CALL* separator)(void);
    uint8_t (CALL* button)(StringView id, StringView label);
    uint8_t (CALL* checkbox)(StringView id, StringView label, uint8_t* value);
    uint8_t (CALL* slider_number)(StringView id, StringView label, double* value,
        double minimum, double maximum, double step);
    void (CALL* same_line)(void);
    void (CALL* begin_disabled)(uint8_t disabled);
    void (CALL* end_disabled)(void);
} UiRenderContext;

typedef Result (CALL* UiRenderCallback)(const UiRenderContext* context, void* user_data);

typedef struct UiPageDescriptor {
    size_t struct_size;
    StringView id;
    StringView title;
    StringView description;
    uint32_t order;
    const UiTabDescriptor* tabs;
    size_t tab_count;
    UiControlCallback on_control;
    UiRenderCallback render;
    void* user_data;
} UiPageDescriptor;

typedef Result (CALL* UiPageVisitor)(StringView owner_id,
    UiPageDescriptor* page, void* user_data);

typedef struct Api {
    size_t struct_size;
    uint32_t api_version;

    Result (CALL* register_service)(StringView owner_id, const ServiceDescriptor* descriptor, Registration* registration);
    Result (CALL* find_service)(const ServiceRequest* request, const void** interface_pointer);
    Result (CALL* subscribe_event)(StringView owner_id, const EventSubscription* subscription, Registration* registration);
    Result (CALL* publish_event)(StringView owner_id, const Event* event_data);
    Result (CALL* register_command)(StringView owner_id, const CommandDescriptor* descriptor, Registration* registration);
    Result (CALL* invoke_command)(StringView owner_id, StringView command_id, StringView arguments);
    Result (CALL* register_action)(StringView owner_id, const Action* action, Registration* registration);
    Result (CALL* invoke_action)(StringView owner_id, StringView action_id, StringView input_json);
    Result (CALL* get_action_state)(StringView action_id, ActionState* state);
    Result (CALL* release_registration)(Registration registration);
    Result (CALL* query_capability)(StringView capability_id, CapabilityInfo* information);
    Result (CALL* check_permission)(StringView owner_id, StringView permission_id, uint8_t* allowed);

    Result (CALL* log)(StringView owner_id, LogLevel level, StringView message);
    Result (CALL* read_log_tail)(LogSource source, char* buffer, size_t capacity, size_t* written);
    Result (CALL* get_mod_setting_bool)(StringView owner_id, StringView key, uint8_t fallback, uint8_t* value);
    Result (CALL* get_mod_setting_number)(StringView owner_id, StringView key, double fallback, double* value);

    Result (CALL* create_patch)(StringView owner_id, const RuntimePatchOptions* options, RuntimePatch* patch);
    Result (CALL* set_patch_enabled)(StringView owner_id, RuntimePatch patch, uint8_t enabled);
    Result (CALL* get_patch_state)(StringView owner_id, RuntimePatch patch, RuntimePatchState* state);
    Result (CALL* release_patch)(StringView owner_id, RuntimePatch patch);

    Result (CALL* list_assets)(StringView owner_id, StringView type_name, AssetVisitor visitor, void* user_data);
    Result (CALL* get_asset)(StringView owner_id, const AssetId* asset, char* buffer, size_t capacity, size_t* required_size);
    Result (CALL* update_asset)(StringView owner_id, const AssetId* asset, StringView json);
    Result (CALL* create_asset)(StringView owner_id, StringView type_name, StringView json, AssetVisitor visitor, void* user_data);
    Result (CALL* reset_assets)(StringView owner_id);
    Result (CALL* save_assets)(StringView owner_id);
    Result (CALL* set_asset_field)(StringView owner_id, const AssetId* asset, StringView path, StringView json);

    Result (CALL* create_text_window)(StringView owner_id, const TextWindowOptions* options, TextWindow* window);
    Result (CALL* set_text_window_text)(StringView owner_id, TextWindow window, size_t tab, StringView text);
    Result (CALL* get_text_window_status)(StringView owner_id, TextWindow window, TextWindowStatus* status);
    Result (CALL* destroy_text_window)(StringView owner_id, TextWindow window);

    Result (CALL* set_mod_setting_bool)(StringView owner_id, StringView key, uint8_t value);
    Result (CALL* set_mod_setting_number)(StringView owner_id, StringView key, double value);
    Result (CALL* register_ui_page)(StringView owner_id, const UiPageDescriptor* descriptor, Registration* registration);
    Result (CALL* visit_ui_pages)(StringView owner_id, UiPageVisitor visitor, void* user_data);
} Api;

typedef Result (CALL* ModLifecycleCallback)(const Api* api, void* user_data);
typedef Result (CALL* ModUpdateCallback)(const Api* api, void* user_data, double delta_seconds);
typedef struct ModDescriptor {
    size_t struct_size;
    StringView mod_id;
    void* user_data;
    ModLifecycleCallback on_load;
    ModLifecycleCallback on_activate;
    ModUpdateCallback on_update;
    ModLifecycleCallback on_deactivate;
    ModLifecycleCallback on_unload;
} ModDescriptor;
typedef Result (CALL* CreateModFunction)(uint32_t requested_api_version, ModDescriptor* descriptor);

API_EXPORT Result CALL ShroudtopiaGetApi(uint32_t requested_api_version, const Api** api);

#ifdef __cplusplus
}
#endif
