#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ST_ServiceDescriptor {
    size_t struct_size;
    ST_StringView contract_id;
    uint32_t version_major;
    uint32_t version_minor;
    const void* interface_pointer;
} ST_ServiceDescriptor;

typedef struct ST_ServiceRequest {
    size_t struct_size;
    ST_StringView contract_id;
    uint32_t version_major;
    uint32_t minimum_minor;
} ST_ServiceRequest;

#ifdef __cplusplus
}
#endif
