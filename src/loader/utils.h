#pragma once
#include "shroudtopia.h"

namespace Utils {
uint8_t LogLevelMask();
void Log(LogLevel level, const char* format, ...);
void LogAs(LogLevel level, const char* source, const char* format, ...);
void BeginLogSession(const char* target);
Result CALL ReadLogTail(LogSource source, char* buffer, size_t capacity, size_t* written);
}
