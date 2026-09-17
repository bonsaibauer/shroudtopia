<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Core API

Defines the ABI version, common string and registration handles, and the Result error model.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** See each operation
- **Capabilities:** None

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|

This contract defines shared types and has no directly callable functions.

## Types

### `Result`

| Value | Numeric value |
|---|---:|
| `RESULT_OK` | `0` |
| `RESULT_INVALID_ARGUMENT` | `1` |
| `RESULT_CONFLICT` | `2` |
| `RESULT_NOT_FOUND` | `3` |
| `RESULT_VERSION_MISMATCH` | `4` |
| `RESULT_PERMISSION_DENIED` | `5` |
| `RESULT_NOT_AVAILABLE` | `6` |
| `RESULT_CALLBACK_FAILED` | `7` |
| `RESULT_INTERNAL_ERROR` | `8` |

### `StringView`

| Field | Type | Ownership |
|---|---|---|
| `data` | `const char*` | Borrowed or caller-owned; see operation |
| `size` | `size_t` | Value |

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
