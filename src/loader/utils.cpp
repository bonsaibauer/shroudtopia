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

namespace {
std::mutex log_mutex;

const char* level_name(Utils::LogLevel level) {
    switch (level) {
    case Utils::VERBOSE: return "TRACE";
    case Utils::DEBUG: return "DEBUG";
    case Utils::INFO: return "INFO";
    case Utils::WARN: return "WARN";
    case Utils::ERRR: return "ERROR";
    default: return "NONE";
    }
}
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
    } catch (...) {
        // Logging remains available in append mode when archival is unavailable.
    }
}

void Utils::Log(LogLevel level, const char* format, ...) {
    if (!Config::get<bool>("enableLogging", true) || level == NONE) return;

    char message[2048]{};
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(message, sizeof(message), format, arguments);
    va_end(arguments);

    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
    gmtime_s(&utc, &now);
    char timestamp[32]{};
    std::strftime(timestamp, sizeof(timestamp), "%FT%TZ", &utc);

    std::scoped_lock lock(log_mutex);
    std::ofstream file(SHROUDTOPIA_LOG_FILE, std::ios::app);
    file << '[' << timestamp << "][shroudtopia][" << level_name(level) << "] " << message << '\n';
    std::cout << "[shroudtopia][" << level_name(level) << "] " << message << std::endl;
}

ST_Result ST_CALL Utils::ReadLogTail(ST_LogSourceV1 source, char* buffer, size_t capacity, size_t* written) {
    if (written) *written = 0;
    if (!buffer || !written || capacity == 0 || capacity > 1024 * 1024 ||
        (source != ST_LOG_SOURCE_LOADER && source != ST_LOG_SOURCE_GAME)) return ST_RESULT_INVALID_ARGUMENT;
    try {
        std::scoped_lock lock(log_mutex);
        std::ifstream file(source == ST_LOG_SOURCE_LOADER ? SHROUDTOPIA_LOG_FILE : "enshrouded.log", std::ios::binary | std::ios::ate);
        if (!file) return ST_RESULT_NOT_FOUND;
        const auto end = file.tellg();
        if (end < 0) return ST_RESULT_INTERNAL_ERROR;
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
        return ST_RESULT_OK;
    } catch (...) { return ST_RESULT_INTERNAL_ERROR; }
}
