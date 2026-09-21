#include <windows.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "shroudtopia.h"

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

    Result CALL CountUiPage(StringView ownerId, UiPageDescriptor* page, void* userData) {
        if (ownerId.data == nullptr || page == nullptr || page->struct_size < sizeof(UiPageDescriptor))
            return RESULT_CALLBACK_FAILED;
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
    HMODULE module = LoadLibraryW(L"winmm.dll");
    if (module == nullptr) {
        std::cerr << "Could not load winmm.dll: " << GetLastError() << '\n';
        return 1;
    }

    using GetApi = Result (CALL*)(uint32_t, const Api**);
    using Stop = BOOL (__cdecl*)(DWORD);
    const auto getApi = reinterpret_cast<GetApi>(GetProcAddress(module, "ShroudtopiaGetApi"));
    const auto stop = reinterpret_cast<Stop>(GetProcAddress(module, "ShroudtopiaStop"));
    if (getApi == nullptr || stop == nullptr) return 2;

    const Api* api = nullptr;
    if (!Check(getApi(API_VERSION, &api), RESULT_OK, "ShroudtopiaGetApi") || api == nullptr) return 3;
    const Api* api11 = nullptr;
    if (!Check(getApi(UINT32_C(0x00010001), &api11), RESULT_OK, "ShroudtopiaGetApi 1.1") ||
        api11 == nullptr || api11->api_version != UINT32_C(0x00010001) ||
        api11->struct_size != offsetof(Api, set_mod_setting_bool)) return 3;
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
    if (!Check(api->query_capability(View(CAPABILITY_ASSETS_READ), &capability), RESULT_OK,
        "query assets without game files") || capability.available != 0) return 4;
    uint8_t allowed = 1;
    if (!Check(api->check_permission(View("mod.smoke"), View("shroudtopia.world.voxels.write"), &allowed), RESULT_OK, "check permission") || allowed != 0) return 4;

    RuntimePatch deniedPatch = 0;
    RuntimePatchOptions deniedDescriptor{sizeof(deniedDescriptor), View("90"), 0, RUNTIME_PATCH_DIRECT, 1, nullptr, 0, nullptr, 0};
    if (!Check(api->create_patch(View("mod.smoke"), &deniedDescriptor, &deniedPatch), RESULT_PERMISSION_DENIED,
        "reject runtime patch without manifest permission")) return 4;

    const auto owner = View("mod.smoke");

    if (!api->set_mod_setting_bool || !api->set_mod_setting_number ||
        !api->register_ui_page || !api->visit_ui_pages) return 4;
    if (!Check(api->set_mod_setting_bool(owner, View("uiSmokeBool"), 1), RESULT_OK,
        "set bool setting")) return 4;
    uint8_t savedBool = 0;
    if (!Check(api->get_mod_setting_bool(owner, View("uiSmokeBool"), 0, &savedBool), RESULT_OK,
        "get bool setting") || savedBool != 1) return 4;
    const UiControlDescriptor uiControl{sizeof(UiControlDescriptor), View("enabled"), View("Enabled"),
        View("Smoke UI control"), UI_CONTROL_BOOL, 0, 0, 1, 1, 1, {}, 0, UI_STATUS_NEUTRAL};
    const UiTabDescriptor uiTab{sizeof(UiTabDescriptor), View("general"), View("General"), &uiControl, 1};
    const UiPageDescriptor uiPage{sizeof(UiPageDescriptor), View("smoke"), View("Smoke"),
        View("Smoke UI page"), 50, &uiTab, 1, nullptr, nullptr, nullptr};
    Registration uiHandle = 0;
    if (!Check(api->register_ui_page(owner, &uiPage, &uiHandle), RESULT_OK, "register UI page")) return 4;
    size_t visitedPages = 0;
    if (!Check(api->visit_ui_pages(owner, CountUiPage, &visitedPages), RESULT_OK,
        "visit UI pages") || visitedPages == 0) return 4;

    size_t visitedAssets = 0;
    if (!Check(api->list_assets(owner, View("keen::RecipeRegistryResource"), CountAsset, &visitedAssets),
        RESULT_PERMISSION_DENIED, "reject asset read without manifest permission")) return 4;
    const auto core = View("shroudtopia.core");
    if (!Check(api->list_assets(core, View("keen::RecipeRegistryResource"), CountAsset, &visitedAssets),
        RESULT_NOT_FOUND, "report unavailable internal asset engine")) return 4;

    std::int32_t serviceValue = 42;
    ServiceDescriptor service{sizeof(service), View("mod.smoke.echo"), 1, 2, &serviceValue};
    Registration serviceHandle = 0;
    if (!Check(api->register_service(owner, &service, &serviceHandle), RESULT_OK, "register_service")) return 4;

    const void* found = nullptr;
    ServiceRequest request{sizeof(request), View("mod.smoke.echo"), 1, 2};
    if (!Check(api->find_service(&request, &found), RESULT_OK, "find_service") || found != &serviceValue) return 5;
    request.version_minor = 1;
    if (!Check(api->find_service(&request, &found), RESULT_NOT_FOUND, "reject different service version")) return 5;

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

    if (!Check(api->release_registration(serviceHandle), RESULT_OK, "release service")) return 13;
    if (!Check(api->release_registration(uiHandle), RESULT_OK, "release UI page")) return 13;
    if (!Check(api->find_service(&request, &found), RESULT_NOT_FOUND, "find_service after release")) return 14;

    if (!stop(10000)) return 15;
    FreeLibrary(module);
    std::cout << "Shroudtopia platform API smoke test passed.\n";
    return 0;
}
