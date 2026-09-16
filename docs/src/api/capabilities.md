# Capabilities and Permissions

Capabilities answer whether the active Shroudtopia build can perform an operation.
Permissions answer whether a specific owner requested that operation.

## Capability matrix

| Status | Capability |
|:---:|---|
| ✅ | `shroudtopia.lifecycle.native` |
| ✅ | `shroudtopia.registry.services` |
| ✅ | `shroudtopia.registry.events` |
| ✅ | `shroudtopia.registry.commands` |
| ✅ | `shroudtopia.registry.settings` |
| ✅ | `shroudtopia.assets.read` when the asset engine is available |
| ✅ | `shroudtopia.assets.write` when the asset engine is available |
| ✅ | `shroudtopia.runtime.patches` |
| 🚧 | `shroudtopia.game.targeting` |
| 🚧 | `shroudtopia.world.entities.read` |
| 🚧 | `shroudtopia.world.entities.write` |
| 🚧 | `shroudtopia.world.voxels.read` |
| 🚧 | `shroudtopia.world.voxels.write` |
| 🚧 | `shroudtopia.ui.overlay` |

Logging and native text UI are implemented services and do not require a capability.

```cpp
ST_CapabilityInfoV1 info{sizeof(info)};
const ST_Result known = host->query_capability(
    View(ST_CAPABILITY_WORLD_VOXELS_READ), &info);
if (known == ST_RESULT_OK && info.available) {
    uint8_t allowed = 0;
    host->check_permission(owner, View(ST_CAPABILITY_WORLD_VOXELS_READ), &allowed);
}
```

Protected operations require the capability in `requires.capabilities` and still
return an operation-specific result. Unknown IDs return `ST_RESULT_NOT_FOUND`.
