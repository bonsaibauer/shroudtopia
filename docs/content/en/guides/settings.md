# Configuration

`shroudtopia.json` contains runtime and mod settings:

```json
{
  "active": true,
  "updateDelay": 500,
  "enableLogging": true,
  "logLevel": "INFO",
  "mods": {
    "mod.example": {
      "enabled": true,
      "multiplier": 1.5
    }
  }
}
```

| Setting | Values | Effect |
|---|---|---|
| `active` | `true`, `false` | Activates or deactivates configured mods. |
| `updateDelay` | Milliseconds | Sets the runtime update interval. |
| `enableLogging` | `true`, `false` | Enables or disables Shroudtopia log output. |
| `logLevel` | `ALL`, `TRACE`, `DEBUG`, `INFO`, `WARNING`, `ERROR` | `ALL` records every level. Every other value records only that level. |

The Debug Console uses the same setting. `INFO` exposes `All` and `Info` in its level selector. `ALL` exposes all level choices.

Mods read their own values with `get_mod_setting_bool` and `get_mod_setting_number`. Actions use `register_action`, `invoke_action`, and `get_action_state`.
