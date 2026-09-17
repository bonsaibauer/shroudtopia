#pragma once

#include "pch.h"
#include "defines.h"

#include <filesystem>
#include <fstream>
#include <utility>

using json = nlohmann::json;

class Config final {
public:
    static json jConfig;
    static std::filesystem::file_time_type lastModifiedTime;

    static bool readFile() {
        try {
            std::ifstream file(SHROUDTOPIA_CONFIG_FILE);
            if (!file) return false;
            file >> jConfig;
            lastModifiedTime = std::filesystem::last_write_time(SHROUDTOPIA_CONFIG_FILE);
            return true;
        } catch (...) { return false; }
    }

    static bool writeFile() {
        std::ofstream file(SHROUDTOPIA_CONFIG_FILE);
        if (!file) return false;
        file << jConfig.dump(2);
        file.close();
        lastModifiedTime = std::filesystem::last_write_time(SHROUDTOPIA_CONFIG_FILE);
        return true;
    }

    template <typename T>
    static T get(const char* key, T fallback = T{}) {
        try { return jConfig.at(key).get<T>(); }
        catch (...) { return fallback; }
    }

    template <typename T>
    static T modGet(const char* mod_id, const char* key, T fallback = T{}) {
        try { return jConfig.at("mods").at(mod_id).at(key).get<T>(); }
        catch (...) { return fallback; }
    }

    static void setConfigFromJSON(json value) { jConfig = std::move(value); }

    static bool reloadIfChanged() {
        try {
            if (std::filesystem::last_write_time(SHROUDTOPIA_CONFIG_FILE) == lastModifiedTime) return false;
            return readFile();
        } catch (...) { return false; }
    }
};
