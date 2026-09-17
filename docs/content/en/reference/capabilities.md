<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Capability information

Describes the negotiated version and availability of a capability.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** See query_capability
- **Capabilities:** None

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|
| [`query_capability`](#query_capability) | Read availability and version information for a capability. | Stable |
| [`check_permission`](#check_permission) | Check whether an owner may use a permission. | Stable |

## Types

### `CapabilityInfo`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `version_major` | `uint32_t` | Value |
| `version_minor` | `uint32_t` | Value |
| `flags` | `uint64_t` | Value |
| `available` | `uint8_t` | Value |
| `reserved` | `uint8_t` | Value |

<section class="api-function" data-api-name="query_capability" data-api-status="stable">

## `query_capability`

Read availability and version information for a capability.

### Signature

```c
Result (CALL* query_capability)(StringView capability_id, CapabilityInfo* information);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `capability_id` | in | `StringView` | yes | – | borrowed | Value for capability id. |
| `information` | out | `CapabilityInfo*` | yes | no | caller-owned | Value for information. |

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
// Read availability and version information for a capability.
Result result = api->query_capability(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="check_permission" data-api-status="stable">

## `check_permission`

Check whether an owner may use a permission.

### Signature

```c
Result (CALL* check_permission)(StringView owner_id, StringView permission_id, uint8_t* allowed);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `permission_id` | in | `StringView` | yes | – | borrowed | Value for permission id. |
| `allowed` | out | `uint8_t*` | yes | no | caller-owned | Value for allowed. |

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
// Check whether an owner may use a permission.
Result result = api->check_permission(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
