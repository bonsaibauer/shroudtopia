#include "pch.h"
#include "runtime.h"

extern "C" __declspec(dllexport) BOOL __cdecl ShroudtopiaStop(DWORD timeout_milliseconds) {
    return Runtime::Stop(timeout_milliseconds);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        return Runtime::Start();
    }
    if (reason == DLL_PROCESS_DETACH && reserved != nullptr) {
        // Process termination: Windows reclaims thread/event handles.
    }
    return TRUE;
}
