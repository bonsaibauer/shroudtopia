#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shroudtopia.h"

#include <cstdio>
#include <cstring>

namespace {
constexpr char ModId[] = "mod.no-stamina-loss";
constexpr char Label[] = "No Stamina Loss";
constexpr uint8_t Payload[] = {0x53,0x8B,0x5C,0x91,0x08,0x89,0x1C,0x91,0x5B,0x8B,0x04,0x91,0x89,0x44,0x24,0x40,0xE9,0,0,0,0};
constexpr RuntimePatchKind Kind = RUNTIME_PATCH_DETOUR;

const Api* Runtime = nullptr;
RuntimePatch Patch = 0;

StringView View(const char* value) { return {value, std::strlen(value)}; }

Result CALL Load(const Api* api, void*) {
    if (api == nullptr || api->api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    Runtime = api;

    const RuntimeRelocation returnJump{
        sizeof(RuntimeRelocation), 17, RUNTIME_RELOCATION_REL32_RETURN
    };
    const RuntimePatchOptions descriptor{
        sizeof(RuntimePatchOptions), View("8B 04 91 89 44 24 40"), 0,
        Kind, 7, Payload, sizeof(Payload),
        Kind == RUNTIME_PATCH_DETOUR ? &returnJump : nullptr,
        Kind == RUNTIME_PATCH_DETOUR ? 1u : 0u
    };
    const auto created = Runtime->create_patch(View(ModId), &descriptor, &Patch);
    if (created != RESULT_NOT_FOUND) return created;
    char message[192]{};
    _snprintf_s(message, _TRUNCATE, "%s unavailable: signature not found for this game build.", Label);
    api->log(View(ModId), LOG_WARNING, View(message));
    return RESULT_OK;
}

Result CALL Activate(const Api*, void*) {
    return Patch == 0 ? RESULT_NOT_FOUND : Runtime->set_patch_enabled(View(ModId), Patch, 1);
}

Result CALL Deactivate(const Api*, void*) {
    return Patch == 0 ? RESULT_OK : Runtime->set_patch_enabled(View(ModId), Patch, 0);
}

Result CALL Unload(const Api* api, void*) {
    if (api == nullptr) return RESULT_INVALID_ARGUMENT;
    if (Patch != 0) {
        const auto released = Runtime->release_patch(View(ModId), Patch);
        if (released != RESULT_OK) return released;
    }
    Patch = 0;
    Runtime = nullptr;
    return RESULT_OK;
}
}

extern "C" __declspec(dllexport) Result CALL CreateMod(
    uint32_t api_version, ModDescriptor* descriptor) {
    if (api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    if (descriptor == nullptr || descriptor->struct_size < sizeof(ModDescriptor)) return RESULT_INVALID_ARGUMENT;
    *descriptor = {sizeof(ModDescriptor), View(ModId), nullptr,
        Load, Activate, nullptr, Deactivate, Unload};
    return RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}

