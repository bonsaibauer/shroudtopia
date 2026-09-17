#include <windows.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "shroudtopia/api.h"

namespace {
    StringView View(const char* value) {
        return {value, std::strlen(value)};
    }

    Result CALL OnEvent(const Event* eventData, void* userData) {
        if (eventData == nullptr || eventData->payload_size != sizeof(std::int32_t)) return RESULT_CALLBACK_FAILED;
        *static_cast<std::int32_t*>(userData) = *static_cast<const std::int32_t*>(eventData->payload);
        return RESULT_OK;
    }

    Result CALL OnCommand(StringView arguments, void* userData) {
        *static_cast<size_t*>(userData) = arguments.size;
        return RESULT_OK;
    }

    Result CALL OnAction(StringView inputJson, void* userData) {
        *static_cast<size_t*>(userData) = inputJson.size;
        return RESULT_OK;
    }

    Result CALL CountAsset(const AssetId* resource, void* userData) {
        if (resource == nullptr || resource->struct_size < sizeof(AssetId)) return RESULT_CALLBACK_FAILED;
        ++*static_cast<size_t*>(userData);
        return RESULT_OK;
    }

    bool Check(Result actual, Result expected, const char* operation) {
        if (actual == expected) return true;
        std::cerr << operation << " returned " << actual << ", expected " << expected << '\n';
        return false;
    }

    bool SetCommandsActive(bool active) {
        std::ifstream input("shroudtopia.json", std::ios::binary);
        std::string config((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        const size_t owner = config.find("\"mod.commands\"");
        const size_t key = owner == std::string::npos ? owner : config.find("\"active\"", owner);
        const size_t value = key == std::string::npos ? key : config.find_first_not_of(" \t\r\n:", key + 8);
        if (value == std::string::npos) return false;
        const std::string previous = config.compare(value, 4, "true") == 0
            ? "true"
            : config.compare(value, 5, "false") == 0 ? "false" : "";
        if (previous.empty()) return false;
        config.replace(value, previous.size(), active ? "true" : "false");
        std::ofstream output("shroudtopia.json", std::ios::binary | std::ios::trunc);
        output << config;
        return output.good();
    }

    bool SetLoaderActive(bool active) {
        std::ifstream input("shroudtopia.json", std::ios::binary);
        std::string config((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        const size_t key = config.find("\"active\"");
        const size_t value = key == std::string::npos ? key : config.find_first_not_of(" \t\r\n:", key + 8);
        if (value == std::string::npos) return false;
        const std::string previous = config.compare(value, 4, "true") == 0
            ? "true"
            : config.compare(value, 5, "false") == 0 ? "false" : "";
        if (previous.empty()) return false;
        config.replace(value, previous.size(), active ? "true" : "false");
        std::ofstream output("shroudtopia.json", std::ios::binary | std::ios::trunc);
        output << config;
        return output.good();
    }
}

int main() {
    HMODULE module = LoadLibraryW(L"shroudtopia.dll");
    if (module == nullptr) {
        std::cerr << "Could not load shroudtopia.dll: " << GetLastError() << '\n';
        return 1;
    }

    using GetApi = Result (CALL*)(uint32_t, const Api**);
    using Stop = BOOL (__cdecl*)(DWORD);
    const auto getApi = reinterpret_cast<GetApi>(GetProcAddress(module, "ShroudtopiaGetApi"));
    const auto stop = reinterpret_cast<Stop>(GetProcAddress(module, "ShroudtopiaStop"));
    if (getApi == nullptr || stop == nullptr) return 2;

    const Api* api = nullptr;
    if (!Check(getApi(API_VERSION, &api), RESULT_OK, "ShroudtopiaGetApi") || api == nullptr) return 3;
    if (api->struct_size < offsetof(Api, log) + sizeof(api->log) || api->log == nullptr) return 3;
    if (!Check(api->log(View("mod.smoke"), LOG_INFO, View("smoke test")), RESULT_OK, "log")) return 3;
    if (api->struct_size < offsetof(Api, get_mod_setting_number) + sizeof(api->get_mod_setting_number) || !api->get_mod_setting_number) return 3;
    double numeric = 0;
    if (!Check(api->get_mod_setting_number(View("mod.smoke"), View("missingNumericSetting"), 123.5, &numeric),
        RESULT_OK, "numeric setting fallback") || numeric != 123.5) return 3;
    if (!Check(api->get_mod_setting_number(View("mod.smoke"), View("missingNumericSetting"), 1, nullptr),
        RESULT_INVALID_ARGUMENT, "numeric setting null output")) return 3;

    CapabilityInfo capability{sizeof(capability)};
    if (!Check(api->query_capability(View("shroudtopia.registry.services"), &capability), RESULT_OK, "query available capability") || capability.available != 1) return 4;
    capability = {sizeof(capability)};
    if (!Check(api->query_capability(View(CAPABILITY_RUNTIME_PATCHES), &capability), RESULT_OK, "query runtime patches capability") || capability.available != 1) return 4;
    capability = {sizeof(capability)};
    if (!Check(api->query_capability(View("shroudtopia.world.voxels.write"), &capability), RESULT_OK, "query unavailable capability") || capability.available != 0) return 4;
    capability = {sizeof(capability)};
    if (!Check(api->query_capability(View(CAPABILITY_ASSETS_READ), &capability), RESULT_OK,
        "query assets without game files") || capability.available != 0) return 4;
    uint8_t allowed = 1;
    if (!Check(api->check_permission(View("mod.smoke"), View("shroudtopia.world.voxels.write"), &allowed), RESULT_OK, "check permission") || allowed != 0) return 4;

    const auto* runtime = api->patches;
    if (runtime == nullptr) return 4;
    RuntimePatch deniedPatch = 0;
    RuntimePatchOptions deniedDescriptor{sizeof(deniedDescriptor), View("90"), 0, RUNTIME_PATCH_DIRECT, 1, nullptr, 0, nullptr, 0};
    if (!Check(runtime->create(View("mod.smoke"), &deniedDescriptor, &deniedPatch), RESULT_PERMISSION_DENIED,
        "reject runtime patch without manifest permission")) return 4;

    const auto owner = View("mod.smoke");

    const auto* assets = api->assets;
    if (assets == nullptr) return 4;
    size_t visitedAssets = 0;
    if (!Check(assets->list(owner, View("keen::RecipeRegistryResource"), CountAsset, &visitedAssets),
        RESULT_PERMISSION_DENIED, "reject asset read without manifest permission")) return 4;
    const auto core = View("shroudtopia.core");
    if (!Check(assets->list(core, View("keen::RecipeRegistryResource"), CountAsset, &visitedAssets),
        RESULT_NOT_FOUND, "report unavailable internal asset engine")) return 4;

    std::int32_t serviceValue = 42;
    ServiceDescriptor service{sizeof(service), View("mod.smoke.echo"), 1, 2, &serviceValue};
    Registration serviceHandle = 0;
    if (!Check(api->register_service(owner, &service, &serviceHandle), RESULT_OK, "register_service")) return 4;

    const void* found = nullptr;
    ServiceRequest request{sizeof(request), View("mod.smoke.echo"), 1, 1};
    if (!Check(api->find_service(&request, &found), RESULT_OK, "find_service") || found != &serviceValue) return 5;

    std::int32_t eventValue = 0;
    EventSubscription subscription{sizeof(subscription), View("mod.smoke.changed"), OnEvent, &eventValue};
    Registration eventHandle = 0;
    if (!Check(api->subscribe_event(owner, &subscription, &eventHandle), RESULT_OK, "subscribe_event")) return 6;
    const std::int32_t payload = 7;
    Event eventData{sizeof(eventData), View("mod.smoke.changed"), &payload, sizeof(payload)};
    if (!Check(api->publish_event(owner, &eventData), RESULT_OK, "publish_event") || eventValue != payload) return 7;

    size_t argumentLength = 0;
    CommandDescriptor command{sizeof(command), View("mod.smoke.run"), View("Smoke command"), OnCommand, &argumentLength};
    Registration commandHandle = 0;
    if (!Check(api->register_command(owner, &command, &commandHandle), RESULT_OK, "register_command")) return 8;
    if (!Check(api->invoke_command(owner, View("mod.smoke.run"), View("abc")), RESULT_OK, "invoke_command") || argumentLength != 3) return 9;

    ModSettingsDescriptor settings{sizeof(settings), View("mod.smoke.settings"), View("{\"type\":\"object\"}"), View("{}")};
    Registration settingsHandle = 0;
    if (!Check(api->register_mod_settings(owner, &settings, &settingsHandle), RESULT_OK, "register_mod_settings")) return 10;

    size_t actionInputLength = 0;
    Action action{sizeof(action), View("mod.smoke.toggle"), View("Toggle smoke feature"),
        View("Exercises the shared in-game action registry."), View("{\"type\":\"object\"}"),
        OnAction, &actionInputLength};
    Registration actionHandle = 0;
    if (!Check(api->register_action(owner, &action, &actionHandle), RESULT_OK, "register_action")) return 10;
    ActionState actionState = ACTION_FAILED;
    if (!Check(api->get_action_state(View("mod.smoke.toggle"), &actionState), RESULT_OK,
        "get_action_state") || actionState != ACTION_AVAILABLE) return 10;
    if (!Check(api->invoke_action(owner, View("mod.smoke.toggle"), View("{}")), RESULT_OK,
        "invoke_action") || actionInputLength != 2) return 10;

    if (!Check(api->stage_game_setting(View("smoke.profile"), View("\"test\"")), RESULT_OK,
        "stage_game_setting")) return 10;
    if (!Check(api->reset_game_setting(View("smoke.profile")), RESULT_OK,
        "reset_game_setting")) return 10;

    Result commandsResult = RESULT_NOT_FOUND;
    for (int attempt = 0; attempt < 60 && commandsResult == RESULT_NOT_FOUND; ++attempt) {
        Sleep(100);
        commandsResult = api->invoke_command(
            owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, RESULT_OK, "execute command through bundled Commands mod")) return 11;

    if (!SetCommandsActive(false)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == RESULT_OK; ++attempt) {
        Sleep(100);
        commandsResult = api->invoke_command(
            owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, RESULT_NOT_FOUND, "disable bundled Commands mod")) return 11;

    if (!SetCommandsActive(true)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == RESULT_NOT_FOUND; ++attempt) {
        Sleep(100);
        commandsResult = api->invoke_command(
            owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, RESULT_OK, "reactivate bundled Commands mod")) return 11;

    if (!SetLoaderActive(false)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == RESULT_OK; ++attempt) {
        Sleep(100);
        commandsResult = api->invoke_command(owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, RESULT_NOT_FOUND, "deactivate mods with global loader switch")) return 11;

    if (!SetLoaderActive(true)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == RESULT_NOT_FOUND; ++attempt) {
        Sleep(100);
        commandsResult = api->invoke_command(owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, RESULT_OK, "reactivate mods with global loader switch")) return 11;

    if (!Check(api->release_owner(owner), RESULT_OK, "release_owner")) return 13;
    if (!Check(api->find_service(&request, &found), RESULT_NOT_FOUND, "find_service after release")) return 14;

    if (!stop(10000)) return 15;
    FreeLibrary(module);
    std::cout << "Shroudtopia platform API smoke test passed.\n";
    return 0;
}
