# 2. Write a First Mod

This mod registers a command and writes through the central Logging API.

```cpp
#include <shroudtopia/api.h>
#include <windows.h>
#include <cstring>

namespace {
constexpr char ModId[] = "mod.author.hello";
const ST_HostApiV1* Host = nullptr;
ST_StringView View(const char* value) { return {value, std::strlen(value)}; }

ST_Result ST_CALL Hello(ST_StringView arguments, void*) {
    return Host->log(View(ModId), ST_LOG_INFO, arguments);
}
ST_Result ST_CALL Load(const ST_HostApiV1* api, void*) {
    if (!api || api->abi_version != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    Host = api;
    ST_CommandDescriptor command{sizeof(command), View("mod.author.hello.say"),
        View("Write text to the log"), Hello, nullptr};
    ST_Registration registration = 0;
    return api->register_command(View(ModId), &command, &registration);
}
ST_Result ST_CALL NoOp(const ST_HostApiV1*, void*) { return ST_RESULT_OK; }
ST_Result ST_CALL Update(const ST_HostApiV1*, void*, double) { return ST_RESULT_OK; }
ST_Result ST_CALL Unload(const ST_HostApiV1* api, void*) {
    const auto result = api->release_owner(View(ModId));
    Host = nullptr;
    return result;
}
}

extern "C" __declspec(dllexport) ST_Result ST_CALL
ShroudtopiaCreateModV1(uint32_t abi, ST_ModDescriptorV1* out) {
    if (abi != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    if (!out || out->struct_size < sizeof(*out)) return ST_RESULT_INVALID_ARGUMENT;
    *out = {sizeof(*out), View(ModId), nullptr, Load, NoOp, Update, NoOp, Unload};
    return ST_RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}
```

## Rules

| Status | Rule |
|:---:|---|
| ✅ | Keep the descriptor ID equal to the manifest ID |
| ✅ | Catch exceptions before they cross the C ABI |
| ✅ | Use the public API only |
| ✅ | Release owner resources during unload |
| ❌ | Call game internals from `on_update` |
