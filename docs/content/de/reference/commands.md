<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Commands

Registriert benannte Commands mit Beschreibungen und Callbacks.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Serviceversion:** 1.1
- **Verfügbar seit:** API 1.1
- **Threading:** Main thread; callbacks are synchronous
- **Capabilities:** `shroudtopia.registry.commands`

</div>

## Funktionen

| Funktion | Aufgabe | Status |
|---|---|---|
| [`CommandCallback`](#commandcallback) | Execute . | Stable |
| [`register_command`](#register_command) | Register command. | Stable |
| [`invoke_command`](#invoke_command) | Invoke command. | Stable |

## Typen

### `CommandDescriptor`

| Feld | Typ | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `command_id` | `StringView` | Value |
| `description` | `StringView` | Value |
| `callback` | `CommandCallback` | Value |
| `user_data` | `void*` | Borrowed or caller-owned; see operation |

<section class="api-function" data-api-name="commandcallback" data-api-status="stable">

## `CommandCallback`

Execute .

### Signatur

```c
Result (CALL* CommandCallback)(StringView arguments, void* user_data);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `arguments` | in | `StringView` | yes | – | borrowed | Value for arguments. |
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
Result result = CommandCallback(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="register_command" data-api-status="stable">

## `register_command`

Register command.

### Signatur

```c
Result (CALL* register_command)(StringView owner_id, const CommandDescriptor* descriptor, Registration* registration);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `descriptor` | in | `const CommandDescriptor*` | yes | no | borrowed | Value for descriptor. |
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
// Register command.
Result result = api->register_command(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="invoke_command" data-api-status="stable">

## `invoke_command`

Invoke command.

### Signatur

```c
Result (CALL* invoke_command)(StringView owner_id, StringView command_id, StringView arguments);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `command_id` | in | `StringView` | yes | – | borrowed | Value for command id. |
| `arguments` | in | `StringView` | yes | – | borrowed | Value for arguments. |

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
// Invoke command.
Result result = api->invoke_command(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

## Kanonische Quelle

`api/shroudtopia.h` is the single public ABI header.
