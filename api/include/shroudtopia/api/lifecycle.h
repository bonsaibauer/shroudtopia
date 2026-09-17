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
