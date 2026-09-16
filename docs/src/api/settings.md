# Settings

## Status

| Status | Feature |
|:---:|---|
| ✅ | Register schema and defaults |
| ✅ | Read Boolean values |
| ✅ | Read finite numeric values |
| ❌ | String/object readers |
| ❌ | Schema validation and generated UI |

```cpp
ST_SettingsDescriptor settings{
    sizeof(settings),
    View("author.mod.settings"),
    View(R"({"type":"object"})"),
    View("{}")
};
```

Runtime values are stored under `mods.<owner>.<key>` in `shroudtopia.json`.
`get_setting_bool` and `get_setting_number` return their supplied fallback for a
missing or mismatched value. Numeric results must be finite. Mods enforce their own
domain limits and integer requirements. Manifest defaults are the persistent source
for missing mod settings.
