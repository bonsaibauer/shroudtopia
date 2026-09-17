# Loader Development

## Scope

| Area | Core responsibility |
|---|---|
| Bootstrap | Start Shroudtopia in the client or dedicated-server process. |
| Discovery | Validate manifests and load matching x64 mod DLLs. |
| Lifecycle | Execute load, activate, update, deactivate, and unload transitions. |
| Platform API | Own registries, capabilities, permissions, logging, and cleanup. |
| Asset engine | Own reflection, KFC3 data, typed JSON, and persistence. |
| Game providers | Convert verified engine behavior into stable public services. |

```text
src/bootstrap → src/loader → public C API ← external mods
                       │
                       └→ src/engine
```

Public API changes require a version review. Public declarations, loader code, bundled mods, tests, and documentation change together.
