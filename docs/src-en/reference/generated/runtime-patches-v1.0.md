<!-- Generated from docs/contracts/runtime-patches-v1.0.yaml; do not edit by hand. -->
# Runtime Patch API

Controlled signature-based direct patches and x64 detours.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia loader
- **Capability:** `shroudtopia.runtime.patches`
- **Header:** `api/include/shroudtopia/api/runtime.h`

</div>

## Types

### `ST_RuntimePatch`

Opaque owner-bound patch handle.

### `ST_RuntimePatchKindV1`

Patch application strategy.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_RUNTIME_PATCH_DIRECT` | `0` | Direct replacement bytes. | — |
| `ST_RUNTIME_PATCH_DETOUR` | `1` | Near x64 detour. | — |

### `ST_RuntimeRelocationKindV1`

Supported payload relocation.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_RUNTIME_RELOCATION_REL32_RETURN` | `1` | Write a relative 32-bit return displacement. | — |

### `ST_RuntimeRelocationV1`

Payload relocation instruction.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `payload_offset` | `size_t` | Offset within payload. | — |
| `kind` | `ST_RuntimeRelocationKindV1` | Supported relocation kind. | — |

### `ST_RuntimePatchDescriptorV1`

Complete inactive patch definition.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `signature` | `ST_StringView` | Executable-section byte signature. | — |
| `match_offset` | `int64_t` | Signed offset from unique match. | — |
| `kind` | `ST_RuntimePatchKindV1` | Direct or detour. | — |
| `overwrite_size` | `size_t` | Complete instruction bytes overwritten. | — |
| `payload` | `const uint8_t*` | Replacement payload. | — |
| `payload_size` | `size_t` | Payload bytes. | — |
| `relocations` | `const ST_RuntimeRelocationV1*` | Relocation array. | — |
| `relocation_count` | `size_t` | Relocation count. | — |

### `ST_RuntimePatchStateV1`

Current patch state.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `enabled` | `uint8_t` | One if applied. | — |
| `reserved` | `uint8_t[7]` | Reserved, zero. | — |

### `ST_RuntimePatchesApiV1`

Versioned service function table.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `create` | `function pointer` | create. | — |
| `set_enabled` | `function pointer` | set enabled. | — |
| `get_state` | `function pointer` | get state. | — |
| `release` | `function pointer` | release. | — |

## Functions

<section class="api-function" data-api-name="create" data-api-status="stable">

### `create`

Validate and create an inactive patch.

```c
ST_Result create(ST_StringView owner_id, const ST_RuntimePatchDescriptorV1* descriptor, ST_RuntimePatch* patch);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.runtime.patches`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Patch owner. |
| `descriptor` | `const ST_RuntimePatchDescriptorV1*` | in | yes | Complete patch definition. |
| `patch` | `ST_RuntimePatch*` | out | yes | Receives patch handle. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, descriptor, patch according to the parameter table.
const ST_Result status = api->create(owner_id, descriptor, patch);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_enabled" data-api-status="stable">

### `set_enabled`

Apply or restore patch bytes.

```c
ST_Result set_enabled(ST_StringView owner_id, ST_RuntimePatch patch, uint8_t enabled);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.runtime.patches`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Patch owner. |
| `patch` | `ST_RuntimePatch` | in | yes | Owned patch. |
| `enabled` | `uint8_t` | in | yes | One to apply, zero to restore. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, patch, enabled according to the parameter table.
const ST_Result status = api->set_enabled(owner_id, patch, enabled);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_state" data-api-status="stable">

### `get_state`

Read whether a patch is enabled.

```c
ST_Result get_state(ST_StringView owner_id, ST_RuntimePatch patch, ST_RuntimePatchStateV1* state);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.runtime.patches`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Patch owner. |
| `patch` | `ST_RuntimePatch` | in | yes | Owned patch. |
| `state` | `ST_RuntimePatchStateV1*` | inout | yes | Initialized output state. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, patch, state according to the parameter table.
const ST_Result status = api->get_state(owner_id, patch, state);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="release" data-api-status="stable">

### `release`

Restore and destroy a patch.

```c
ST_Result release(ST_StringView owner_id, ST_RuntimePatch patch);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.runtime.patches`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Patch owner. |
| `patch` | `ST_RuntimePatch` | in | yes | Owned patch. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, patch according to the parameter table.
const ST_Result status = api->release(owner_id, patch);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
