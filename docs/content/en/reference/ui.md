<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Text UI

Creates native text windows, updates tab snapshots, observes readiness, and destroys windows.

<div class="api-meta">

- **Status:** Experimental
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** Creation and destruction coordinate a UI thread
- **Capabilities:** `shroudtopia.ui.overlay`

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|
| [`create_text_window`](#create_text_window) | Create text window. | Experimental |
| [`set_text_window_text`](#set_text_window_text) | Set text window text. | Experimental |
| [`get_text_window_status`](#get_text_window_status) | Read text window status. | Experimental |
| [`destroy_text_window`](#destroy_text_window) | Destroy text window. | Experimental |

## Types

### `TextWindowStatus`

| Value | Numeric value |
|---|---:|
| `TEXT_PENDING` | `0` |
| `TEXT_READY` | `1` |
| `TEXT_FAILED` | `2` |

### `TextWindowOptions`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `title` | `StringView` | Value |
| `tabs` | `const StringView*` | Borrowed or caller-owned; see operation |
| `tab_count` | `size_t` | Value |
| `toggle_key` | `uint32_t` | Value |

<section class="api-function" data-api-name="create_text_window" data-api-status="experimental">

## `create_text_window`

Create text window.

### Signature

```c
Result (CALL* create_text_window)(StringView owner_id, const TextWindowOptions* options, TextWindow* window);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `options` | in | `const TextWindowOptions*` | yes | no | borrowed | Value for options. |
| `window` | in | `TextWindow*` | yes | no | borrowed | Value for window. |

### Results

| Result | Meaning | Recommended handling |
|---|---|---|
| `RESULT_OK` | The operation completed successfully. | Continue with the returned value. |
| `RESULT_INVALID_ARGUMENT` | A pointer, structure size, value, or JSON document is invalid. | Correct the call; do not retry unchanged. |
| `RESULT_CONFLICT` | The requested ownership or mutation conflicts with existing state. | Resolve the competing owner or operation. |
| `RESULT_NOT_FOUND` | The requested resource or provider does not exist. | Check identifiers and availability. |
| `RESULT_VERSION_MISMATCH` | The requested API version does not match API 1.1. | Request API 1.1. |
| `RESULT_PERMISSION_DENIED` | The owner lacks the required permission. | Declare and obtain the required permission. |
| `RESULT_NOT_AVAILABLE` | The feature is unavailable in the current runtime state. | Wait for the required state or degrade gracefully. |
| `RESULT_CALLBACK_FAILED` | A consumer callback returned a failure. | Inspect the callback and its user data. |
| `RESULT_INTERNAL_ERROR` | The loader could not complete an otherwise valid operation. | Log context and fail safely. |

### Example

> Always initialize structures, check Result, and release ownership-bound handles.

```cpp
// Create text window.
Result result = api->create_text_window(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="set_text_window_text" data-api-status="experimental">

## `set_text_window_text`

Set text window text.

### Signature

```c
Result (CALL* set_text_window_text)(StringView owner_id, TextWindow window, size_t tab, StringView text);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `window` | in | `TextWindow` | yes | – | value | Value for window. |
| `tab` | in | `size_t` | yes | – | value | Value for tab. |
| `text` | in | `StringView` | yes | – | borrowed | Value for text. |

### Results

| Result | Meaning | Recommended handling |
|---|---|---|
| `RESULT_OK` | The operation completed successfully. | Continue with the returned value. |
| `RESULT_INVALID_ARGUMENT` | A pointer, structure size, value, or JSON document is invalid. | Correct the call; do not retry unchanged. |
| `RESULT_CONFLICT` | The requested ownership or mutation conflicts with existing state. | Resolve the competing owner or operation. |
| `RESULT_NOT_FOUND` | The requested resource or provider does not exist. | Check identifiers and availability. |
| `RESULT_VERSION_MISMATCH` | The requested API version does not match API 1.1. | Request API 1.1. |
| `RESULT_PERMISSION_DENIED` | The owner lacks the required permission. | Declare and obtain the required permission. |
| `RESULT_NOT_AVAILABLE` | The feature is unavailable in the current runtime state. | Wait for the required state or degrade gracefully. |
| `RESULT_CALLBACK_FAILED` | A consumer callback returned a failure. | Inspect the callback and its user data. |
| `RESULT_INTERNAL_ERROR` | The loader could not complete an otherwise valid operation. | Log context and fail safely. |

### Example

> Always initialize structures, check Result, and release ownership-bound handles.

```cpp
// Set text window text.
Result result = api->set_text_window_text(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="get_text_window_status" data-api-status="experimental">

## `get_text_window_status`

Read text window status.

### Signature

```c
Result (CALL* get_text_window_status)(StringView owner_id, TextWindow window, TextWindowStatus* status);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `window` | in | `TextWindow` | yes | – | value | Value for window. |
| `status` | in | `TextWindowStatus*` | yes | no | borrowed | Value for status. |

### Results

| Result | Meaning | Recommended handling |
|---|---|---|
| `RESULT_OK` | The operation completed successfully. | Continue with the returned value. |
| `RESULT_INVALID_ARGUMENT` | A pointer, structure size, value, or JSON document is invalid. | Correct the call; do not retry unchanged. |
| `RESULT_CONFLICT` | The requested ownership or mutation conflicts with existing state. | Resolve the competing owner or operation. |
| `RESULT_NOT_FOUND` | The requested resource or provider does not exist. | Check identifiers and availability. |
| `RESULT_VERSION_MISMATCH` | The requested API version does not match API 1.1. | Request API 1.1. |
| `RESULT_PERMISSION_DENIED` | The owner lacks the required permission. | Declare and obtain the required permission. |
| `RESULT_NOT_AVAILABLE` | The feature is unavailable in the current runtime state. | Wait for the required state or degrade gracefully. |
| `RESULT_CALLBACK_FAILED` | A consumer callback returned a failure. | Inspect the callback and its user data. |
| `RESULT_INTERNAL_ERROR` | The loader could not complete an otherwise valid operation. | Log context and fail safely. |

### Example

> Always initialize structures, check Result, and release ownership-bound handles.

```cpp
// Read text window status.
Result result = api->get_text_window_status(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="destroy_text_window" data-api-status="experimental">

## `destroy_text_window`

Destroy text window.

### Signature

```c
Result (CALL* destroy_text_window)(StringView owner_id, TextWindow window);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `window` | in | `TextWindow` | yes | – | value | Value for window. |

### Results

| Result | Meaning | Recommended handling |
|---|---|---|
| `RESULT_OK` | The operation completed successfully. | Continue with the returned value. |
| `RESULT_INVALID_ARGUMENT` | A pointer, structure size, value, or JSON document is invalid. | Correct the call; do not retry unchanged. |
| `RESULT_CONFLICT` | The requested ownership or mutation conflicts with existing state. | Resolve the competing owner or operation. |
| `RESULT_NOT_FOUND` | The requested resource or provider does not exist. | Check identifiers and availability. |
| `RESULT_VERSION_MISMATCH` | The requested API version does not match API 1.1. | Request API 1.1. |
| `RESULT_PERMISSION_DENIED` | The owner lacks the required permission. | Declare and obtain the required permission. |
| `RESULT_NOT_AVAILABLE` | The feature is unavailable in the current runtime state. | Wait for the required state or degrade gracefully. |
| `RESULT_CALLBACK_FAILED` | A consumer callback returned a failure. | Inspect the callback and its user data. |
| `RESULT_INTERNAL_ERROR` | The loader could not complete an otherwise valid operation. | Log context and fail safely. |

### Example

> Always initialize structures, check Result, and release ownership-bound handles.

```cpp
// Destroy text window.
Result result = api->destroy_text_window(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
