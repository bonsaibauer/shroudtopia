#include "pch.h"
#include "runtime.h"

#include "config.h"
#include "defines.h"
#include "platform_api.h"
#include "utils.h"
#include "shroudtopia.h"
#include "world_diagnostic.h"
#include "world_engine.h"
#include "../ui/ui_module.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <cwctype>

namespace fs = std::filesystem;

#ifndef SHROUDTOPIA_VERSION
#define SHROUDTOPIA_VERSION "dev"
#endif

#ifndef SHROUDTOPIA_BUILD_NUMBER
#define SHROUDTOPIA_BUILD_NUMBER "dev"
#endif

json Config::jConfig;
std::filesystem::file_time_type Config::lastModifiedTime;

namespace {
struct LoadedMod {
    HMODULE module = nullptr;
    ModDescriptor descriptor{};
    std::string id;
    std::string version;
    const Api* mod_api = nullptr;
    bool loaded = false;
    bool active = false;
    bool failed = false;
    Registration settings_page = 0;
};

std::map<std::string, LoadedMod> mods;
std::set<std::string> discovered_manifests;
const Api* api = nullptr;
HANDLE runtime_thread = nullptr;
HANDLE stop_event = nullptr;

json default_config{
    {"active", true},
    {"updateDelay", 500},
    {"enableLogging", true},
    {"logLevel", "INFO"},
    {"mods", json::object()}
};

enum class ProcessTarget { Unknown, Client, Server };

ProcessTarget current_target() {
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) return ProcessTarget::Unknown;
    std::wstring name(path, length);
    std::transform(name.begin(), name.end(), name.begin(), [](wchar_t value) {
        return static_cast<wchar_t>(std::towlower(value));
    });
    if (name.ends_with(L"enshrouded_server.exe")) return ProcessTarget::Server;
    if (name.ends_with(L"enshrouded.exe")) return ProcessTarget::Client;
    return ProcessTarget::Unknown;
}

const char* target_name(ProcessTarget target) {
    return target == ProcessTarget::Client ? "client" :
        target == ProcessTarget::Server ? "server" : "unknown";
}

double elapsed_ms(std::chrono::steady_clock::time_point started) {
    return std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - started).count();
}

bool supports_target(const std::string& target) {
    if (target == "both") return true;
    const auto process = current_target();
    return (target == "client" && process == ProcessTarget::Client) ||
        (target == "server" && process == ProcessTarget::Server);
}

Result invoke(const Api* mod_api, ModLifecycleCallback callback, void* user_data) {
    if (callback == nullptr) return RESULT_OK;
    try { return callback(mod_api, user_data); }
    catch (...) { return RESULT_CALLBACK_FAILED; }
}

Result invoke_update(const Api* mod_api, ModUpdateCallback callback, void* user_data, double delta_seconds) {
    if (callback == nullptr) return RESULT_OK;
    try { return callback(mod_api, user_data, delta_seconds); }
    catch (...) { return RESULT_CALLBACK_FAILED; }
}

Result CALL update_mod_setting(StringView control, uint8_t boolean, double, void* user_data) {
    auto* mod = static_cast<LoadedMod*>(user_data);
    if (!mod || std::string_view(control.data ? control.data : "", control.size) != "active" || !api->set_mod_setting_bool)
        return RESULT_INVALID_ARGUMENT;
    return api->set_mod_setting_bool({mod->id.data(), mod->id.size()}, {"active", 6}, boolean);
}

void register_settings_page(LoadedMod& mod, const json& manifest) {
    if (!api->register_ui_page) return;
    const auto name = manifest.value("name", mod.id);
    const auto description = manifest.value("description", std::string{});
    const bool active = Config::modGet<bool>(mod.id.c_str(), "active", false);
    static constexpr char Active[] = "active", ActiveLabel[] = "Mod aktiviert";
    static constexpr char ActiveDescription[] = "Aktiviert oder deaktiviert die Mod beim naechsten Loader-Tick.";
    static constexpr char General[] = "general", GeneralLabel[] = "Allgemein", PageId[] = "shroudtopia.settings";
    const UiControlDescriptor controls[]{
        {sizeof(UiControlDescriptor), {Active, sizeof(Active) - 1}, {ActiveLabel, sizeof(ActiveLabel) - 1},
            {ActiveDescription, sizeof(ActiveDescription) - 1},
            UI_CONTROL_BOOL, 0, 0, 1, 1, active ? uint8_t{1} : uint8_t{0}, {}, 0, UI_STATUS_NEUTRAL}
    };
    const UiTabDescriptor tabs[]{
        {sizeof(UiTabDescriptor), {General, sizeof(General) - 1}, {GeneralLabel, sizeof(GeneralLabel) - 1}, controls, std::size(controls)}
    };
    const UiPageDescriptor page{sizeof(UiPageDescriptor), {PageId, sizeof(PageId) - 1},
        {name.data(), name.size()}, {description.data(), description.size()}, 100, tabs, std::size(tabs),
        update_mod_setting, nullptr, &mod};
    const auto result = api->register_ui_page({mod.id.data(), mod.id.size()}, &page, &mod.settings_page);
    if (result != RESULT_OK) Utils::LogAs(LOG_WARNING, mod.id.c_str(), "Could not register Shroudforge page: result=%d", result);
}

void apply_manifest_defaults(const std::string& id, const json& manifest) {
    if (!manifest.contains("shroudtopia") || !manifest["shroudtopia"].contains("default")) return;
    if (!Config::jConfig.contains("mods") || !Config::jConfig["mods"].is_object()) {
        Config::jConfig["mods"] = json::object();
    }
    if (Config::jConfig["mods"].contains(id)) return;
    Config::jConfig["mods"][id] = manifest["shroudtopia"]["default"];
    if (!Config::jConfig["mods"][id].contains("active")) Config::jConfig["mods"][id]["active"] = true;
    Config::writeFile();
}

void load_mod(const fs::path& directory, const json& manifest) {
    const auto id = manifest.value("id", "");
    const auto version = manifest.value("version", "");
    if (id.empty() || version.empty() || !manifest.contains("shroudtopia")) {
        Utils::Log(LOG_ERROR, "Invalid mod manifest: %s", directory.string().c_str());
        return;
    }

    const auto& section = manifest["shroudtopia"];
    const auto api_name = section.value("api", "");
    const uint32_t requested_api_version = api_name == "1.2" ? API_VERSION :
        api_name == "1.1" ? UINT32_C(0x00010001) : 0;
    if (requested_api_version == 0 || section.value("entrypoint", "") != "CreateMod") {
        Utils::Log(LOG_ERROR, "Unsupported API or entrypoint for mod: %s", id.c_str());
        return;
    }
    const fs::path binary = section.value("binary", "");
    if (binary.empty() || binary.is_absolute() || binary.has_parent_path()) {
        Utils::Log(LOG_ERROR, "Missing or invalid shroudtopia.binary for mod: %s", id.c_str());
        return;
    }
    const auto target = section.value("target", "");
    if (target != "client" && target != "server" && target != "both") {
        Utils::Log(LOG_ERROR, "Missing or invalid shroudtopia.target for mod: %s", id.c_str());
        return;
    }
    if (!supports_target(target)) {
        Utils::Log(LOG_INFO, "Skipping mod %s: target is %s", id.c_str(), target.c_str());
        return;
    }
    const fs::path dll_path = directory / binary;
    const auto key = dll_path.lexically_normal().string();
    if (mods.contains(key) || !fs::is_regular_file(dll_path)) return;

    HMODULE module = LoadLibraryW(dll_path.c_str());
    if (module == nullptr) {
        Utils::Log(LOG_ERROR, "Failed to load DLL: %s", key.c_str());
        return;
    }

    const auto create_mod = reinterpret_cast<CreateModFunction>(GetProcAddress(module, "CreateMod"));
    if (create_mod == nullptr) {
        Utils::Log(LOG_ERROR, "CreateMod is missing: %s", key.c_str());
        FreeLibrary(module);
        return;
    }

    ModDescriptor descriptor{};
    descriptor.struct_size = sizeof(descriptor);
    const Api* mod_api = nullptr;
    if (ShroudtopiaGetApi(requested_api_version, &mod_api) != RESULT_OK || mod_api == nullptr) {
        Utils::Log(LOG_ERROR, "Requested API is unavailable for mod: %s", id.c_str());
        FreeLibrary(module);
        return;
    }
    Result result = RESULT_CALLBACK_FAILED;
    try { result = create_mod(requested_api_version, &descriptor); }
    catch (...) {}

    if (result != RESULT_OK || descriptor.struct_size < sizeof(descriptor) ||
        descriptor.mod_id.data == nullptr) {
        Utils::Log(LOG_ERROR, "Invalid API descriptor: %s", key.c_str());
        FreeLibrary(module);
        return;
    }

    const std::string descriptor_id(descriptor.mod_id.data, descriptor.mod_id.size);
    if (descriptor_id != id) {
        Utils::Log(LOG_ERROR, "Manifest and descriptor IDs differ for: %s", id.c_str());
        FreeLibrary(module);
        return;
    }

    apply_manifest_defaults(id, manifest);
    std::vector<std::string> capabilities;
    if (manifest.contains("requires") && manifest["requires"].is_object() &&
        manifest["requires"].contains("capabilities") && manifest["requires"]["capabilities"].is_array()) {
        for (const auto& capability : manifest["requires"]["capabilities"]) {
            if (capability.is_string()) capabilities.push_back(capability.get<std::string>());
        }
    }
    PlatformApi::GrantCapabilities(id, capabilities);
    const auto [entry, inserted] = mods.emplace(key, LoadedMod{module, descriptor, id, version, mod_api});
    if (inserted) register_settings_page(entry->second, manifest);
    Utils::Log(LOG_INFO, "Registered mod: %s v%s", id.c_str(), version.c_str());
    Utils::Log(LOG_DEBUG, "Mod manifest accepted: id=%s target=%s capabilities=%zu binary=%s",
        id.c_str(), target.c_str(), capabilities.size(), key.c_str());
}

void discover_mods() {
    const fs::path root = SHROUDTOPIA_MOD_FOLDER;
    std::error_code error;
    fs::create_directories(root, error);
    for (const auto& entry : fs::directory_iterator(root, error)) {
        if (error || !entry.is_directory()) continue;
        const auto manifest_path = entry.path() / "mod.json";
        if (!fs::is_regular_file(manifest_path)) continue;
        const auto manifest_key = manifest_path.lexically_normal().string();
        if (!discovered_manifests.insert(manifest_key).second) continue;
        Utils::Log(LOG_DEBUG, "Discovered mod manifest: %s", manifest_key.c_str());
        try {
            std::ifstream stream(manifest_path);
            load_mod(entry.path(), json::parse(stream));
        } catch (const std::exception& exception) {
            Utils::Log(LOG_ERROR, "Manifest error at %s: %s", manifest_path.string().c_str(), exception.what());
        }
    }
}

void update_mods(double delta_seconds) {
    for (auto& [path, mod] : mods) {
        if (mod.failed) continue;
        if (!mod.loaded) {
            const auto started = std::chrono::steady_clock::now();
            const auto result = invoke(mod.mod_api, mod.descriptor.on_load, mod.descriptor.user_data);
            Utils::LogAs(LOG_DEBUG, mod.id.c_str(), "on_load: result=%d duration=%.3f ms",
                static_cast<int>(result), elapsed_ms(started));
            if (result != RESULT_OK) {
                Utils::LogAs(LOG_ERROR, mod.id.c_str(), "Load failed: result=%d", static_cast<int>(result));
                mod.failed = true;
                continue;
            }
            mod.loaded = true;
            Utils::LogAs(LOG_INFO, mod.id.c_str(), "Loaded");
        }

        const bool should_activate = Config::modGet<bool>(mod.id.c_str(), "active", false);
        if (should_activate && !mod.active) {
            const auto started = std::chrono::steady_clock::now();
            const auto result = invoke(mod.mod_api, mod.descriptor.on_activate, mod.descriptor.user_data);
            Utils::LogAs(LOG_DEBUG, mod.id.c_str(), "on_activate: result=%d duration=%.3f ms",
                static_cast<int>(result), elapsed_ms(started));
            if (result != RESULT_OK) {
                Utils::LogAs(LOG_ERROR, mod.id.c_str(), "Activation failed: result=%d", static_cast<int>(result));
                mod.failed = true;
                continue;
            }
            mod.active = true;
            Utils::LogAs(LOG_INFO, mod.id.c_str(), "Activated");
        } else if (!should_activate && mod.active) {
            const auto started = std::chrono::steady_clock::now();
            const auto result = invoke(mod.mod_api, mod.descriptor.on_deactivate, mod.descriptor.user_data);
            Utils::LogAs(LOG_DEBUG, mod.id.c_str(), "on_deactivate: result=%d duration=%.3f ms",
                static_cast<int>(result), elapsed_ms(started));
            if (result != RESULT_OK) {
                Utils::LogAs(LOG_ERROR, mod.id.c_str(), "Deactivation failed: result=%d", static_cast<int>(result));
                continue;
            }
            mod.active = false;
            Utils::LogAs(LOG_INFO, mod.id.c_str(), "Deactivated");
        }

        if (mod.active && mod.descriptor.on_update != nullptr) {
            const auto result = invoke_update(mod.mod_api, mod.descriptor.on_update, mod.descriptor.user_data, delta_seconds);
            if (result != RESULT_OK) Utils::LogAs(LOG_ERROR, mod.id.c_str(), "Update failed: result=%d", static_cast<int>(result));
        }
    }
}

void deactivate_mods() {
    for (auto& [path, mod] : mods) {
        if (!mod.active) continue;
        const auto result = invoke(mod.mod_api, mod.descriptor.on_deactivate, mod.descriptor.user_data);
        if (result != RESULT_OK) {
            Utils::LogAs(LOG_ERROR, mod.id.c_str(), "Deactivation failed: result=%d", static_cast<int>(result));
        }
        mod.active = false;
    }
}

void unload_mods() {
    for (auto& [path, mod] : mods) {
        if (mod.active) invoke(mod.mod_api, mod.descriptor.on_deactivate, mod.descriptor.user_data);
        if (mod.loaded) invoke(mod.mod_api, mod.descriptor.on_unload, mod.descriptor.user_data);
        PlatformApi::ReleaseOwner(mod.id);
        if (mod.module != nullptr) FreeLibrary(mod.module);
    }
    mods.clear();
    discovered_manifests.clear();
}

DWORD WINAPI run(LPVOID) {
    Utils::BeginLogSession(target_name(current_target()));
    if (ShroudtopiaGetApi(API_VERSION, &api) != RESULT_OK) return 1;
    if (!Config::readFile()) {
        Config::setConfigFromJSON(default_config);
        Config::writeFile();
    } else if (!Config::jConfig.contains("logLevel")) {
        Config::jConfig["logLevel"] = "INFO";
        Config::writeFile();
    }
    Utils::Log(LOG_INFO, "Starting %s-%s", SHROUDTOPIA_VERSION, SHROUDTOPIA_BUILD_NUMBER);
    Utils::Log(LOG_DEBUG, "Runtime initialized: process=%s updateDelay=%dms modsDirectory=%s",
        target_name(current_target()), Config::get<int>("updateDelay", 500), SHROUDTOPIA_MOD_FOLDER);
    Utils::Log(LOG_DEBUG, "API ready: version=1.2");
    if (current_target() == ProcessTarget::Client) {
        const auto uiResult = ShroudforgeUi::Initialize(api);
        if (uiResult != RESULT_OK) Utils::Log(LOG_WARNING, "Shroudforge UI unavailable: result=%d", static_cast<int>(uiResult));
    }
    const auto worldResult=WorldEngine::Initialize(api);
    if (worldResult!=RESULT_OK) Utils::Log(LOG_WARNING,"Native world service unavailable: result=%d",static_cast<int>(worldResult));
    WorldDiagnostic::Start(Config::get<bool>("worldDiagnostic",false));

    while (WaitForSingleObject(stop_event, 0) != WAIT_OBJECT_0) {
        if (Config::reloadIfChanged()) {
            Utils::Log(LOG_DEBUG, "Configuration reloaded from %s", SHROUDTOPIA_CONFIG_FILE);
        }
        const int update_delay = (std::max)(Config::get<int>("updateDelay", 500), 1);
        WorldDiagnostic::Tick(Config::get<bool>("worldDiagnostic",false) && Config::get<bool>("active",true));
        if (current_target() == ProcessTarget::Client) ShroudforgeUi::Tick();
        if (worldResult==RESULT_OK) WorldEngine::Tick();
        if (Config::get<bool>("active", true)) {
            discover_mods();
            update_mods(static_cast<double>(update_delay) / 1000.0);
        } else {
            deactivate_mods();
        }
        if (WaitForSingleObject(stop_event, static_cast<DWORD>(update_delay)) == WAIT_OBJECT_0) break;
    }

    WorldDiagnostic::Tick(false);
    unload_mods();
    if (current_target() == ProcessTarget::Client) ShroudforgeUi::Shutdown();
    WorldEngine::Shutdown(api);
    Utils::Log(LOG_DEBUG, "Runtime shutdown: all mods unloaded");
    PlatformApi::Shutdown();
    api = nullptr;
    return 0;
}
}

namespace Runtime {
BOOL Start() {
    if (runtime_thread != nullptr) return TRUE;
    stop_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (stop_event == nullptr) return FALSE;
    runtime_thread = CreateThread(nullptr, 0, run, nullptr, 0, nullptr);
    if (runtime_thread != nullptr) return TRUE;
    CloseHandle(stop_event);
    stop_event = nullptr;
    return FALSE;
}

BOOL Stop(DWORD timeout_milliseconds) {
    if (runtime_thread == nullptr) return TRUE;
    SetEvent(stop_event);
    if (WaitForSingleObject(runtime_thread, timeout_milliseconds) != WAIT_OBJECT_0) return FALSE;
    CloseHandle(runtime_thread);
    CloseHandle(stop_event);
    runtime_thread = nullptr;
    stop_event = nullptr;
    return TRUE;
}
}
