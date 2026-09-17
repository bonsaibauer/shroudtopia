# Erste Mod in 10 Minuten

## 1. Projekt

Eine Windows-x64-DLL erstellen, `api/include` zum Include-Pfad hinzufügen, `shroudtopia/api.h` einbinden und `ShroudtopiaCreateModV1` exportieren.

```cpp
#include <shroudtopia/api.h>
#include <cstring>
static ST_StringView View(const char* s) { return {s, std::strlen(s)}; }
static ST_Result ST_CALL Load(const ST_HostApiV1* host, void*) {
    return host->log(View("example.hello"), ST_LOG_INFO, View("Hallo von Shroudtopia"));
}
extern "C" __declspec(dllexport) ST_Result ST_CALL
ShroudtopiaCreateModV1(uint32_t requested, ST_ModDescriptorV1* out) {
    if (!out || requested != ST_ABI_VERSION_1 || out->struct_size < sizeof(*out))
        return ST_RESULT_VERSION_MISMATCH;
    *out = {sizeof(*out), View("example.hello"), nullptr, Load, nullptr, nullptr, nullptr, nullptr};
    return ST_RESULT_OK;
}
```

## 2. Paket

Die x64-DLL und `mod.json` in dasselbe Verzeichnis legen. Die Manifest-ID muss der von der Mod verwendeten Owner-ID entsprechen.

## 3. Prüfen

Spiel starten, die Owner-markierte Meldung in `shroudtopia.log` suchen und Loader-Fehler beheben, bevor Services ergänzt werden. Danach mit [Ownership](../concepts/ownership.md) und der [Host API](../reference/generated/host-api-v1.md) fortfahren.
