<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Mod-Lebenszyklus

Deklariert eine Mod und ihre Callbacks für Laden, Aktivierung, Update, Deaktivierung und Entladen.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Serviceversion:** 1.1
- **Verfügbar seit:** API 1.1
- **Threading:** Loader-controlled lifecycle thread
- **Capabilities:** `shroudtopia.lifecycle.native`

</div>

## Funktionen

| Funktion | Aufgabe | Status |
|---|---|---|
| [`ModLifecycleCallback`](#modlifecyclecallback) | Execute . | Stable |
| [`ModUpdateCallback`](#modupdatecallback) | Execute . | Stable |
| [`CreateModFunction`](#createmodfunction) | Execute . | Stable |

## Typen

### `ModDescriptor`

| Feld | Typ | Ownership |
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

### Signatur

```c
Result (CALL* ModLifecycleCallback)(const Api* api, void* user_data);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `api` | in | `const Api*` | yes | no | borrowed | Value for api. |
| `user_data` | in | `void*` | yes | no | borrowed | Opaque context forwarded unchanged to the callback. |

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
// Execute .
Result result = ModLifecycleCallback(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="modupdatecallback" data-api-status="stable">

## `ModUpdateCallback`

Execute .

### Signatur

```c
Result (CALL* ModUpdateCallback)(const Api* api, void* user_data, double delta_seconds);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `api` | in | `const Api*` | yes | no | borrowed | Value for api. |
| `user_data` | in | `void*` | yes | no | borrowed | Opaque context forwarded unchanged to the callback. |
| `delta_seconds` | in | `double` | yes | – | value | Value for delta seconds. |

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
// Execute .
Result result = ModUpdateCallback(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="createmodfunction" data-api-status="stable">

## `CreateModFunction`

Execute .

### Signatur

```c
Result (CALL* CreateModFunction)(uint32_t requested_api_version, ModDescriptor* descriptor);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `requested_api_version` | in | `uint32_t` | yes | – | value | Value for requested api version. |
| `descriptor` | in | `ModDescriptor*` | yes | no | borrowed | Value for descriptor. |

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
// Execute .
Result result = CreateModFunction(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

## Kanonische Quelle

`api/shroudtopia.h` is the single public ABI header.
