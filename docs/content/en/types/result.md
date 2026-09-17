# Result

Every fallible API operation returns `Result`. `RESULT_OK` indicates success; every other value indicates failure.

| Value | Meaning |
|---|---|
| `RESULT_OK` | Operation completed successfully. |
| `RESULT_INVALID_ARGUMENT` | A value, pointer, structure size, or document is invalid. |
| `RESULT_CONFLICT` | Requested ownership or mutation conflicts with current state. |
| `RESULT_NOT_FOUND` | Resource or provider does not exist. |
| `RESULT_VERSION_MISMATCH` | The requested version does not match API 1.1. |
| `RESULT_PERMISSION_DENIED` | Required permission is missing. |
| `RESULT_NOT_AVAILABLE` | Feature is unavailable in the current state. |
| `RESULT_CALLBACK_FAILED` | A consumer callback failed. |
| `RESULT_INTERNAL_ERROR` | The loader failed to complete a valid operation. |
