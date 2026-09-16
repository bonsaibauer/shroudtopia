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
    ST_StringView View(const char* value) {
        return {value, std::strlen(value)};
    }

    ST_Result ST_CALL OnEvent(const ST_Event* eventData, void* userData) {
        if (eventData == nullptr || eventData->payload_size != sizeof(std::int32_t)) return ST_RESULT_CALLBACK_FAILED;
        *static_cast<std::int32_t*>(userData) = *static_cast<const std::int32_t*>(eventData->payload);
        return ST_RESULT_OK;
    }

    ST_Result ST_CALL OnCommand(ST_StringView arguments, void* userData) {
        *static_cast<size_t*>(userData) = arguments.size;
        return ST_RESULT_OK;
    }

    ST_Result ST_CALL CountAsset(const ST_AssetResourceKeyV1* resource, void* userData) {
        if (resource == nullptr || resource->struct_size < sizeof(ST_AssetResourceKeyV1)) return ST_RESULT_CALLBACK_FAILED;
        ++*static_cast<size_t*>(userData);
        return ST_RESULT_OK;
    }

    bool Check(ST_Result actual, ST_Result expected, const char* operation) {
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

    using GetApi = ST_Result (ST_CALL*)(uint32_t, const ST_HostApiV1**);
    using Stop = BOOL (__cdecl*)(DWORD);
    const auto getApi = reinterpret_cast<GetApi>(GetProcAddress(module, "ShroudtopiaGetApi"));
    const auto stop = reinterpret_cast<Stop>(GetProcAddress(module, "ShroudtopiaStop"));
    if (getApi == nullptr || stop == nullptr) return 2;

    const ST_HostApiV1* api = nullptr;
    if (!Check(getApi(ST_ABI_VERSION_1, &api), ST_RESULT_OK, "ShroudtopiaGetApi") || api == nullptr) return 3;
    if (api->struct_size < offsetof(ST_HostApiV1, log) + sizeof(api->log) || api->log == nullptr) return 3;
    if (!Check(api->log(View("mod.smoke"), ST_LOG_INFO, View("smoke test")), ST_RESULT_OK, "log")) return 3;
    if (api->struct_size < offsetof(ST_HostApiV1, get_setting_number) + sizeof(api->get_setting_number) || !api->get_setting_number) return 3;
    double numeric = 0;
    if (!Check(api->get_setting_number(View("mod.smoke"), View("missingNumericSetting"), 123.5, &numeric),
        ST_RESULT_OK, "numeric setting fallback") || numeric != 123.5) return 3;
    if (!Check(api->get_setting_number(View("mod.smoke"), View("missingNumericSetting"), 1, nullptr),
        ST_RESULT_INVALID_ARGUMENT, "numeric setting null output")) return 3;

    ST_CapabilityInfoV1 capability{sizeof(capability)};
    if (!Check(api->query_capability(View("shroudtopia.registry.services"), &capability), ST_RESULT_OK, "query available capability") || capability.available != 1) return 4;
    capability = {sizeof(capability)};
    if (!Check(api->query_capability(View(ST_CAPABILITY_RUNTIME_PATCHES), &capability), ST_RESULT_OK, "query runtime patches capability") || capability.available != 1) return 4;
    capability = {sizeof(capability)};
    if (!Check(api->query_capability(View("shroudtopia.world.voxels.write"), &capability), ST_RESULT_OK, "query unavailable capability") || capability.available != 0) return 4;
    capability = {sizeof(capability)};
    if (!Check(api->query_capability(View(ST_CAPABILITY_ASSETS_READ), &capability), ST_RESULT_OK,
        "query assets without game files") || capability.available != 0) return 4;
    uint8_t allowed = 1;
    if (!Check(api->check_permission(View("mod.smoke"), View("shroudtopia.world.voxels.write"), &allowed), ST_RESULT_OK, "check permission") || allowed != 0) return 4;

    const void* runtimeService = nullptr;
    ST_ServiceRequest runtimeRequest{sizeof(runtimeRequest), View(ST_RUNTIME_PATCHES_SERVICE_ID), 1, 0};
    if (!Check(api->find_service(&runtimeRequest, &runtimeService), ST_RESULT_OK, "find runtime patch service") || runtimeService == nullptr) return 4;
    const auto* runtime = static_cast<const ST_RuntimePatchesApiV1*>(runtimeService);
    ST_RuntimePatch deniedPatch = 0;
    ST_RuntimePatchDescriptorV1 deniedDescriptor{sizeof(deniedDescriptor), View("90"), 0, ST_RUNTIME_PATCH_DIRECT, 1, nullptr, 0, nullptr, 0};
    if (!Check(runtime->create(View("mod.smoke"), &deniedDescriptor, &deniedPatch), ST_RESULT_PERMISSION_DENIED,
        "reject runtime patch without manifest permission")) return 4;

    const auto owner = View("mod.smoke");

    const void* assetsService = nullptr;
    ST_ServiceRequest assetsRequest{sizeof(assetsRequest), View(ST_ASSETS_SERVICE_ID),
        ST_ASSETS_SERVICE_VERSION_MAJOR, ST_ASSETS_SERVICE_VERSION_MINOR};
    if (!Check(api->find_service(&assetsRequest, &assetsService), ST_RESULT_OK,
        "find assets facade") || assetsService == nullptr) return 4;
    const auto* assets = static_cast<const ST_AssetsApiV1*>(assetsService);
    size_t visitedAssets = 0;
    if (!Check(assets->visit_resources(owner, View("keen::RecipeRegistryResource"), CountAsset, &visitedAssets),
        ST_RESULT_PERMISSION_DENIED, "reject asset read without manifest permission")) return 4;
    const auto core = View("shroudtopia.core");
    if (!Check(assets->visit_resources(core, View("keen::RecipeRegistryResource"), CountAsset, &visitedAssets),
        ST_RESULT_NOT_FOUND, "report unavailable internal asset engine")) return 4;

    std::int32_t serviceValue = 42;
    ST_ServiceDescriptor service{sizeof(service), View("mod.smoke.echo"), 1, 2, &serviceValue};
    ST_Registration serviceHandle = 0;
    if (!Check(api->register_service(owner, &service, &serviceHandle), ST_RESULT_OK, "register_service")) return 4;

    const void* found = nullptr;
    ST_ServiceRequest request{sizeof(request), View("mod.smoke.echo"), 1, 1};
    if (!Check(api->find_service(&request, &found), ST_RESULT_OK, "find_service") || found != &serviceValue) return 5;

    std::int32_t eventValue = 0;
    ST_EventSubscription subscription{sizeof(subscription), View("mod.smoke.changed"), OnEvent, &eventValue};
    ST_Registration eventHandle = 0;
    if (!Check(api->subscribe_event(owner, &subscription, &eventHandle), ST_RESULT_OK, "subscribe_event")) return 6;
    const std::int32_t payload = 7;
    ST_Event eventData{sizeof(eventData), View("mod.smoke.changed"), &payload, sizeof(payload)};
    if (!Check(api->publish_event(owner, &eventData), ST_RESULT_OK, "publish_event") || eventValue != payload) return 7;

    size_t argumentLength = 0;
    ST_CommandDescriptor command{sizeof(command), View("mod.smoke.run"), View("Smoke command"), OnCommand, &argumentLength};
    ST_Registration commandHandle = 0;
    if (!Check(api->register_command(owner, &command, &commandHandle), ST_RESULT_OK, "register_command")) return 8;
    if (!Check(api->execute_command(owner, View("mod.smoke.run"), View("abc")), ST_RESULT_OK, "execute_command") || argumentLength != 3) return 9;

    ST_SettingsDescriptor settings{sizeof(settings), View("mod.smoke.settings"), View("{\"type\":\"object\"}"), View("{}")};
    ST_Registration settingsHandle = 0;
    if (!Check(api->register_settings(owner, &settings, &settingsHandle), ST_RESULT_OK, "register_settings")) return 10;

    ST_Result commandsResult = ST_RESULT_NOT_FOUND;
    for (int attempt = 0; attempt < 60 && commandsResult == ST_RESULT_NOT_FOUND; ++attempt) {
        Sleep(100);
        commandsResult = api->execute_command(
            owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, ST_RESULT_OK, "execute command through bundled Commands mod")) return 11;

    if (!SetCommandsActive(false)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == ST_RESULT_OK; ++attempt) {
        Sleep(100);
        commandsResult = api->execute_command(
            owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, ST_RESULT_NOT_FOUND, "disable bundled Commands mod")) return 11;

    if (!SetCommandsActive(true)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == ST_RESULT_NOT_FOUND; ++attempt) {
        Sleep(100);
        commandsResult = api->execute_command(
            owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, ST_RESULT_OK, "reactivate bundled Commands mod")) return 11;

    if (!SetLoaderActive(false)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == ST_RESULT_OK; ++attempt) {
        Sleep(100);
        commandsResult = api->execute_command(owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, ST_RESULT_NOT_FOUND, "deactivate mods with global loader switch")) return 11;

    if (!SetLoaderActive(true)) return 11;
    for (int attempt = 0; attempt < 60 && commandsResult == ST_RESULT_NOT_FOUND; ++attempt) {
        Sleep(100);
        commandsResult = api->execute_command(owner, View("mod.commands.execute"), View("mod.smoke.run routed"));
    }
    if (!Check(commandsResult, ST_RESULT_OK, "reactivate mods with global loader switch")) return 11;

    if (!Check(api->release_owner(owner), ST_RESULT_OK, "release_owner")) return 13;
    if (!Check(api->find_service(&request, &found), ST_RESULT_NOT_FOUND, "find_service after release")) return 14;

    if (!stop(10000)) return 15;
    FreeLibrary(module);
    std::cout << "Shroudtopia platform API smoke test passed.\n";
    return 0;
}

