# First mod in 10 minutes

## 1. Project

Create a Windows x64 DLL, add `api/include` to the include path, include `shroudtopia/api.h`, and export `CreateMod`.

```cpp
#include <shroudtopia/api.h>
#include <cstring>
static StringView View(const char* s) { return {s, std::strlen(s)}; }
static Result CALL Load(const Api* host, void*) {
    return host->log(View("example.hello"), LOG_INFO, View("Hello from Shroudtopia"));
}
extern "C" __declspec(dllexport) Result CALL
CreateMod(uint32_t requested, ModDescriptor* out) {
    if (!out || requested != API_VERSION || out->struct_size < sizeof(*out))
        return RESULT_VERSION_MISMATCH;
    *out = {sizeof(*out), View("example.hello"), nullptr, Load, nullptr, nullptr, nullptr, nullptr};
    return RESULT_OK;
}
```

## 2. Package

Put the x64 DLL and `mod.json` in one directory. The manifest ID must match the owner ID used by the mod.

## 3. Verify

Start the game, find the owner-tagged message in `shroudtopia.log`, and fix any loader error before adding services. Continue with [ownership](../concepts/ownership.md) and the [Host API](../reference/api.md).
