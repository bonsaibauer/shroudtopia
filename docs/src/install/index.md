# Quick Start

## Runtime files

| File | Purpose |
|---|---|
| `winmm.dll` | Starts Shroudtopia with the process. |
| `shroudtopia.dll` | Provides runtime, lifecycle, registries, and public API. |
| `shroudtopia-assets.dll` | Provides internal KFC3 reflection and asset processing. |

Copy all three files next to `enshrouded.exe` or `enshrouded_server.exe`, then start
the process.

```text
Enshrouded/
├── winmm.dll
├── shroudtopia.dll
├── shroudtopia-assets.dll
├── shroudtopia.json
├── shroudtopia.log
├── shroudtopia_logs/
└── mods/
```

Shroudtopia creates the configuration and log paths when needed. Continue with
[Install Mods](./mods.md).
