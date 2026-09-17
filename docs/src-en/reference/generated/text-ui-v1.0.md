<!-- Generated from docs/contracts/text-ui-v1.0.yaml; do not edit by hand. -->
# Native Text UI API

Native tabbed text windows for windowed and borderless clients.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia loader on Windows clients
- **Capability:** None
- **Header:** `api/include/shroudtopia/api/ui.h`

</div>

## Types

### `ST_TextWindow`

Opaque owner-bound native window.

### `ST_TextWindowStatusV1`

Asynchronous creation state.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_TEXT_PENDING` | `0` | Creation pending. | — |
| `ST_TEXT_READY` | `1` | Window ready. | — |
| `ST_TEXT_FAILED` | `2` | Creation failed. | — |

### `ST_TextWindowDescriptorV1`

Native tabbed window definition.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `title` | `ST_StringView` | Window title, at most 128 bytes. | — |
| `tabs` | `const ST_StringView*` | One to eight tab labels. | — |
| `tab_count` | `size_t` | Number of tabs, 1..8. | — |
| `toggle_key` | `uint32_t` | Windows virtual key; zero disables. | — |

### `ST_UiTextApiV1`

Versioned service function table.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `abi_version` | `uint32_t` | abi version. | — |
| `create` | `function pointer` | create. | — |
| `set_text` | `function pointer` | set text. | — |
| `get_status` | `function pointer` | get status. | — |
| `destroy` | `function pointer` | destroy. | — |

## Functions

<section class="api-function" data-api-name="create" data-api-status="stable">

### `create`

Begin asynchronous native window creation.

```c
ST_Result create(ST_StringView owner, const ST_TextWindowDescriptorV1* descriptor, ST_TextWindow* window);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner` | `ST_StringView` | in | yes | Window owner. |
| `descriptor` | `const ST_TextWindowDescriptorV1*` | in | yes | Window and tabs. |
| `window` | `ST_TextWindow*` | out | yes | Receives window handle. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner, descriptor, window according to the parameter table.
const ST_Result status = api->create(owner, descriptor, window);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_text" data-api-status="stable">

### `set_text`

Replace one tab snapshot.

```c
ST_Result set_text(ST_StringView owner, ST_TextWindow window, size_t tab, ST_StringView text);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner` | `ST_StringView` | in | yes | Window owner. |
| `window` | `ST_TextWindow` | in | yes | Owned window. |
| `tab` | `size_t` | in | yes | Zero-based tab index. |
| `text` | `ST_StringView` | in | yes | Copied UTF-8, at most one MiB. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner, window, tab, text according to the parameter table.
const ST_Result status = api->set_text(owner, window, tab, text);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_status" data-api-status="stable">

### `get_status`

Read asynchronous creation status.

```c
ST_Result get_status(ST_StringView owner, ST_TextWindow window, ST_TextWindowStatusV1* status);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner` | `ST_StringView` | in | yes | Window owner. |
| `window` | `ST_TextWindow` | in | yes | Owned window. |
| `status` | `ST_TextWindowStatusV1*` | out | yes | Receives current status. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner, window, status according to the parameter table.
const ST_Result status = api->get_status(owner, window, status);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="destroy" data-api-status="stable">

### `destroy`

Destroy the window and join its UI thread.

```c
ST_Result destroy(ST_StringView owner, ST_TextWindow window);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner` | `ST_StringView` | in | yes | Window owner. |
| `window` | `ST_TextWindow` | in | yes | Owned window. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner, window according to the parameter table.
const ST_Result status = api->destroy(owner, window);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
