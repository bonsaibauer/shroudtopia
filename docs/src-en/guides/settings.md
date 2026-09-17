# Settings

Register one JSON Schema plus defaults during `on_load`. Runtime values live below `mods.<owner>.<key>` in `shroudtopia.json`. Boolean and finite-number readers return the supplied fallback for a missing or mismatched value.

```cpp
ST_SettingsDescriptor settings{sizeof(settings), View("author.mod.settings"),
    View(R"({"type":"object","properties":{"speed":{"type":"number","minimum":0}}})"),
    View(R"({"speed":1.0})")};
```

The host does not yet validate schemas or generate a UI, and it has no string/object reader. Mods must enforce domain bounds and integer requirements.
