<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Runtime Patches

Erstellt und steuert besitzergebundene direkte oder Detour-Runtime-Patches.

<div class="api-meta">

- **Status:** Experimental
- **Header:** `shroudtopia.h`
- **Serviceversion:** 1.1
- **Verfügbar seit:** API 1.1
- **Threading:** Main thread
- **Capabilities:** `shroudtopia.runtime.patches`

</div>

## Funktionen

| Funktion | Aufgabe | Status |
|---|---|---|
| [`create_patch`](#create_patch) | Create patch. | Experimental |
| [`set_patch_enabled`](#set_patch_enabled) | Set patch enabled. | Experimental |
| [`get_patch_state`](#get_patch_state) | Read patch state. | Experimental |
| [`release_patch`](#release_patch) | Release patch. | Experimental |

## Typen

### `RuntimePatchKind`

| Wert | Numerischer Wert |
|---|---:|
| `RUNTIME_PATCH_DIRECT` | `0` |
| `RUNTIME_PATCH_DETOUR` | `1` |

### `RuntimeRelocationKind`

| Wert | Numerischer Wert |
|---|---:|
| `RUNTIME_RELOCATION_REL32_RETURN` | `1` |

### `RuntimeRelocation`

| Feld | Typ | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `payload_offset` | `size_t` | Value |
| `kind` | `RuntimeRelocationKind` | Value |

### `RuntimePatchOptions`

| Feld | Typ | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `signature` | `StringView` | Value |
| `match_offset` | `int64_t` | Value |
| `kind` | `RuntimePatchKind` | Value |
| `overwrite_size` | `size_t` | Value |
| `payload` | `const uint8_t*` | Borrowed or caller-owned; see operation |
| `payload_size` | `size_t` | Value |
| `relocations` | `const RuntimeRelocation*` | Borrowed or caller-owned; see operation |
| `relocation_count` | `size_t` | Value |

### `RuntimePatchState`

| Feld | Typ | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `enabled` | `uint8_t` | Value |
| `reserved` | `uint8_t` | Value |

<section class="api-function" data-api-name="create_patch" data-api-status="experimental">

## `create_patch`

Create patch.

### Signatur

```c
Result (CALL* create_patch)(StringView owner_id, const RuntimePatchOptions* options, RuntimePatch* patch);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `options` | in | `const RuntimePatchOptions*` | yes | no | borrowed | Value for options. |
| `patch` | out | `RuntimePatch*` | yes | no | caller-owned | Value for patch. |

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
// Create patch.
Result result = api->create_patch(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="set_patch_enabled" data-api-status="experimental">

## `set_patch_enabled`

Set patch enabled.

### Signatur

```c
Result (CALL* set_patch_enabled)(StringView owner_id, RuntimePatch patch, uint8_t enabled);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `patch` | in | `RuntimePatch` | yes | – | value | Value for patch. |
| `enabled` | in | `uint8_t` | yes | – | value | Value for enabled. |

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
// Set patch enabled.
Result result = api->set_patch_enabled(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="get_patch_state" data-api-status="experimental">

## `get_patch_state`

Read patch state.

### Signatur

```c
Result (CALL* get_patch_state)(StringView owner_id, RuntimePatch patch, RuntimePatchState* state);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `patch` | in | `RuntimePatch` | yes | – | value | Value for patch. |
| `state` | out | `RuntimePatchState*` | yes | no | caller-owned | Value for state. |

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
// Read patch state.
Result result = api->get_patch_state(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="release_patch" data-api-status="experimental">

## `release_patch`

Release patch.

### Signatur

```c
Result (CALL* release_patch)(StringView owner_id, RuntimePatch patch);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `patch` | in | `RuntimePatch` | yes | – | value | Value for patch. |

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
// Release patch.
Result result = api->release_patch(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

## Kanonische Quelle

`api/shroudtopia.h` is the single public ABI header.
