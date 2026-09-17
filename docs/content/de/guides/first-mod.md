# Erste Mod in 10 Minuten

## 1. Projekt

Eine Windows-x64-DLL erstellen, `api` zum Include-Pfad hinzufügen, `shroudtopia.h` einbinden und `CreateMod` exportieren.

```cpp
#include <shroudtopia.h>
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

## 2. Paket

Die x64-DLL und `mod.json` in dasselbe Verzeichnis legen. Die Manifest-ID muss der von der Mod verwendeten Owner-ID entsprechen.

## 3. Prüfen

Spiel starten, die Owner-markierte Meldung in `shroudtopia.log` suchen und Loader-Fehler beheben, bevor Services ergänzt werden. Danach mit [Ownership](../concepts/ownership.md) und der [API](../reference/api.md) fortfahren.
