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
