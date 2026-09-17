<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Logging

Writes structured loader messages and reads bounded tails of loader or game logs.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** Thread-safe where stated
- **Capabilities:** None

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|
| [`log`](#log) | Write . | Stable |
| [`read_log_tail`](#read_log_tail) | Execute log tail. | Stable |

## Types

### `LogLevel`

| Value | Numeric value |
|---|---:|
| `LOG_TRACE` | `0` |
| `LOG_DEBUG` | `1` |
| `LOG_INFO` | `2` |
| `LOG_WARNING` | `3` |
| `LOG_ERROR` | `4` |

### `LogSource`

| Value | Numeric value |
|---|---:|
| `LOG_SOURCE_LOADER` | `0` |
| `LOG_SOURCE_GAME` | `1` |

<section class="api-function" data-api-name="log" data-api-status="stable">

## `log`

Write .

### Signature

```c
Result (CALL* log)(StringView owner_id, LogLevel level, StringView message);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `level` | in | `LogLevel` | yes | – | value | Value for level. |
| `message` | in | `StringView` | yes | – | borrowed | Value for message. |

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
// Write .
Result result = api->log(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="read_log_tail" data-api-status="stable">

## `read_log_tail`

Execute log tail.

### Signature

```c
Result (CALL* read_log_tail)(LogSource source, char* buffer, size_t capacity, size_t* written);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `source` | in | `LogSource` | yes | – | value | Value for source. |
| `buffer` | in | `char*` | conditional | yes, for capacity query | borrowed | Caller-owned output buffer; may be null for a size query. |
| `capacity` | in | `size_t` | yes | – | value | Elements or bytes available in the output buffer. |
| `written` | out | `size_t*` | yes | no | caller-owned | Value for written. |

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
// Execute log tail.
Result result = api->read_log_tail(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
