<!-- Generated from docs/contracts/world-v1.1.yaml; do not edit by hand. -->
# World API

Entity snapshots, transforms, and explicit-coverage world grids.

<div class="api-meta" data-api-status="experimental">

- **Status:** 🧪 experimental
- **Version:** `1.1`
- **Provider:** ShroudEdit controlled test provider only
- **Capability:** `shroudtopia.world.entities.* / shroudtopia.world.voxels.*`
- **Header:** `api/include/shroudtopia/api/world.h`

</div>

## Types

### `ST_Vec3d`

Three-dimensional double vector.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `x` | `double` | X component. | — |
| `y` | `double` | Y component. | — |
| `z` | `double` | Z component. | — |

### `ST_Quaterniond`

Quaternion rotation.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `x` | `double` | X component. | — |
| `y` | `double` | Y component. | — |
| `z` | `double` | Z component. | — |
| `w` | `double` | W component. | — |

### `ST_TransformV1`

Position, rotation, and scale.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `position` | `ST_Vec3d` | World position. | — |
| `rotation` | `ST_Quaterniond` | Quaternion rotation. | — |
| `scale` | `ST_Vec3d` | Per-axis scale. | — |

### `ST_AabbV1`

Half-open world-space bounds.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `minimum` | `ST_Vec3d` | Inclusive minimum. | — |
| `maximum` | `ST_Vec3d` | Exclusive maximum. | — |

### `ST_EntityHandleV1`

Session- and generation-bound entity identity.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `id` | `uint64_t` | Entity ID. | — |
| `generation` | `uint32_t` | Reuse generation. | — |
| `session` | `uint32_t` | World session. | — |

### `ST_EntityKindV1`

Supported entity classification.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_ENTITY_UNKNOWN` | `0` | Unsafe to copy. | — |
| `ST_ENTITY_PROP` | `1` | Supported static template instance. | — |
| `ST_ENTITY_OTHER` | `2` | Player, NPC, or unsupported dynamic object. | — |

### `ST_EntitySnapshotV1`

Complete query snapshot.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `handle` | `ST_EntityHandleV1` | Entity handle. | — |
| `template_id` | `ST_StringView` | Template identity. | Valid until the next world call on this thread. |
| `transform` | `ST_TransformV1` | Snapshot transform. | — |
| `kind` | `ST_EntityKindV1` | Entity classification. | — |

### `ST_CoverageV1`

Meaning of one grid cell.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_COVERAGE_UNKNOWN` | `0` | Not loaded or not known; never air. | — |
| `ST_COVERAGE_EMPTY` | `1` | Known empty cell. | — |
| `ST_COVERAGE_OCCUPIED` | `2` | Known occupied cell. | — |

### `ST_GridSpecV1`

World grid geometry.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `grid_id` | `ST_StringView` | Grid identifier. | — |
| `origin` | `ST_Vec3d` | Grid origin. | — |
| `cell_size` | `ST_Vec3d` | Positive cell dimensions. | — |
| `chunk_size_x` | `uint32_t` | Chunk width. | — |
| `chunk_size_y` | `uint32_t` | Chunk height. | — |
| `chunk_size_z` | `uint32_t` | Chunk depth. | — |

### `ST_WorldApiV1`

Versioned service function table.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `api_version` | `uint32_t` | api version. | — |
| `query_entities` | `function pointer` | query entities. | — |
| `get_entity_transform` | `function pointer` | get entity transform. | — |
| `spawn_entity` | `function pointer` | spawn entity. | — |
| `destroy_entity` | `function pointer` | destroy entity. | — |
| `set_entity_transform` | `function pointer` | set entity transform. | — |
| `get_grid_spec` | `function pointer` | get grid spec. | — |
| `read_grid_region` | `function pointer` | read grid region. | — |
| `write_grid_region` | `function pointer` | write grid region. | — |

### `ST_GridRegionV1`

Bounded aligned grid region.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure size. | — |
| `bounds` | `ST_AabbV1` | Half-open world bounds. | — |
| `minimum` | `int32_t[3]` | Minimum cell indices. | — |
| `dimensions` | `uint32_t[3]` | Cell count per axis. | — |
| `cell_count` | `size_t` | Total cell count. | — |

## Functions

<section class="api-function" data-api-name="query_entities" data-api-status="experimental">

### `query_entities`

Return complete entity snapshots in a half-open region.

```c
ST_Result query_entities(ST_StringView owner_id, const ST_AabbV1* region, ST_EntitySnapshotV1* entities, size_t capacity, size_t* required_count);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.entities.read`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `region` | `const ST_AabbV1*` | in | yes | Half-open world bounds. |
| `entities` | `ST_EntitySnapshotV1*` | out | no | Caller-owned initialized array. |
| `capacity` | `size_t` | in | yes | Array element capacity. |
| `required_count` | `size_t*` | out | yes | Receives required elements. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, region, entities, capacity, required_count according to the parameter table.
const ST_Result status = api->query_entities(owner_id, region, entities, capacity, required_count);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_entity_transform" data-api-status="experimental">

### `get_entity_transform`

Read an entity transform.

```c
ST_Result get_entity_transform(ST_StringView owner_id, ST_EntityHandleV1 entity, ST_TransformV1* transform);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.entities.read`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `entity` | `ST_EntityHandleV1` | in | yes | Session-bound entity. |
| `transform` | `ST_TransformV1*` | inout | yes | Initialized output. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, entity, transform according to the parameter table.
const ST_Result status = api->get_entity_transform(owner_id, entity, transform);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="spawn_entity" data-api-status="experimental">

### `spawn_entity`

Spawn a supported template.

```c
ST_Result spawn_entity(ST_StringView owner_id, ST_StringView template_id, const ST_TransformV1* transform, ST_EntityHandleV1* entity);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.entities.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `template_id` | `ST_StringView` | in | yes | Supported template. |
| `transform` | `const ST_TransformV1*` | in | yes | Initial transform. |
| `entity` | `ST_EntityHandleV1*` | out | yes | Receives entity handle. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, template_id, transform, entity according to the parameter table.
const ST_Result status = api->spawn_entity(owner_id, template_id, transform, entity);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="destroy_entity" data-api-status="experimental">

### `destroy_entity`

Destroy a supported entity.

```c
ST_Result destroy_entity(ST_StringView owner_id, ST_EntityHandleV1 entity);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.entities.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `entity` | `ST_EntityHandleV1` | in | yes | Entity to destroy. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, entity according to the parameter table.
const ST_Result status = api->destroy_entity(owner_id, entity);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_entity_transform" data-api-status="experimental">

### `set_entity_transform`

Change an entity transform.

```c
ST_Result set_entity_transform(ST_StringView owner_id, ST_EntityHandleV1 entity, const ST_TransformV1* transform);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.entities.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `entity` | `ST_EntityHandleV1` | in | yes | Entity to update. |
| `transform` | `const ST_TransformV1*` | in | yes | New transform. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, entity, transform according to the parameter table.
const ST_Result status = api->set_entity_transform(owner_id, entity, transform);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_grid_spec" data-api-status="experimental">

### `get_grid_spec`

Read grid geometry.

```c
ST_Result get_grid_spec(ST_StringView owner_id, ST_StringView grid_id, ST_GridSpecV1* grid);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.voxels.read`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `grid_id` | `ST_StringView` | in | yes | Grid identity. |
| `grid` | `ST_GridSpecV1*` | inout | yes | Initialized output. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, grid_id, grid according to the parameter table.
const ST_Result status = api->get_grid_spec(owner_id, grid_id, grid);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="read_grid_region" data-api-status="experimental">

### `read_grid_region`

Read aligned cell values and explicit coverage.

```c
ST_Result read_grid_region(ST_StringView owner_id, ST_StringView grid_id, const ST_AabbV1* region, uint32_t* values, ST_CoverageV1* coverage, size_t capacity, size_t* required_count);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.voxels.read`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `grid_id` | `ST_StringView` | in | yes | Grid identity. |
| `region` | `const ST_AabbV1*` | in | yes | Grid-aligned bounds. |
| `values` | `uint32_t*` | out | yes | X-fastest caller buffer. |
| `coverage` | `ST_CoverageV1*` | out | yes | Parallel coverage buffer. |
| `capacity` | `size_t` | in | yes | Cells available. |
| `required_count` | `size_t*` | out | yes | Required cells. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, grid_id, region, values, coverage, capacity, required_count according to the parameter table.
const ST_Result status = api->read_grid_region(owner_id, grid_id, region, values, coverage, capacity, required_count);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="write_grid_region" data-api-status="experimental">

### `write_grid_region`

Write known cells in an aligned region.

```c
ST_Result write_grid_region(ST_StringView owner_id, ST_StringView grid_id, const ST_AabbV1* region, const uint32_t* values, const ST_CoverageV1* coverage, size_t count);
```

- **Status:** 🧪 experimental
- **Since:** `1.0`
- **Permission:** `shroudtopia.world.voxels.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `grid_id` | `ST_StringView` | in | yes | Grid identity. |
| `region` | `const ST_AabbV1*` | in | yes | Grid-aligned bounds. |
| `values` | `const uint32_t*` | in | yes | X-fastest values. |
| `coverage` | `const ST_CoverageV1*` | in | yes | UNKNOWN cells remain unchanged. |
| `count` | `size_t` | in | yes | Exact cell count. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, grid_id, region, values, coverage, count according to the parameter table.
const ST_Result status = api->write_grid_region(owner_id, grid_id, region, values, coverage, count);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="st_gridregionfrompoints" data-api-status="stable">

### `ST_GridRegionFromPoints`

Convert two included points into bounded aligned half-open grid bounds.

```c
ST_Result ST_GridRegionFromPoints(const ST_GridSpecV1* grid, ST_Vec3d a, ST_Vec3d b, size_t maximum_cells, ST_GridRegionV1* output);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Pure, reentrant, and callable on any thread.
- **Ownership:** No allocation; caller owns input and output.
- **Side effects:** None.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `grid` | `const ST_GridSpecV1*` | in | yes | Initialized grid geometry. |
| `a` | `ST_Vec3d` | in | yes | First included point. |
| `b` | `ST_Vec3d` | in | yes | Second included point. |
| `maximum_cells` | `size_t` | in | yes | Nonzero allocation budget. |
| `output` | `ST_GridRegionV1*` | inout | yes | Initialized result. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | A bounded region was written. |
| `ST_RESULT_INVALID_ARGUMENT` | Input is invalid, non-finite, overflowing, or over budget. |

#### Example

```cpp
ST_GridRegionV1 region{sizeof(region)};
ST_Result status = ST_GridRegionFromPoints(&grid, a, b, 1u << 20, &region);
```

</section>
