<!-- Generated from api/include/shroudtopia/api/*.h; do not edit by hand. -->
# API-Referenz

Diese Referenz wird direkt aus den öffentlichen Headern erzeugt. Die folgenden Deklarationen sind die kanonische API; es wird keine separate Contract-Kopie gepflegt.

> Abläufe, Lifecycle-Regeln, Berechtigungen und Beispiele stehen in den Domain-Guides.

## `actions.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/actions.h`

```c
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
```

## `assets.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/assets.h`

```c
#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASSETS_SERVICE_ID "shroudtopia.assets"
#define ASSETS_SERVICE_VERSION_MAJOR 2u
#define ASSETS_SERVICE_VERSION_MINOR 1u

typedef struct AssetId {
    size_t struct_size;
    StringView guid;
    StringView type_name;
    uint32_t part;
} AssetId;

typedef Result (CALL* AssetVisitor)(
    const AssetId* asset, void* user_data);

typedef struct AssetsApi {
    size_t struct_size;
    Result (CALL* list)(StringView owner_id, StringView type_name,
        AssetVisitor visitor, void* user_data);
    Result (CALL* get)(StringView owner_id,
        const AssetId* asset, char* buffer, size_t capacity, size_t* required_size);
    Result (CALL* update)(StringView owner_id,
        const AssetId* asset, StringView json);
    Result (CALL* create)(StringView owner_id, StringView type_name,
        StringView json, AssetVisitor visitor, void* user_data);
    Result (CALL* reset)(StringView owner_id);
    Result (CALL* save)(StringView owner_id);
    /* Asset API 2.1. Existing JSON-pointer path, typed JSON value.
       Disjoint fields compose; overlapping owners return CONFLICT. */
    Result (CALL* set)(StringView owner_id,
        const AssetId* asset, StringView path, StringView json);
} AssetsApi;

#ifdef __cplusplus
}
#endif
```

## `capabilities.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/capabilities.h`

```c
#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CapabilityInfo {
    size_t struct_size;
    uint32_t version_major;
    uint32_t version_minor;
    uint64_t flags;
    uint8_t available;
    uint8_t reserved[7];
} CapabilityInfo;

#ifdef __cplusplus
}
#endif
```

## `commands.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/commands.h`

```c
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
```

## `events.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/events.h`

```c
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
```

## `game_settings.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/game_settings.h`

```c
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
```

## `host.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/host.h`

```c
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
```

## `lifecycle.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/lifecycle.h`

```c
#pragma once

#include "shroudtopia/api/host.h"

#ifdef __cplusplus
extern "C" {
#endif

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
```

## `log.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/log.h`

```c
#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4
} LogLevel;

#define LOG_READ_SERVICE_ID "shroudtopia.logging.read"
#define LOG_READ_SERVICE_VERSION_MAJOR 2u
#define LOG_READ_SERVICE_VERSION_MINOR 0u
typedef enum LogSource { LOG_SOURCE_LOADER = 0, LOG_SOURCE_GAME = 1 } LogSource;
/* Reads the existing file, not a second logging pipeline. Caller owns buffer.
   At most capacity bytes, no terminator. Tail is bounded; NOT_FOUND means no file yet. */
typedef struct LogApi {
    size_t struct_size;
    uint32_t api_version;
    Result (CALL* read_tail)(LogSource source, char* buffer, size_t capacity, size_t* written);
} LogApi;

#ifdef __cplusplus
}
#endif
```

## `mod_settings.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/mod_settings.h`

```c
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
```

## `patches.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/patches.h`

```c
#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RUNTIME_PATCHES_SERVICE_ID "shroudtopia.runtime.patches"
#define RUNTIME_PATCHES_VERSION_MAJOR 2u
#define RUNTIME_PATCHES_VERSION_MINOR 0u

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

typedef struct PatchesApi {
    size_t struct_size;
    Result (CALL* create)(
        StringView owner_id, const RuntimePatchOptions* descriptor, RuntimePatch* patch);
    Result (CALL* set_enabled)(StringView owner_id, RuntimePatch patch, uint8_t enabled);
    Result (CALL* get_state)(StringView owner_id, RuntimePatch patch, RuntimePatchState* state);
    Result (CALL* release)(StringView owner_id, RuntimePatch patch);
} PatchesApi;

#ifdef __cplusplus
}
#endif
```

## `result.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/result.h`

```c
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

#define API_VERSION UINT32_C(0x00020000)

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


#ifdef __cplusplus
}
#endif
```

## `services.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/services.h`

```c
#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    uint32_t minimum_minor;
} ServiceRequest;

#ifdef __cplusplus
}
#endif
```

## `ui.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/ui.h`

```c
#pragma once
#include "shroudtopia/api/result.h"
#ifdef __cplusplus
extern "C" {
#endif
#define UI_TEXT_SERVICE_ID "shroudtopia.ui.text"
#define UI_TEXT_SERVICE_VERSION_MAJOR 2u
#define UI_TEXT_SERVICE_VERSION_MINOR 0u
typedef uint64_t TextWindow;
/* Native Windows overlay for windowed/borderless clients, hidden initially.
   No exclusive-fullscreen rendering. UTF-8 strings copied, 1..8 tabs.
   Title/labels <= 128 bytes. Hotkey is a Windows virtual key (0 disables).
   Creation asynchronous; destroy joins the UI thread. No mod callbacks. */
typedef struct TextWindowOptions {
    size_t struct_size;
    StringView title;
    const StringView* tabs;
    size_t tab_count;
    uint32_t toggle_key;
} TextWindowOptions;
typedef enum TextWindowStatus { TEXT_PENDING = 0, TEXT_READY = 1, TEXT_FAILED = 2 } TextWindowStatus;
typedef struct UiApi {
    size_t struct_size;
    uint32_t api_version;
    Result (CALL* create)(StringView owner, const TextWindowOptions*, TextWindow*);
    /* Replace one tab's snapshot, <=1 MiB UTF-8; empty clears it.
       Thread safe, coalesced; not an event queue. */
    Result (CALL* set_text)(StringView owner, TextWindow, size_t tab, StringView text);
    Result (CALL* get_status)(StringView owner, TextWindow, TextWindowStatus*);
    Result (CALL* destroy)(StringView owner, TextWindow);
} UiApi;
#ifdef __cplusplus
}
#endif
```

## `world.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/world.h`

```c
#pragma once

#include "shroudtopia/api/result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WORLD_SERVICE_ID "shroudtopia.world"
#define WORLD_SERVICE_VERSION_MAJOR 2u
#define WORLD_SERVICE_VERSION_MINOR 1u

typedef struct Vec3d { double x, y, z; } Vec3d;
typedef struct Quaterniond { double x, y, z, w; } Quaterniond;
typedef struct Transform {
    size_t struct_size;
    Vec3d position;
    Quaterniond rotation;
    Vec3d scale;
} Transform;

typedef struct Aabb {
    size_t struct_size;
    Vec3d minimum;
    Vec3d maximum;
} Aabb;

typedef struct EntityId {
    uint64_t id;
    uint32_t generation;
    uint32_t session;
} EntityId;

typedef enum EntityKind {
    ENTITY_UNKNOWN = 0,
    ENTITY_PROP = 1,
    ENTITY_OTHER = 2
} EntityKind;

typedef struct Entity {
    size_t struct_size;
    EntityId id;
    StringView template_id;
    Transform transform;
    /* World 2.1: PROP must identify a supported static template instance.
       NPCs/players and unsupported dynamic objects are OTHER, never PROP. */
    EntityKind kind;
} Entity;

typedef enum Coverage {
    COVERAGE_UNKNOWN = 0,
    COVERAGE_EMPTY = 1,
    COVERAGE_OCCUPIED = 2
} Coverage;

typedef struct Grid {
    size_t struct_size;
    StringView id;
    Vec3d origin;
    Vec3d cell_size;
    uint32_t chunk_size_x;
    uint32_t chunk_size_y;
    uint32_t chunk_size_z;
} Grid;

typedef struct WorldApi {
    size_t struct_size;
    uint32_t api_version;
    /* Regions are half-open world-space bounds. OK means complete coverage;
       an unloaded/unknown region must return an error, never a partial OK.
       Template strings remain valid until the next world API call on this thread.
       A capacity query performs no writes; callers initialize output struct sizes. */
    Result (CALL* list_entities)(StringView owner_id, const Aabb* region,
        Entity* entities, size_t capacity, size_t* required_count);
    Result (CALL* get_entity)(StringView owner_id, EntityId entity, Transform* transform);
    Result (CALL* create_entity)(StringView owner_id, StringView template_id,
        const Transform* transform, EntityId* entity);
    Result (CALL* remove_entity)(StringView owner_id, EntityId entity);
    Result (CALL* update_entity)(StringView owner_id, EntityId entity, const Transform* transform);
    Result (CALL* get_grid)(StringView owner_id, StringView grid_id, Grid* grid);
    /* Grid bounds must align with grid origin/cell size. X varies fastest.
       UNKNOWN is not air. Writers leave UNKNOWN cells untouched. A failed
       write may be partial; callers must preserve recovery data. */
    Result (CALL* get_grid_region)(StringView owner_id, StringView grid_id,
        const Aabb* region, uint32_t* values, Coverage* coverage, size_t capacity, size_t* required_count);
    Result (CALL* update_grid_region)(StringView owner_id, StringView grid_id,
        const Aabb* region, const uint32_t* values, const Coverage* coverage, size_t count);
} WorldApi;

#ifdef __cplusplus
}
#endif
```

## `world_grid.h`

**Kanonische Quelle:** `api/include/shroudtopia/api/world_grid.h`

```c
#pragma once

#include "shroudtopia/api/world.h"
#include <math.h>
#include <limits.h>

/* Pure API utilities: these calculations do not require a loaded game world. */
typedef struct GridRegion {
    size_t struct_size;
    Aabb bounds;
    int32_t minimum[3];
    uint32_t dimensions[3];
    size_t cell_count;
} GridRegion;

/* Both marked cells are included. The resulting world bounds are half-open.
   Buffer order for world grid operations is x + width * (y + height * z). */
static inline Result GridRegionFromPoints(const Grid* grid,
    Vec3d a, Vec3d b, size_t maximum_cells, GridRegion* output) {
    if (!grid || !output || grid->struct_size < sizeof(*grid) ||
        output->struct_size < sizeof(*output) || maximum_cells == 0) return RESULT_INVALID_ARGUMENT;
    const double origins[3] = {grid->origin.x, grid->origin.y, grid->origin.z};
    const double steps[3] = {grid->cell_size.x, grid->cell_size.y, grid->cell_size.z};
    const double first[3] = {a.x, a.y, a.z}, second[3] = {b.x, b.y, b.z};
    double lo[3], hi[3];
    GridRegion result = {0};
    result.struct_size = sizeof(result);
    result.bounds.struct_size = sizeof(result.bounds);
    result.cell_count = 1;
    for (int axis = 0; axis < 3; ++axis) {
        if (!isfinite(origins[axis]) || !isfinite(steps[axis]) || steps[axis] <= 0 ||
            !isfinite(first[axis]) || !isfinite(second[axis])) return RESULT_INVALID_ARGUMENT;
        const double x = floor((first[axis] - origins[axis]) / steps[axis]);
        const double y = floor((second[axis] - origins[axis]) / steps[axis]);
        const double minimum = fmin(x, y), maximum = fmax(x, y);
        if (!isfinite(minimum) || !isfinite(maximum) || minimum < INT32_MIN || maximum >= INT32_MAX)
            return RESULT_INVALID_ARGUMENT;
        const double count = maximum - minimum + 1;
        if (count > INT32_MAX || count > (double)(maximum_cells / result.cell_count))
            return RESULT_INVALID_ARGUMENT;
        result.minimum[axis] = (int32_t)minimum;
        result.dimensions[axis] = (uint32_t)count;
        result.cell_count *= result.dimensions[axis];
        lo[axis] = origins[axis] + minimum * steps[axis];
        hi[axis] = origins[axis] + (maximum + 1) * steps[axis];
        if (!isfinite(lo[axis]) || !isfinite(hi[axis]) || hi[axis] <= lo[axis]) return RESULT_INVALID_ARGUMENT;
    }
    result.bounds.minimum.x = lo[0]; result.bounds.minimum.y = lo[1]; result.bounds.minimum.z = lo[2];
    result.bounds.maximum.x = hi[0]; result.bounds.maximum.y = hi[1]; result.bounds.maximum.z = hi[2];
    *output = result;
    return RESULT_OK;
}
```
