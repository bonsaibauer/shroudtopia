<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Settings

Liest typisierte Werte aus der Konfiguration der aufrufenden Mod.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Serviceversion:** 1.1
- **Verfügbar seit:** API 1.1
- **Threading:** Main thread
- **Capabilities:** None

</div>

## Funktionen

| Funktion | Aufgabe | Status |
|---|---|---|
| [`get_mod_setting_bool`](#get_mod_setting_bool) | Read mod setting bool. | Stable |
| [`get_mod_setting_number`](#get_mod_setting_number) | Read mod setting number. | Stable |

<section class="api-function" data-api-name="get_mod_setting_bool" data-api-status="stable">

## `get_mod_setting_bool`

Read mod setting bool.

### Signatur

```c
Result (CALL* get_mod_setting_bool)(StringView owner_id, StringView key, uint8_t fallback, uint8_t* value);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `key` | in | `StringView` | yes | – | borrowed | Value for key. |
| `fallback` | in | `uint8_t` | yes | – | value | Value for fallback. |
| `value` | out | `uint8_t*` | yes | no | caller-owned | Value for value. |

### Ergebnisse

| Result | Bedeutung | Empfohlene Behandlung |
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

### Beispiel

> Strukturen immer initialisieren, Result prüfen und besitzergebundene Handles freigeben.

```cpp
// Read mod setting bool.
Result result = api->get_mod_setting_bool(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="get_mod_setting_number" data-api-status="stable">

## `get_mod_setting_number`

Read mod setting number.

### Signatur

```c
Result (CALL* get_mod_setting_number)(StringView owner_id, StringView key, double fallback, double* value);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `key` | in | `StringView` | yes | – | borrowed | Value for key. |
| `fallback` | in | `double` | yes | – | value | Value for fallback. |
| `value` | out | `double*` | yes | no | caller-owned | Value for value. |

### Ergebnisse

| Result | Bedeutung | Empfohlene Behandlung |
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

### Beispiel

> Strukturen immer initialisieren, Result prüfen und besitzergebundene Handles freigeben.

```cpp
// Read mod setting number.
Result result = api->get_mod_setting_number(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

## Kanonische Quelle

`api/shroudtopia.h` is the single public ABI header.
