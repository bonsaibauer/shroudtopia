# First mod in 10 minutes

## 1. Project

Create a Windows x64 DLL, add `api/include` to the include path, include `shroudtopia/api.h`, and export `ShroudtopiaCreateModV1`.

```cpp
#include <shroudtopia/api.h>
#include <cstring>
static ST_StringView View(const char* s) { return {s, std::strlen(s)}; }
static ST_Result ST_CALL Load(const ST_HostApiV1* host, void*) {
    return host->log(View("example.hello"), ST_LOG_INFO, View("Hello from Shroudtopia"));
}
extern "C" __declspec(dllexport) ST_Result ST_CALL
ShroudtopiaCreateModV1(uint32_t requested, ST_ModDescriptorV1* out) {
    if (!out || requested != ST_ABI_VERSION_1 || out->struct_size < sizeof(*out))
        return ST_RESULT_VERSION_MISMATCH;
    *out = {sizeof(*out), View("example.hello"), nullptr, Load, nullptr, nullptr, nullptr, nullptr};
    return ST_RESULT_OK;
}
```

## 2. Package

Put the x64 DLL and `mod.json` in one directory. The manifest ID must match the owner ID used by the mod.

## 3. Verify

Start the game, find the owner-tagged message in `shroudtopia.log`, and fix any loader error before adding services. Continue with [ownership](../concepts/ownership.md) and the [Host API](../reference/generated/host-api-v1.md).
