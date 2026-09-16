# Lifecycle

## Status

| Status | Feature |
|:---:|---|
| ✅ | Native v1 descriptor |
| ✅ | Load, activate, update, deactivate, unload |
| ❌ | Game-thread scheduler |

```text
Discover → Factory → Load → Activate → Update*
                              │          │
                              └→ Deactivate → Unload
```

| Callback | Responsibility |
|---|---|
| `on_load` | Register services, events, commands, settings, and non-active resources. |
| `on_activate` | Enable behavior allowed by configuration. |
| `on_update` | Perform bounded mod work on the Shroudtopia worker thread. |
| `on_deactivate` | Stop active behavior and restore reversible changes. |
| `on_unload` | Release external resources and owner registrations. |

Callbacks are optional. Missing callbacks succeed. Exceptions must not cross the C
ABI. `on_update` is not the game thread and must not call unverified engine functions.
