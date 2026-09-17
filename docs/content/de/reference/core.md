<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Core API

Definiert ABI-Version, gemeinsame String- und Registrierungs-Handles sowie das Result-Fehlermodell.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Serviceversion:** 1.1
- **Verfügbar seit:** API 1.1
- **Threading:** See each operation
- **Capabilities:** None

</div>

## Funktionen

| Funktion | Aufgabe | Status |
|---|---|---|

Dieser Vertrag definiert gemeinsame Typen und besitzt keine direkt aufrufbaren Funktionen.

## Typen

### `Result`

| Wert | Numerischer Wert |
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

| Feld | Typ | Ownership |
|---|---|---|
| `data` | `const char*` | Borrowed or caller-owned; see operation |
| `size` | `size_t` | Value |

## Kanonische Quelle

`api/shroudtopia.h` is the single public ABI header.
