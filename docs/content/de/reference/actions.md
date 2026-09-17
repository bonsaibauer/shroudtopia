<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Actions

Stellt auffindbare Operationen mit JSON-Schema-Eingaben und beobachtbarem Status bereit.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Serviceversion:** 1.1
- **Verfügbar seit:** API 1.1
- **Threading:** Main thread
- **Capabilities:** `shroudtopia.registry.services`

</div>

## Funktionen

| Funktion | Aufgabe | Status |
|---|---|---|
| [`register_action`](#register_action) | Register action. | Stable |
| [`invoke_action`](#invoke_action) | Invoke action. | Stable |
| [`get_action_state`](#get_action_state) | Read action state. | Stable |

## Typen

### `Action`

| Feld | Typ | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `id` | `StringView` | Value |
| `title` | `StringView` | Value |
| `description` | `StringView` | Value |
| `input_schema_json` | `StringView` | Value |
| `invoke` | `ActionHandler` | Value |
| `user_data` | `void*` | Borrowed or caller-owned; see operation |

<section class="api-function" data-api-name="register_action" data-api-status="stable">

## `register_action`

Register action.

### Signatur

```c
Result (CALL* register_action)(StringView owner_id, const Action* action, Registration* registration);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `action` | in | `const Action*` | yes | no | borrowed | Value for action. |
| `registration` | out | `Registration*` | yes | no | caller-owned | Receives or identifies an ownership-bound registration. |

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
// Register action.
Result result = api->register_action(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="invoke_action" data-api-status="stable">

## `invoke_action`

Invoke action.

### Signatur

```c
Result (CALL* invoke_action)(StringView owner_id, StringView action_id, StringView input_json);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `action_id` | in | `StringView` | yes | – | borrowed | Value for action id. |
| `input_json` | in | `StringView` | yes | – | borrowed | UTF-8 JSON input validated against the action schema. |

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
// Invoke action.
Result result = api->invoke_action(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="get_action_state" data-api-status="stable">

## `get_action_state`

Read action state.

### Signatur

```c
Result (CALL* get_action_state)(StringView action_id, ActionState* state);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `action_id` | in | `StringView` | yes | – | borrowed | Value for action id. |
| `state` | out | `ActionState*` | yes | no | caller-owned | Value for state. |

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
// Read action state.
Result result = api->get_action_state(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

## Kanonische Quelle

`api/shroudtopia.h` is the single public ABI header.
