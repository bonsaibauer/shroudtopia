# Bundled Shroudtopia Mods

| Status | Mod ID | Display name | Target | Default |
|:---:|---|---|---|:---:|
| ✅ | `mod.commands` | Shroudtopia Commands | `both` | On |
| ✅ | `mod.debug-console` | Shroudtopia Debug Console | `client` | On |
| ✅ | `mod.flight` | Shroudtopia Flight | `client` | Off |
| ✅ | `mod.no-stamina-loss` | Shroudtopia No Stamina Loss | `both` | Off |
| ✅ | `mod.no-fall-damage` | Shroudtopia No Fall Damage | `both` | Off |
| ✅ | `mod.no-resource-cost` | Shroudtopia No Resource Cost | `both` | Off |
| ✅ | `mod.infinite-item-use` | Shroudtopia Infinite Item Use | `both` | Off |
| ✅ | `mod.unlock-blueprints` | Shroudtopia Unlock Blueprints | `both` | Off |
| ✅ | `mod.infinite-item-split` | Shroudtopia Infinite Item Split | `both` | Off |

Each feature is an independent mod controlled by `mods.<mod-id>.active`. The Debug
Console uses the public Logging and Text UI services. Gameplay patch mods require a
validated signature for the active game profile. Unlock Blueprints uses the Asset API.

`target` controls the process in which a mod may load. Multiplayer authority still
depends on the operation and the authoritative server.
