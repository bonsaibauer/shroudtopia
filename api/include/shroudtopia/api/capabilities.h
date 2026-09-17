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
