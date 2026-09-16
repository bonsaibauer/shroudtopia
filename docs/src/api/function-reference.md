# Function Reference and Status

## Host table

| Status | Function | Purpose |
|:---:|---|---|
| ✅ | `register_service` | Publish a versioned provider. |
| ✅ | `find_service` | Find the highest compatible provider. |
| ✅ | `subscribe_event` | Register a synchronous callback. |
| ✅ | `publish_event` | Deliver a borrowed payload. |
| ✅ | `register_command` | Register a global command ID. |
| ✅ | `execute_command` | Invoke a command. |
| ✅ | `register_settings` | Publish schema and defaults. |
| ✅ | `release_registration` | Remove one registration. |
| ✅ | `release_owner` | Remove all owner resources. |
| ✅ | `query_capability` | Query feature availability. |
| ✅ | `check_permission` | Query owner permission. |
| ✅ | `log` | Write an owner-tagged message. |
| ✅ | `get_setting_bool` | Read a Boolean setting. |
| ✅ | `get_setting_number` | Read a finite number setting. |

## Built-in services

| Status | Service | Version | Functions |
|:---:|---|:---:|---|
| ✅ | `shroudtopia.logging.read` | 1.0 | `read_tail` |
| ✅ | `shroudtopia.ui.text` | 1.0 | `create`, `set_text`, `get_status`, `destroy` |
| ✅ | `shroudtopia.runtime.patches` | 1.0 | `create`, `set_enabled`, `get_state`, `release` |
| ✅ | `shroudtopia.assets` | 1.1 | visit, read, replace, create, field update, discard, flush |
| 🧪 | `shroudtopia.world` | 1.1 | entity and grid operations |

## World functions

| Status | Function | Purpose |
|:---:|---|---|
| 🧪 | `query_entities` | Return complete entity snapshots for a region. |
| 🧪 | `get_entity_transform` | Read a session-bound entity transform. |
| 🧪 | `spawn_entity` | Create an entity from a template. |
| 🧪 | `destroy_entity` | Remove an entity. |
| 🧪 | `set_entity_transform` | Change an entity transform. |
| 🧪 | `get_grid_spec` | Read grid origin, cell size, and chunk dimensions. |
| 🧪 | `read_grid_region` | Read values plus explicit coverage. |
| 🧪 | `write_grid_region` | Write known cells in one aligned region. |
| ✅ | `ST_GridRegionFromPoints` | Convert two positions into a bounded grid region. |

`🧪` World functions pass against the ShroudEdit test provider. Shroudtopia does not
register a production World provider in Enshrouded.

## API gaps

| Status | Area |
|:---:|---|
| 🚧 | Target snapshots and semantic tool actions |
| 🚧 | Game-thread jobs and progress |
| 🚧 | World snapshots, revisions, and server authorization |
| 🚧 | Renderer overlay and ghost instances |
| 🚧 | Archived-log enumeration |
| ❌ | Settings string/object readers and generated settings UI |
