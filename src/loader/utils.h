#pragma once
#include "shroudtopia/api/logging.h"

namespace Utils {
enum LogLevel {
    VERBOSE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERRR = 4,
    NONE = 5
};

void Log(LogLevel level, const char* format, ...);
void BeginLogSession(const char* target);
ST_Result ST_CALL ReadLogTail(ST_LogSourceV1 source, char* buffer, size_t capacity, size_t* written);
}
