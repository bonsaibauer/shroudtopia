# API Quick Start

## Requirements

| Item | Value |
|---|---|
| Platform | Windows x64 |
| Language boundary | C ABI |
| Public include | `api/include/shroudtopia/api.h` |
| Required export | `ShroudtopiaCreateModV1` |
| Package metadata | `mod.json` |

```text
my-mod/
├── CMakeLists.txt
├── mod.json
└── src/
    └── mod.cpp
```

## Workflow

1. Add `api/include` to the compiler include path.
2. Export the v1 factory and provide lifecycle callbacks.
3. Package the x64 DLL and `mod.json` in one mod directory.
4. Use `ST_HostApiV1` and services returned by `find_service` only.
5. Query optional capabilities and handle `NOT_FOUND` and `UNSUPPORTED`.
6. Log through `host->log` and release owner resources during unload.

## Current API status

| Status | Area |
|:---:|---|
| ✅ | Lifecycle, logging, services, events, commands, and settings |
| ✅ | Capabilities, permissions, ownership, and cleanup |
| ✅ | Current-log reading and native text UI |
| ✅ | Asset API and runtime patch service |
| 🧪 | World contract and controlled test provider |
| 🚧 | Live Game, World, and renderer providers |

Continue with [Create a Project](./guide/project-setup.md) or use the
[Function Reference](./api/function-reference.md).
