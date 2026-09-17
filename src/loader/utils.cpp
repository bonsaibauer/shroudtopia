#include "pch.h"
#include "utils.h"

#include "config.h"
#include "defines.h"

#include <cstdarg>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <cstring>
#include <algorithm>
#include <cctype>

namespace {
std::mutex log_mutex;
auto session_started = std::chrono::steady_clock::now();

char level_name(LogLevel level) {
    switch (level) {
    case LOG_TRACE: return 'T';
    case LOG_DEBUG: return 'D';
    case LOG_INFO: return 'I';
    case LOG_WARNING: return 'W';
    case LOG_ERROR: return 'E';
    default: return '?';
    }
}

void Write(LogLevel level, const char* source, const char* format, va_list arguments) {
    if ((Utils::LogLevelMask() & (1u << static_cast<unsigned>(level))) == 0) return;
    char message[2048]{};
    vsnprintf(message, sizeof(message), format, arguments);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - session_started).count();
    const auto hours = elapsed / 3600000;
    const auto minutes = (elapsed / 60000) % 60;
    const auto seconds = (elapsed / 1000) % 60;
    const auto milliseconds = elapsed % 1000;
    char prefix[128]{};
    std::snprintf(prefix, sizeof(prefix), "[%c %02lld:%02lld:%02lld,%03lld] [%s] ",
        level_name(level), hours, minutes, seconds, milliseconds,
        source && *source ? source : "shroudtopia");
    std::scoped_lock lock(log_mutex);
    std::ofstream file(SHROUDTOPIA_LOG_FILE, std::ios::app);
    file << prefix << message << '\n';
    std::cout << prefix << message << std::endl;
}
}

uint8_t Utils::LogLevelMask() {
    if (!Config::get<bool>("enableLogging", true)) return 0;
    auto configured = Config::get<std::string>("logLevel", "INFO");
    std::transform(configured.begin(), configured.end(), configured.begin(), [](unsigned char value) {
        return static_cast<char>(std::toupper(value));
    });
    if (configured == "ALL") return 0x1f;
    if (configured == "TRACE") return 1u << LOG_TRACE;
    if (configured == "DEBUG") return 1u << LOG_DEBUG;
    if (configured == "WARNING" || configured == "WARN") return 1u << LOG_WARNING;
    if (configured == "ERROR") return 1u << LOG_ERROR;
    return 1u << LOG_INFO;
}

void Utils::BeginLogSession(const char* target) {
    try {
        std::scoped_lock lock(log_mutex);
        namespace fs = std::filesystem;
        const fs::path current{SHROUDTOPIA_LOG_FILE};
        if (fs::exists(current) && fs::file_size(current) > 0) {
            const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm utc{};
            gmtime_s(&utc, &now);
            char timestamp[32]{};
            std::strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", &utc);
            const fs::path archive{SHROUDTOPIA_LOG_ARCHIVE_FOLDER};
            fs::create_directories(archive);
            std::string base = std::string("shroudtopia-") + timestamp + '-' + (target && *target ? target : "unknown");
            fs::path destination = archive / (base + ".log");
            for (unsigned suffix = 2; fs::exists(destination); ++suffix)
                destination = archive / (base + '-' + std::to_string(suffix) + ".log");
            fs::rename(current, destination);
        }
        std::ofstream fresh(current, std::ios::trunc);
        session_started = std::chrono::steady_clock::now();
    } catch (...) {
        // Logging remains available in append mode when archival is unavailable.
    }
}

void Utils::Log(LogLevel level, const char* format, ...) {
    va_list arguments;
    va_start(arguments, format);
    Write(level, "shroudtopia", format, arguments);
    va_end(arguments);
}

void Utils::LogAs(LogLevel level, const char* source, const char* format, ...) {
    va_list arguments;
    va_start(arguments, format);
    Write(level, source, format, arguments);
    va_end(arguments);
}

Result CALL Utils::ReadLogTail(LogSource source, char* buffer, size_t capacity, size_t* written) {
    if (written) *written = 0;
    if (!buffer || !written || capacity == 0 || capacity > 1024 * 1024 ||
        (source != LOG_SOURCE_LOADER && source != LOG_SOURCE_GAME)) return RESULT_INVALID_ARGUMENT;
    try {
        std::scoped_lock lock(log_mutex);
        std::ifstream file(source == LOG_SOURCE_LOADER ? SHROUDTOPIA_LOG_FILE : "enshrouded.log", std::ios::binary | std::ios::ate);
        if (!file) return RESULT_NOT_FOUND;
        const auto end = file.tellg();
        if (end < 0) return RESULT_INTERNAL_ERROR;
        const auto count = (std::min)(static_cast<size_t>(end), capacity);
        const auto start = end - static_cast<std::streamoff>(count);
        bool partial = false;
        if (start > 0) { file.seekg(start - std::streamoff{1}); partial = file.get() != '\n'; }
        file.seekg(start);
        file.read(buffer, static_cast<std::streamsize>(count));
        size_t length = static_cast<size_t>(file.gcount());
        // Discard the first partial line when reading a bounded tail.
        if (partial) {
            const char* newline = static_cast<const char*>(std::memchr(buffer, '\n', length));
            if (newline) {
                const size_t skip = static_cast<size_t>(newline - buffer) + 1;
                length -= skip;
                std::memmove(buffer, buffer + skip, length);
            } else length = 0;
        }
        *written = length;
        return RESULT_OK;
    } catch (...) { return RESULT_INTERNAL_ERROR; }
}
