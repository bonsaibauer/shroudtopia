# Vollständige Minimal-Mod

Der [Erste-Mod-Guide](../guides/first-mod.md) enthält die vollständige exportierte Factory. Ein Production-Projekt sollte Zustand und Callbacks trennen, Registrierungs-Handles behalten und Cleanup implementieren:

```cpp
struct State { StringView owner; Registration command{}; };

static Result CALL Unload(const Api* host, void* user) {
    auto* state = static_cast<State*>(user);
    if (state->command) host->release_registration(state->command);
    host->release_owner(state->owner);
    return RESULT_OK;
}
```

Als Windows-x64-DLL bauen, Exceptions innerhalb der Callbacks halten, mit `mod.json` paketieren und Laden in `shroudtopia.log` prüfen. Nur tatsächlich benötigte Capabilities ergänzen.
