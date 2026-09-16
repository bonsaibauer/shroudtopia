#pragma once

#include "shroudtopia/api/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ST_LogLevel {
    ST_LOG_TRACE = 0,
    ST_LOG_DEBUG = 1,
    ST_LOG_INFO = 2,
    ST_LOG_WARNING = 3,
    ST_LOG_ERROR = 4
} ST_LogLevel;

#define ST_LOG_READ_SERVICE_ID "shroudtopia.logging.read"
typedef enum ST_LogSourceV1 { ST_LOG_SOURCE_LOADER = 0, ST_LOG_SOURCE_GAME = 1 } ST_LogSourceV1;
/* Reads the existing file, not a second logging pipeline. Caller owns buffer.
   At most capacity bytes, no terminator. Tail is bounded; NOT_FOUND means no file yet. */
typedef struct ST_LogReadApiV1 {
    size_t struct_size;
    uint32_t abi_version;
    ST_Result (ST_CALL* read_tail)(ST_LogSourceV1 source, char* buffer, size_t capacity, size_t* written);
} ST_LogReadApiV1;

#ifdef __cplusplus
}
#endif
