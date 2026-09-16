# Build a Flight Mod

Shroudtopia Flight is the complete native reference mod.

```text
mods/native/flight/
├── mod.cpp
├── mod.json
└── shroudtopia-flight.vcxproj
```

## API usage

| Step | Operation |
|---:|---|
| 1 | Declare `shroudtopia.runtime.patches` in the manifest. |
| 2 | Find `shroudtopia.runtime.patches@1.0`. |
| 3 | Create an owner-bound detour during load. |
| 4 | Enable it during activation. |
| 5 | Restore original bytes during deactivation. |
| 6 | Release the handle and owner during unload. |

```json
{{#include ../../../mods/native/flight/mod.json}}
```

```cpp
{{#include ../../../mods/native/flight/mod.cpp}}
```

The installed files are:

```text
mods/mod.flight/
├── mod.json
└── shroudtopia-flight.dll
```

Set `mods.mod.flight.active` to `true`. A missing or ambiguous signature returns an
error and leaves process memory unchanged.
