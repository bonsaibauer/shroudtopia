# Configuration and Logs

## Loader configuration

```json
{
  "active": true,
  "updateDelay": 500,
  "enableLogging": true,
  "mods": {
    "mod.commands": { "active": true },
    "mod.debug-console": { "active": true, "toggleKey": 121 },
    "mod.flight": { "active": false },
    "mod.no-stamina-loss": { "active": false },
    "mod.no-fall-damage": { "active": false },
    "mod.no-resource-cost": { "active": false },
    "mod.infinite-item-use": { "active": false },
    "mod.unlock-blueprints": { "active": false },
    "mod.infinite-item-split": { "active": false }
  }
}
```

| Key | Type | Purpose |
|---|---|---|
| `active` | Boolean | Enables or deactivates all mods. |
| `updateDelay` | Integer | Loader update interval in milliseconds. |
| `enableLogging` | Boolean | Enables the central Shroudtopia log writer. |
| `mods.<id>.active` | Boolean | Controls one mod. |
| `mods.mod.debug-console.toggleKey` | Integer | Windows virtual-key code. `121` is F10. |

The loader reloads configuration changes while running. Manifest defaults populate
missing mod settings.

## Log sessions

| Path | Content |
|---|---|
| `shroudtopia.log` | Current loader session and all owner-tagged mod messages. |
| `enshrouded.log` | Current game log. |
| `shroudtopia_logs/` | Completed Shroudtopia sessions. |

At process start, a non-empty `shroudtopia.log` moves to
`shroudtopia_logs/shroudtopia-YYYYMMDD-HHMMSS-client.log` or `-server.log`. The
timestamp is UTC. Every generated level is written. The Debug Console filters its
view only.

The native Debug Console opens with F10, remains unaffected by Escape, and provides
search, level selection, Copy Logs, pause, auto-scroll, and separate Game and
Shroudtopia tabs. Each tab reads up to the latest 1 MiB through the public API.
