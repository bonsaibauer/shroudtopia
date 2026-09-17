# Settings

Während `on_load` ein JSON Schema plus Standardwerte registrieren. Laufzeitwerte liegen unter `mods.<owner>.<key>` in `shroudtopia.json`. Reader für Boolean und endliche Zahlen liefern bei fehlendem oder typfalschem Wert den angegebenen Fallback.

```cpp
ST_SettingsDescriptor settings{sizeof(settings), View("author.mod.settings"),
    View(R"({"type":"object","properties":{"speed":{"type":"number","minimum":0}}})"),
    View(R"({"speed":1.0})")};
```

Der Host validiert Schemas noch nicht, erzeugt keine UI und besitzt keinen String-/Object-Reader. Mods müssen Domain-Grenzen und Integer-Anforderungen selbst durchsetzen.
