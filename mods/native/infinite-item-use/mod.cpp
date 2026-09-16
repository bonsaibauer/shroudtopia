#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shroudtopia/api.h"

#include <cstdio>
#include <cstring>

namespace {
constexpr char ModId[] = "mod.infinite-item-use";
constexpr char Label[] = "Infinite Item Use";
// The current game passes the consume flag as the seventh x64 call argument.
// Replace `movzx eax, byte ptr [rbp+398h]` with `mov eax, 0`; the unchanged
// following instruction then writes AL to [rsp+30h]. MOV preserves flags.
constexpr uint8_t Payload[] = {0xB8,0,0,0,0,0x90,0x90};
constexpr ST_RuntimePatchKindV1 Kind = ST_RUNTIME_PATCH_DIRECT;

const ST_RuntimePatchesApiV1* Runtime = nullptr;
ST_RuntimePatch Patch = 0;

ST_StringView View(const char* value) { return {value, std::strlen(value)}; }

ST_Result ST_CALL Load(const ST_HostApiV1* host, void*) {
    if (host == nullptr || host->abi_version != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    const ST_ServiceRequest request{
        sizeof(ST_ServiceRequest), View(ST_RUNTIME_PATCHES_SERVICE_ID),
        ST_RUNTIME_PATCHES_VERSION_MAJOR, ST_RUNTIME_PATCHES_VERSION_MINOR
    };
    const void* service = nullptr;
    const auto found = host->find_service(&request, &service);
    if (found != ST_RESULT_OK) return found;
    Runtime = static_cast<const ST_RuntimePatchesApiV1*>(service);

    const ST_RuntimePatchDescriptorV1 descriptor{
        sizeof(ST_RuntimePatchDescriptorV1),
        View("0F B6 85 ?? ?? 00 00 48 8D 4C 24 40 88 44 24 30 44 8B CE 8B 85 ?? ?? 00 00 4C 8B C3"), 0,
        Kind, 7, Payload, sizeof(Payload),
        nullptr, 0
    };
    const auto created = Runtime->create(View(ModId), &descriptor, &Patch);
    if (created != ST_RESULT_NOT_FOUND) return created;
    char message[192]{};
    _snprintf_s(message, _TRUNCATE, "%s unavailable: signature not found for this game build.", Label);
    host->log(View(ModId), ST_LOG_WARNING, View(message));
    return ST_RESULT_OK;
}

ST_Result ST_CALL Activate(const ST_HostApiV1*, void*) {
    return Patch == 0 ? ST_RESULT_NOT_FOUND : Runtime->set_enabled(View(ModId), Patch, 1);
}

ST_Result ST_CALL Update(const ST_HostApiV1*, void*, double) { return ST_RESULT_OK; }

ST_Result ST_CALL Deactivate(const ST_HostApiV1*, void*) {
    return Patch == 0 ? ST_RESULT_OK : Runtime->set_enabled(View(ModId), Patch, 0);
}

ST_Result ST_CALL Unload(const ST_HostApiV1* host, void*) {
    if (host == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    if (Patch != 0) {
        const auto released = Runtime->release(View(ModId), Patch);
        if (released != ST_RESULT_OK) return released;
    }
    Patch = 0;
    Runtime = nullptr;
    return host->release_owner(View(ModId));
}
}

extern "C" __declspec(dllexport) ST_Result ST_CALL ShroudtopiaCreateModV1(
    uint32_t abi, ST_ModDescriptorV1* descriptor) {
    if (abi != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    if (descriptor == nullptr || descriptor->struct_size < sizeof(ST_ModDescriptorV1)) return ST_RESULT_INVALID_ARGUMENT;
    *descriptor = {sizeof(ST_ModDescriptorV1), View(ModId), nullptr,
        Load, Activate, Update, Deactivate, Unload};
    return ST_RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}

