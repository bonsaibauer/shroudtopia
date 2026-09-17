#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shroudtopia.h"

#include <cstring>
#include <string_view>

namespace {
    constexpr char ModId[] = "mod.commands";
    constexpr char InvokeCommandId[] = "mod.commands.execute";

    StringView View(const char* value) { return {value, std::strlen(value)}; }

    struct State {
        const Api* api = nullptr;
        Registration invoke_command = 0;
        Registration status_command = 0;
    } state;

    Result CALL ExecuteRegisteredCommand(StringView arguments, void*) {
        if (state.api == nullptr || arguments.data == nullptr) return RESULT_INVALID_ARGUMENT;

        const std::string_view input(arguments.data, arguments.size);
        const size_t separator = input.find_first_of(" \t\r\n");
        const std::string_view commandId = input.substr(0, separator);
        if (commandId.empty() || commandId == InvokeCommandId) return RESULT_INVALID_ARGUMENT;

        const size_t argumentStart = separator == std::string_view::npos
            ? input.size()
            : input.find_first_not_of(" \t\r\n", separator);
        const std::string_view commandArguments = argumentStart == std::string_view::npos
            ? std::string_view{}
            : input.substr(argumentStart);

        return state.api->invoke_command(
            View(ModId),
            {commandId.data(), commandId.size()},
            {commandArguments.data(), commandArguments.size()});
    }

    Result CALL ReportStatus(StringView, void*) {
        return state.api->log(View(ModId), LOG_INFO,
            View("Shroudtopia Commands is active and routes registered mod commands."));
    }

    Result CALL Load(const Api* api, void*) {
        if (api == nullptr || api->api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
        state.api = api;

        return RESULT_OK;
    }

    Result CALL Activate(const Api* api, void*) {
        if (api == nullptr) return RESULT_INVALID_ARGUMENT;
        const CommandDescriptor execute{
            sizeof(CommandDescriptor), View(InvokeCommandId),
            View("Executes a command registered by any active mod."),
            ExecuteRegisteredCommand, nullptr
        };
        Result result = api->register_command(View(ModId), &execute, &state.invoke_command);
        if (result != RESULT_OK) return result;

        const CommandDescriptor status{
            sizeof(CommandDescriptor), View("mod.commands.status"),
            View("Reports whether the bundled Commands mod is active."),
            ReportStatus, nullptr
        };
        result = api->register_command(View(ModId), &status, &state.status_command);
        if (result != RESULT_OK) {
            api->release_registration(state.invoke_command);
            state.invoke_command = 0;
        }
        return result;
    }

    Result CALL Deactivate(const Api* api, void*) {
        if (api == nullptr) return RESULT_INVALID_ARGUMENT;
        if (state.invoke_command != 0) api->release_registration(state.invoke_command);
        if (state.status_command != 0) api->release_registration(state.status_command);
        state.invoke_command = 0;
        state.status_command = 0;
        return RESULT_OK;
    }

    Result CALL Unload(const Api* api, void*) {
        if (api == nullptr) return RESULT_INVALID_ARGUMENT;
        state = {};
        return RESULT_OK;
    }
}

extern "C" __declspec(dllexport) Result CALL CreateMod(
    uint32_t requestedApiVersion, ModDescriptor* descriptor) {
    if (requestedApiVersion != API_VERSION) return RESULT_VERSION_MISMATCH;
    if (descriptor == nullptr || descriptor->struct_size < sizeof(ModDescriptor)) return RESULT_INVALID_ARGUMENT;

    *descriptor = {
        sizeof(ModDescriptor), View(ModId), nullptr,
        Load, Activate, nullptr, Deactivate, Unload
    };
    return RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}

