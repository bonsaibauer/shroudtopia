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
