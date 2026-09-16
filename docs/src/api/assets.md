# Asset API

## Status

| Status | Contract or feature |
|:---:|---|
| ✅ | `shroudtopia.assets@1.1` |
| ✅ | Typed KFC3 read and write |
| ✅ | Ownership and conflict detection |
| ✅ | Staging, backup, and recovery journal |
| ❌ | Live reload of resources already loaded by the game |

Mods use `ST_AssetsApiV1`. Shroudtopia owns KFC3 parsing, reflection, JSON
conversion, validation, backup, and persistence.

```text
Mod → ST_AssetsApiV1 → permission and ownership → Shroudtopia asset engine → KFC3
```

## Required capabilities

```json
{
  "requires": {
    "capabilities": ["shroudtopia.assets.read", "shroudtopia.assets.write"]
  }
}
```

## Functions

| Function | Purpose |
|---|---|
| `visit_resources` | Visit a snapshot of resources with one qualified type. |
| `read_resource_json` | Read typed JSON using caller-owned storage. |
| `replace_resource_json` | Replace one complete resource document. |
| `create_resource_json` | Create one typed resource. |
| `set_resource_field_json` | Set one existing JSON-pointer field. |
| `discard_changes` | Remove changes owned by one mod. |
| `flush` | Publish the combined overlay through staging and recovery. |

`ST_AssetResourceKeyV1` contains GUID, qualified type name, and part index. Visitor
keys are borrowed for the callback. A reader performs a zero-capacity size query,
allocates its buffer, and performs the data read.

## Field ownership

`set_resource_field_json` accepts an existing JSON Pointer such as `/maxStackSize`.
The final argument is one JSON value, not a full document. Disjoint fields from
different owners compose. Equal, parent, child, or full-document overlap returns
`ST_RESULT_ALREADY_EXISTS`. `discard_changes` affects the supplied owner only.

`flush` confirms file publication. It does not promise that a running game reloads
resources already held in memory.
