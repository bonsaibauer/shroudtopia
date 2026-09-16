# Build an Asset Mod

An Asset mod describes a typed resource change. It does not open KFC files or locate
process memory.

## Manifest

```json
{
  "id": "mod.author.recipe-change",
  "version": "1.0.0",
  "requires": {
    "capabilities": ["shroudtopia.assets.read", "shroudtopia.assets.write"]
  },
  "shroudtopia": {
    "abi": "1.0",
    "entrypoint": "ShroudtopiaCreateModV1",
    "binary": "recipe-change.dll",
    "target": "both",
    "default": { "active": false }
  }
}
```

## Find the service

```cpp
ST_ServiceRequest request{sizeof(request), View(ST_ASSETS_SERVICE_ID),
    ST_ASSETS_SERVICE_VERSION_MAJOR, ST_ASSETS_SERVICE_VERSION_MINOR};
const void* service = nullptr;
if (api->find_service(&request, &service) != ST_RESULT_OK) return ST_RESULT_NOT_FOUND;
const auto* assets = static_cast<const ST_AssetsApiV1*>(service);
```

## Operations

| Function | Use |
|---|---|
| `visit_resources` | Enumerate resources of one qualified type. |
| `read_resource_json` | Read typed JSON with a size query followed by a data read. |
| `set_resource_field_json` | Change one existing JSON-pointer field. |
| `replace_resource_json` | Replace one complete resource document. |
| `create_resource_json` | Create a typed resource. |
| `flush` | Publish the combined owner overlays. |
| `discard_changes` | Remove changes owned by the calling mod. |

```cpp
const auto result = assets->set_resource_field_json(
    View(ModId), resource, View("/maxStackSize"), View("65535"));
```

`ST_RESULT_ALREADY_EXISTS` reports an ownership conflict on the same or overlapping
path. The bundled `mod.unlock-blueprints` is the complete resource-visitor example.
