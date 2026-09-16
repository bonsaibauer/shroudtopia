#pragma once

#include "shroudtopia/api/host.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ST_Result (ST_CALL* ST_ModLifecycleCallback)(const ST_HostApiV1* api, void* user_data);
typedef ST_Result (ST_CALL* ST_ModUpdateCallback)(const ST_HostApiV1* api, void* user_data, double delta_seconds);
typedef struct ST_ModDescriptorV1 {
    size_t struct_size;
    ST_StringView mod_id;
    void* user_data;
    ST_ModLifecycleCallback on_load;
    ST_ModLifecycleCallback on_activate;
    ST_ModUpdateCallback on_update;
    ST_ModLifecycleCallback on_deactivate;
    ST_ModLifecycleCallback on_unload;
} ST_ModDescriptorV1;
typedef ST_Result (ST_CALL* ST_CreateModV1)(uint32_t requested_abi, ST_ModDescriptorV1* descriptor);
ST_API ST_Result ST_CALL ShroudtopiaGetApi(uint32_t requested_abi, const ST_HostApiV1** api);

#ifdef __cplusplus
}
#endif
