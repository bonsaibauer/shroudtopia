<!-- Generated from docs/contracts/logging-v1.0.yaml; do not edit by hand. -->
# Logging API

Owner-tagged writes through Host API and bounded reads of current logs.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia loader
- **Capability:** None
- **Header:** `api/include/shroudtopia/api/logging.h`

</div>

## Types

### `ST_LogLevel`

Message severity.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_LOG_TRACE` | `0` | Focused high-frequency diagnostics. | — |
| `ST_LOG_DEBUG` | `1` | Technical decisions and state. | — |
| `ST_LOG_INFO` | `2` | Successful user-relevant change. | — |
| `ST_LOG_WARNING` | `3` | Restricted or unsupported operation. | — |
| `ST_LOG_ERROR` | `4` | Requested operation failed. | — |

### `ST_LogSourceV1`

Readable current log source.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_LOG_SOURCE_LOADER` | `0` | Shroudtopia and mod log. | — |
| `ST_LOG_SOURCE_GAME` | `1` | Enshrouded game log. | — |

### `ST_LogReadApiV1`

Current-log reader table.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `abi_version` | `uint32_t` | Must equal ST_ABI_VERSION_1. | — |
| `read_tail` | `function pointer` | Bounded tail reader. | — |

## Functions

<section class="api-function" data-api-name="read_tail" data-api-status="stable">

### `read_tail`

Read at most one MiB from the current log tail.

```c
ST_Result read_tail(ST_LogSourceV1 source, char* buffer, size_t capacity, size_t* written);
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
| `source` | `ST_LogSourceV1` | in | yes | Loader or game log. |
| `buffer` | `char*` | out | yes | Caller-owned byte buffer. |
| `capacity` | `size_t` | in | yes | Between 1 and 1,048,576 bytes. |
| `written` | `size_t*` | out | yes | Receives bytes written; no terminator. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_NOT_FOUND` | The current log file does not exist. |

#### Example

```cpp
// Initialize source, buffer, capacity, written according to the parameter table.
const ST_Result status = api->read_tail(source, buffer, capacity, written);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
