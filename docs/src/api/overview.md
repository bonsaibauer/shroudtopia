# API Structure

## Status legend

| Symbol | Meaning |
|:---:|---|
| ✅ | Available and implemented |
| 🧪 | Public contract or controlled test provider only |
| 🚧 | In active development |
| ❌ | Not available |

## Layers

| Layer | Path | Purpose |
|---|---|---|
| Public C API | `api/include/shroudtopia/api.h` | Stable DLL boundary and common entry point |
| Domain contracts | `api/include/shroudtopia/api/*.h` | Versioned types and service tables |
| Loader core | `src/loader/` | Implements `ST_HostApiV1` and built-in services |
| Mods | external or `mods/native/` | Consume the API and may publish services |

## Headers

| Header | Area |
|---|---|
| `base.h` | ABI version, strings, registrations, and results |
| `host.h` | Host function table |
| `lifecycle.h` | Mod descriptor and factory |
| `services.h` | Service publication and discovery |
| `events.h` | Synchronous event publication |
| `commands.h` | Command registry |
| `settings.h` | Settings contracts |
| `capabilities.h` | Capability result type |
| `logging.h` | Log levels and current-log reader |
| `ui.h` | Native text windows |
| `runtime.h` | Controlled direct patches and detours |
| `assets.h` | Typed game resources |
| `world.h` | Entity and grid contract |
| `world_grid.h` | Pure grid calculations |

## Access model

```text
ST_HostApiV1
├─ direct host functions
├─ register_service() for mod APIs
└─ find_service()
   ├─ shroudtopia.logging.read@1.0       ✅
   ├─ shroudtopia.ui.text@1.0            ✅
   ├─ shroudtopia.runtime.patches@1.0    ✅
   ├─ shroudtopia.assets@1.1             ✅
   └─ shroudtopia.world@1.1              🧪
```

Consumers check the result, returned pointer, `struct_size`, service version, and
optional `abi_version`. `ST_RESULT_NOT_FOUND` is a normal result for an optional
service.
