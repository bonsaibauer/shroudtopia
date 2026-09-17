<!-- Generated from docs/contracts/lifecycle-v1.0.yaml; do not edit by hand. -->
# Lifecycle API

Native mod discovery, creation, activation, update, and deterministic teardown.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia loader
- **Capability:** `shroudtopia.lifecycle.native`
- **Header:** `api/include/shroudtopia/api/lifecycle.h`

</div>

## Types

### `ST_ModDescriptorV1`

Native mod identity, state, and lifecycle callbacks.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure size. | — |
| `mod_id` | `ST_StringView` | Globally unique stable mod ID. | — |
| `user_data` | `void*` | Mod-owned callback context. | — |
| `on_load` | `ST_ModLifecycleCallback` | Optional registration callback. | — |
| `on_activate` | `ST_ModLifecycleCallback` | Optional activation callback. | — |
| `on_update` | `ST_ModUpdateCallback` | Optional bounded worker-thread update. | — |
| `on_deactivate` | `ST_ModLifecycleCallback` | Optional deactivation callback. | — |
| `on_unload` | `ST_ModLifecycleCallback` | Optional final cleanup callback. | — |

## Functions

<section class="api-function" data-api-name="shroudtopiagetapi" data-api-status="stable">

### `ShroudtopiaGetApi`

Request the loader host table.

```c
ST_Result ShroudtopiaGetApi(uint32_t requested_abi, const ST_HostApiV1** api);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Call during native bootstrap on the loading thread.
- **Ownership:** The loader owns the returned table for the loaded mod lifetime.
- **Side effects:** No persistent side effect.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `requested_abi` | `uint32_t` | in | yes | Required ABI constant. |
| `api` | `const ST_HostApiV1**` | out | yes | Receives the loader-owned table. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The compatible table was returned. |
| `ST_RESULT_INVALID_ARGUMENT` | The output pointer is null. |
| `ST_RESULT_VERSION_MISMATCH` | The ABI is unsupported. |

#### Example

```cpp
const ST_HostApiV1* host = nullptr;
ST_Result status = ShroudtopiaGetApi(ST_ABI_VERSION_1, &host);
```

</section>
