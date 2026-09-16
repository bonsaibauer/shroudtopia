#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shroudtopia/api.h"

#include <cstring>
#include <string_view>

namespace {
    constexpr char ModId[] = "mod.commands";
    constexpr char ExecuteCommandId[] = "mod.commands.execute";

    ST_StringView View(const char* value) { return {value, std::strlen(value)}; }

    struct State {
        const ST_HostApiV1* api = nullptr;
        ST_Registration execute_command = 0;
        ST_Registration status_command = 0;
        ST_Registration settings = 0;
    } state;

    ST_Result ST_CALL ExecuteRegisteredCommand(ST_StringView arguments, void*) {
        if (state.api == nullptr || arguments.data == nullptr) return ST_RESULT_INVALID_ARGUMENT;

        const std::string_view input(arguments.data, arguments.size);
        const size_t separator = input.find_first_of(" \t\r\n");
        const std::string_view commandId = input.substr(0, separator);
        if (commandId.empty() || commandId == ExecuteCommandId) return ST_RESULT_INVALID_ARGUMENT;

        const size_t argumentStart = separator == std::string_view::npos
            ? input.size()
            : input.find_first_not_of(" \t\r\n", separator);
        const std::string_view commandArguments = argumentStart == std::string_view::npos
            ? std::string_view{}
            : input.substr(argumentStart);

        return state.api->execute_command(
            View(ModId),
            {commandId.data(), commandId.size()},
            {commandArguments.data(), commandArguments.size()});
    }

    ST_Result ST_CALL ReportStatus(ST_StringView, void*) {
        return state.api->log(View(ModId), ST_LOG_INFO,
            View("Shroudtopia Commands is active and routes registered mod commands."));
    }

    ST_Result ST_CALL Load(const ST_HostApiV1* api, void*) {
        if (api == nullptr || api->abi_version != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
        state.api = api;

        const ST_SettingsDescriptor settings{
            sizeof(ST_SettingsDescriptor), View("mod.commands.settings"),
            View("{\"type\":\"object\",\"additionalProperties\":false}"),
            View("{}")
        };
        return api->register_settings(View(ModId), &settings, &state.settings);
    }

    ST_Result ST_CALL Activate(const ST_HostApiV1* api, void*) {
        if (api == nullptr) return ST_RESULT_INVALID_ARGUMENT;
        const ST_CommandDescriptor execute{
            sizeof(ST_CommandDescriptor), View(ExecuteCommandId),
            View("Executes a command registered by any active mod."),
            ExecuteRegisteredCommand, nullptr
        };
        ST_Result result = api->register_command(View(ModId), &execute, &state.execute_command);
        if (result != ST_RESULT_OK) return result;

        const ST_CommandDescriptor status{
            sizeof(ST_CommandDescriptor), View("mod.commands.status"),
            View("Reports whether the bundled Commands mod is active."),
            ReportStatus, nullptr
        };
        result = api->register_command(View(ModId), &status, &state.status_command);
        if (result != ST_RESULT_OK) {
            api->release_registration(state.execute_command);
            state.execute_command = 0;
        }
        return result;
    }

    ST_Result ST_CALL Update(const ST_HostApiV1*, void*, double) { return ST_RESULT_OK; }

    ST_Result ST_CALL Deactivate(const ST_HostApiV1* api, void*) {
        if (api == nullptr) return ST_RESULT_INVALID_ARGUMENT;
        if (state.execute_command != 0) api->release_registration(state.execute_command);
        if (state.status_command != 0) api->release_registration(state.status_command);
        state.execute_command = 0;
        state.status_command = 0;
        return ST_RESULT_OK;
    }

    ST_Result ST_CALL Unload(const ST_HostApiV1* api, void*) {
        if (api == nullptr) return ST_RESULT_INVALID_ARGUMENT;
        const ST_Result result = api->release_owner(View(ModId));
        state = {};
        return result;
    }
}

extern "C" __declspec(dllexport) ST_Result ST_CALL ShroudtopiaCreateModV1(
    uint32_t requestedAbi, ST_ModDescriptorV1* descriptor) {
    if (requestedAbi != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    if (descriptor == nullptr || descriptor->struct_size < sizeof(ST_ModDescriptorV1)) return ST_RESULT_INVALID_ARGUMENT;

    *descriptor = {
        sizeof(ST_ModDescriptorV1), View(ModId), nullptr,
        Load, Activate, Update, Deactivate, Unload
    };
    return ST_RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}

