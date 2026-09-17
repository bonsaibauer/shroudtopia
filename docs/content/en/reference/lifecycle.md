<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Mod lifecycle

Declares a mod and its load, activation, update, deactivation, and unload callbacks.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** Loader-controlled lifecycle thread
- **Capabilities:** `shroudtopia.lifecycle.native`

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|
| [`ModLifecycleCallback`](#modlifecyclecallback) | Execute . | Stable |
| [`ModUpdateCallback`](#modupdatecallback) | Execute . | Stable |
| [`CreateModFunction`](#createmodfunction) | Execute . | Stable |

## Types

### `ModDescriptor`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `mod_id` | `StringView` | Value |
| `user_data` | `void*` | Borrowed or caller-owned; see operation |
| `on_load` | `ModLifecycleCallback` | Value |
| `on_activate` | `ModLifecycleCallback` | Value |
| `on_update` | `ModUpdateCallback` | Value |
| `on_deactivate` | `ModLifecycleCallback` | Value |
| `on_unload` | `ModLifecycleCallback` | Value |

<section class="api-function" data-api-name="modlifecyclecallback" data-api-status="stable">

## `ModLifecycleCallback`

Execute .

### Signature

```c
Result (CALL* ModLifecycleCallback)(const Api* api, void* user_data);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `api` | in | `const Api*` | yes | no | borrowed | Value for api. |
| `user_data` | in | `void*` | yes | no | borrowed | Opaque context forwarded unchanged to the callback. |

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
// Execute .
Result result = ModLifecycleCallback(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="modupdatecallback" data-api-status="stable">

## `ModUpdateCallback`

Execute .

### Signature

```c
Result (CALL* ModUpdateCallback)(const Api* api, void* user_data, double delta_seconds);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `api` | in | `const Api*` | yes | no | borrowed | Value for api. |
| `user_data` | in | `void*` | yes | no | borrowed | Opaque context forwarded unchanged to the callback. |
| `delta_seconds` | in | `double` | yes | – | value | Value for delta seconds. |

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
// Execute .
Result result = ModUpdateCallback(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="createmodfunction" data-api-status="stable">

## `CreateModFunction`

Execute .

### Signature

```c
Result (CALL* CreateModFunction)(uint32_t requested_api_version, ModDescriptor* descriptor);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `requested_api_version` | in | `uint32_t` | yes | – | value | Value for requested api version. |
| `descriptor` | in | `ModDescriptor*` | yes | no | borrowed | Value for descriptor. |

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
// Execute .
Result result = CreateModFunction(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
