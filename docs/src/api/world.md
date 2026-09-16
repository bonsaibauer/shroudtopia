# World Contract

## Status

| Status | Area |
|:---:|---|
| ✅ | `shroudtopia.world@1.1` header contract |
| ✅ | Pure grid-region calculation |
| 🧪 | ShroudEdit controlled test provider |
| ❌ | Production Enshrouded provider |

The contract is general and independent of ShroudEdit.

## Entity operations

| Function | Purpose |
|---|---|
| `query_entities` | Return complete snapshots inside a half-open world region. |
| `get_entity_transform` | Read a generation- and session-bound handle. |
| `spawn_entity` | Create a supported entity from a template ID. |
| `destroy_entity` | Remove a supported entity. |
| `set_entity_transform` | Change position, rotation, and scale. |

`ST_ENTITY_PROP` identifies supported static template instances. Players, NPCs, and
unsupported dynamic objects use `ST_ENTITY_OTHER`. `ST_ENTITY_UNKNOWN` is not safe
to copy as a prop.

## Grid operations

| Function | Purpose |
|---|---|
| `get_grid_spec` | Return grid ID, origin, cell size, and chunk dimensions. |
| `read_grid_region` | Return values and explicit coverage for an aligned region. |
| `write_grid_region` | Write known cells in an aligned region. |
| `ST_GridRegionFromPoints` | Convert two included world points into a bounded region. |

Buffers use `x + width * (y + height * z)`. Coverage is
`UNKNOWN`, `EMPTY`, or `OCCUPIED`. Unknown is not empty and writers leave unknown
cells unchanged. Failed writes may be partial. Callers retain recovery data.

The pure helper uses grid origin and cell size, applies `floor` for negative
coordinates, includes both marked cells, emits half-open bounds, and enforces the
caller-supplied cell budget.

Shroudtopia does not register this service in the game process. Mods must handle
`ST_RESULT_NOT_FOUND` or `ST_RESULT_UNSUPPORTED`.
