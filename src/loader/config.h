#pragma once

#include "pch.h"
#include "defines.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <utility>

using json = nlohmann::json;

class Config final {
public:
    static json jConfig;
    static std::filesystem::file_time_type lastModifiedTime;
    inline static std::recursive_mutex mutex;

    static bool readFile() {
        std::scoped_lock lock(mutex);
        try {
            std::ifstream file(SHROUDTOPIA_CONFIG_FILE);
            if (!file) return false;
            file >> jConfig;
            lastModifiedTime = std::filesystem::last_write_time(SHROUDTOPIA_CONFIG_FILE);
            return true;
        } catch (...) { return false; }
    }

    static bool writeFile() {
        std::scoped_lock lock(mutex);
        std::ofstream file(SHROUDTOPIA_CONFIG_FILE);
        if (!file) return false;
        file << jConfig.dump(2);
        file.close();
        lastModifiedTime = std::filesystem::last_write_time(SHROUDTOPIA_CONFIG_FILE);
        return true;
    }

    template <typename T>
    static T get(const char* key, T fallback = T{}) {
        std::scoped_lock lock(mutex);
        try { return jConfig.at(key).get<T>(); }
        catch (...) { return fallback; }
    }

    template <typename T>
    static T modGet(const char* mod_id, const char* key, T fallback = T{}) {
        std::scoped_lock lock(mutex);
        try { return jConfig.at("mods").at(mod_id).at(key).get<T>(); }
        catch (...) { return fallback; }
    }

    static void setConfigFromJSON(json value) { std::scoped_lock lock(mutex); jConfig = std::move(value); }

    template <typename T>
    static bool modSet(const char* mod_id, const char* key, T value) {
        std::scoped_lock lock(mutex);
        try {
            if (!jConfig.contains("mods") || !jConfig["mods"].is_object()) jConfig["mods"] = json::object();
            jConfig["mods"][mod_id][key] = std::move(value);
            return writeFile();
        } catch (...) { return false; }
    }

    static bool reloadIfChanged() {
        std::scoped_lock lock(mutex);
        try {
            if (std::filesystem::last_write_time(SHROUDTOPIA_CONFIG_FILE) == lastModifiedTime) return false;
            return readFile();
        } catch (...) { return false; }
    }
};
