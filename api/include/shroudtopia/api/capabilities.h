#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ST_CapabilityInfoV1 {
    size_t struct_size;
    uint32_t version_major;
    uint32_t version_minor;
    uint64_t flags;
    uint8_t available;
    uint8_t reserved[7];
} ST_CapabilityInfoV1;

#ifdef __cplusplus
}
#endif
