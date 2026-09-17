#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shroudtopia/api.h"

#include <cstring>

namespace {
constexpr char ModId[] = "mod.flight";
constexpr uint8_t Payload[] = {
    0xF3,0x0F,0x10,0x05,0x05,0,0,0,0xE9,0,0,0,0,0xC3,0xF5,0xC8,0xBF
};

const PatchesApi* Runtime = nullptr;
RuntimePatch Patch = 0;
Registration ToggleAction = 0;
bool IsEnabled = false;

StringView View(const char* value) { return {value, std::strlen(value)}; }

Result CALL ToggleFlight(StringView, void*) {
    if (Runtime == nullptr || Patch == 0) return RESULT_NOT_FOUND;
    const bool enable = !IsEnabled;
    const auto result = Runtime->set_enabled(View(ModId), Patch, enable ? 1 : 0);
    if (result == RESULT_OK) IsEnabled = enable;
    return result;
}

Result CALL Load(const Api* api, void*) {
    if (api == nullptr || api->api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    Runtime = api->patches;
    if (Runtime == nullptr) return RESULT_NOT_FOUND;

    const RuntimeRelocation returnJump{
        sizeof(RuntimeRelocation), 9, RUNTIME_RELOCATION_REL32_RETURN
    };
    const RuntimePatchOptions descriptor{
        sizeof(RuntimePatchOptions),
        View("F3 0F 10 05 ?? ?? ?? ?? F2 0F 11 4C"),
        0, RUNTIME_PATCH_DETOUR, 8,
        Payload, sizeof(Payload), &returnJump, 1
    };
    const auto created = Runtime->create(View(ModId), &descriptor, &Patch);
    if (created == RESULT_NOT_FOUND) {
        api->log(View(ModId), LOG_WARNING,
            View("Flight unavailable: signature not found for this game build."));
        return RESULT_OK;
    }
    if (created != RESULT_OK) return created;
    const Action action{
        sizeof(action), View("flight.toggle"), View("Toggle flight"),
        View("Enables or disables flight in the active game session."),
        View("{\"type\":\"null\"}"), ToggleFlight, nullptr
    };
    return api->register_action(View(ModId), &action, &ToggleAction);
}

Result CALL Activate(const Api*, void*) {
    if (Patch == 0) return RESULT_NOT_FOUND;
    const auto result = Runtime->set_enabled(View(ModId), Patch, 1);
    if (result == RESULT_OK) IsEnabled = true;
    return result;
}

Result CALL Update(const Api*, void*, double) { return RESULT_OK; }

Result CALL Deactivate(const Api*, void*) {
    if (Patch == 0) return RESULT_OK;
    const auto result = Runtime->set_enabled(View(ModId), Patch, 0);
    if (result == RESULT_OK) IsEnabled = false;
    return result;
}

Result CALL Unload(const Api* api, void*) {
    if (api == nullptr) return RESULT_INVALID_ARGUMENT;
    if (Patch != 0) {
        const auto released = Runtime->release(View(ModId), Patch);
        if (released != RESULT_OK) return released;
    }
    Patch = 0;
    ToggleAction = 0;
    IsEnabled = false;
    Runtime = nullptr;
    return api->release_owner(View(ModId));
}
}

extern "C" __declspec(dllexport) Result CALL CreateMod(
    uint32_t api_version, ModDescriptor* descriptor) {
    if (api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    if (descriptor == nullptr || descriptor->struct_size < sizeof(ModDescriptor)) return RESULT_INVALID_ARGUMENT;
    *descriptor = {sizeof(ModDescriptor), View(ModId), nullptr,
        Load, Activate, Update, Deactivate, Unload};
    return RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}
