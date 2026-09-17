#include <windows.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "runtime_patches.h"

#pragma section(".sttest", read, execute)
__declspec(allocate(".sttest")) volatile std::uint8_t DirectTarget[] = {
    0x62,0x19,0xD4,0xA7,0x35,0xCE,0x81,0xFA
};
__declspec(allocate(".sttest")) volatile std::uint8_t DetourTarget[] = {
    0x71,0x28,0xE5,0xB6,0x43,0xDC,0x92,0xAF
};

__declspec(allocate(".sttest")) volatile std::uint8_t AmbiguousTargets[] = {
    0x39,0xA2,0x18,0xF3,0xCD,0x42,0x97,0xB5,
    0x39,0xA2,0x18,0xF3,0xCD,0x42,0x97,0xB5
};

namespace {
StringView View(const char* value) { return {value, std::strlen(value)}; }

int Fail(const char* message) {
    std::cerr << message << '\n';
    RuntimePatches::Shutdown();
    return 1;
}
}

int main() {
    const std::string owner = "mod.runtime-smoke";

    const std::uint8_t directPayload[]{0x90,0x90,0x90};
    RuntimePatchOptions direct{
        sizeof(direct), View("62 19 D4 A7 35 CE 81 FA"), 2,
        RUNTIME_PATCH_DIRECT, sizeof(directPayload),
        directPayload, sizeof(directPayload), nullptr, 0
    };
    RuntimePatch directHandle = 0;
    if (RuntimePatches::Create(owner, &direct, &directHandle) != RESULT_OK || directHandle == 0) return Fail("direct patch creation failed");
    RuntimePatch duplicate = 0;
    if (RuntimePatches::Create("mod.other", &direct, &duplicate) != RESULT_CONFLICT || duplicate != 0)
        return Fail("overlapping patch was accepted");
    auto ambiguous = direct;
    ambiguous.signature = View("39 A2 18 F3 CD 42 97 B5");
    if (RuntimePatches::Create(owner, &ambiguous, &duplicate) != RESULT_NOT_FOUND || duplicate != 0)
        return Fail("ambiguous signature was accepted");
    auto missing = direct;
    missing.signature = View("FF FF FF FF 12 34 56 78 90 AB CD EF 11 22 33 44");
    if (RuntimePatches::Create(owner, &missing, &duplicate) != RESULT_NOT_FOUND)
        return Fail("missing signature was accepted");
    if (RuntimePatches::SetEnabled(owner, directHandle, true) != RESULT_OK ||
        DirectTarget[2] != 0x90 || DirectTarget[3] != 0x90 || DirectTarget[4] != 0x90) return Fail("direct patch activation failed");
    RuntimePatchState state{sizeof(state)};
    if (RuntimePatches::GetState(owner, directHandle, &state) != RESULT_OK || state.enabled != 1) return Fail("direct patch state failed");
    if (RuntimePatches::Release(owner, directHandle) != RESULT_OK ||
        DirectTarget[2] != 0xD4 || DirectTarget[3] != 0xA7 || DirectTarget[4] != 0x35) return Fail("direct patch restoration failed");

    const std::uint8_t detourPayload[]{0x90,0x90,0xE9,0,0,0,0};
    const RuntimeRelocation relocation{
        sizeof(relocation), 3, RUNTIME_RELOCATION_REL32_RETURN
    };
    RuntimePatchOptions detour{
        sizeof(detour), View("71 28 E5 B6 43 DC 92 AF"), 0,
        RUNTIME_PATCH_DETOUR, 8,
        detourPayload, sizeof(detourPayload), &relocation, 1
    };
    RuntimePatch detourHandle = 0;
    if (RuntimePatches::Create(owner, &detour, &detourHandle) != RESULT_OK || detourHandle == 0) return Fail("detour creation failed");
    if (RuntimePatches::SetEnabled(owner, detourHandle, true) != RESULT_OK || DetourTarget[0] != 0xE9) return Fail("detour activation failed");

    std::int32_t shellDelta = 0;
    std::memcpy(&shellDelta, const_cast<const std::uint8_t*>(DetourTarget) + 1, sizeof(shellDelta));
    const auto target = reinterpret_cast<std::uintptr_t>(const_cast<std::uint8_t*>(DetourTarget));
    const auto shell = target + 5 + shellDelta;
    const auto shellBytes = reinterpret_cast<const std::uint8_t*>(shell);
    if (shellBytes[0] != 0x90 || shellBytes[1] != 0x90 || shellBytes[2] != 0xE9) return Fail("detour payload mismatch");
    std::int32_t returnDelta = 0;
    std::memcpy(&returnDelta, shellBytes + 3, sizeof(returnDelta));
    if (shell + 7 + returnDelta != target + 8) return Fail("detour return relocation failed");
    if (RuntimePatches::Release(owner, detourHandle) != RESULT_OK ||
        DetourTarget[0] != 0x71 || DetourTarget[7] != 0xAF) return Fail("detour restoration failed");

    RuntimePatches::Shutdown();
    std::cout << "Runtime patch direct/detour smoke test passed.\n";
    return 0;
}
