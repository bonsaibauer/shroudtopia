# Shroudtopia

Shroudtopia is a native Enshrouded mod loader with a versioned C API. Mods use the
public API exclusively, and the loader does not depend on any mod.

## Status legend

| Symbol | Meaning |
|:---:|---|
| ✅ | Available and implemented |
| 🧪 | Public contract or controlled test provider only |
| 🚧 | In active development |
| ❌ | Not available |

## Install

1. Download the current release archive.
2. Copy `winmm.dll`, `shroudtopia.dll`, and `shroudtopia-assets.dll` next to the
   game or dedicated-server executable.
3. Place each mod in its own `mods/<mod-id>/` directory.
4. Start the game or server. Shroudtopia creates `shroudtopia.json` and the current
   `shroudtopia.log`.

```text
Enshrouded/
├── winmm.dll
├── shroudtopia.dll
├── shroudtopia-assets.dll
├── shroudtopia.json
├── shroudtopia.log
├── shroudtopia_logs/
└── mods/
    └── mod.author.mod-name/
        ├── mod.json
        └── mod-name.dll
```

## Bundled mods

| Status | Mod | Default | Purpose |
|:---:|---|:---:|---|
| ✅ | Shroudtopia Commands | On | Routes calls through the shared command registry. |
| ✅ | Shroudtopia Debug Console | On | Displays the current Enshrouded and Shroudtopia logs. |
| ✅ | Shroudtopia Flight | Off | Applies the validated glider-flight patch. |
| ✅ | Shroudtopia No Stamina Loss | Off | Prevents the supported stamina deduction. |
| ✅ | Shroudtopia No Fall Damage | Off | Prevents the supported fall-damage operation. |
| ✅ | Shroudtopia No Resource Cost | Off | Sets the supported crafting and building resource cost to zero. |
| ✅ | Shroudtopia Infinite Item Use | Off | Prevents the supported item-consumption operation. |
| ✅ | Shroudtopia Unlock Blueprints | Off | Unlocks recipes through the Asset API. |
| ✅ | Shroudtopia Infinite Item Split | Off | Prevents the supported deduction when splitting stacks. |

Runtime patches activate only when their signature matches the running game build.

## Platform capabilities

| Status | Area |
|:---:|---|
| ✅ | Lifecycle, discovery, services, events, commands, and settings |
| ✅ | Owner-tagged logging, current-log reading, and native text UI |
| ✅ | Capabilities, permissions, ownership, and cleanup |
| ✅ | Runtime patches and detours |
| ✅ | Typed KFC3 Asset API |
| 🧪 | World API contract and ShroudEdit test provider |
| 🚧 | Targeting, live World provider, game-thread jobs, and world overlay |

## Develop Shroudtopia

```powershell
git clone --recurse-submodules <repository-url>
cd shroudtopia
.\build.ps1
```

The build produces the loader, asset engine, bundled mods, automated checks, and one
installable archive under `build/`.

## Create mods

Mods include `api/include`, export `ShroudtopiaCreateModV1`, and ship with a
`mod.json`. The public C API is the only binary contract between mods and the loader.

- API entry point: [api/include/shroudtopia/api.h](api/include/shroudtopia/api.h)
- Documentation: [Shroudtopia API](https://bonsaibauer.github.io/shroudtopia/)
- Local book: [docs/src/SUMMARY.md](docs/src/SUMMARY.md)
- Complete reference mod: [Shroudtopia Flight](mods/native/flight)

## Versioning

`VERSION` is the single product-version source. Branches use `MAJOR.MINOR.PATCH`,
release tags use `vMAJOR.MINOR.PATCH`, and CI assigns the independent build number.

The loader and public API use the [MIT license](LICENSE). The internal KFC3 engine
and parser submodule use GPL-3.0-or-later. See [src/engine/NOTICE.md](src/engine/NOTICE.md).
