<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# API

Negotiates and exposes the single flat Shroudtopia interface used by every native mod.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** Main thread unless an operation states otherwise
- **Capabilities:** None

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|
| [`ShroudtopiaGetApi`](#shroudtopiagetapi) | Negotiate the requested ABI version and return the process-wide host API. | Stable |

## Types

### `Api`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `api_version` | `uint32_t` | Value |

<section class="api-function" data-api-name="shroudtopiagetapi" data-api-status="stable">

## `ShroudtopiaGetApi`

Negotiate the requested ABI version and return the process-wide host API.

### Signature

```c
API_EXPORT Result CALL ShroudtopiaGetApi(uint32_t requested_api_version, const Api** api);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `requested_api_version` | in | `uint32_t` | yes | – | value | Value for requested api version. |
| `api` | in | `const Api**` | yes | no | borrowed | Value for api. |

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
// Negotiate the requested ABI version and return the process-wide host API.
Result result = ShroudtopiaGetApi(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
