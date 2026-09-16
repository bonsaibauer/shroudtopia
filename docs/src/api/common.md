# Results and Base Types

## `ST_StringView`

A borrowed byte string. It does not require a null terminator. `data` must be valid
when `size` is greater than zero. The receiving function documents whether it copies
or borrows the bytes.

## `ST_Registration`

An opaque nonzero handle returned by a successful registry operation. It remains
owned by the registering mod until explicit release or owner cleanup.

## `ST_Result`

| Value | Meaning |
|---|---|
| `ST_RESULT_OK` | Operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, value, ID, or descriptor is invalid. |
| `ST_RESULT_ALREADY_EXISTS` | A unique ID, provider, path, or conflicting resource already exists. |
| `ST_RESULT_NOT_FOUND` | The requested item or optional provider is absent. |
| `ST_RESULT_VERSION_MISMATCH` | ABI or service versions are incompatible. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_UNSUPPORTED` | The operation is known but unavailable for this host or game profile. |
| `ST_RESULT_CALLBACK_FAILED` | A mod callback returned an error or threw an exception. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete an internal operation. |

Mods propagate specific errors and must not convert every failure to
`ST_RESULT_INTERNAL_ERROR`.
