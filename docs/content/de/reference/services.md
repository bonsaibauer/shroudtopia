<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Service Discovery

Veröffentlicht Erweiterungsverträge und ermittelt Provider anhand der exakten Contract-Version.

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
| [`register_service`](#register_service) | Publish a versioned service implementation owned by the calling mod. | Stable |
| [`find_service`](#find_service) | Resolve a service implementation by its exact contract version. | Stable |
| [`release_registration`](#release_registration) | Release one registration handle. | Stable |

## Typen

### `ServiceDescriptor`

| Feld | Typ | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `contract_id` | `StringView` | Value |
| `version_major` | `uint32_t` | Value |
| `version_minor` | `uint32_t` | Value |
| `interface_pointer` | `const void*` | Borrowed or caller-owned; see operation |

### `ServiceRequest`

| Feld | Typ | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `contract_id` | `StringView` | Value |
| `version_major` | `uint32_t` | Value |
| `version_minor` | `uint32_t` | Value |

<section class="api-function" data-api-name="register_service" data-api-status="stable">

## `register_service`

Publish a versioned service implementation owned by the calling mod.

### Signatur

```c
Result (CALL* register_service)(StringView owner_id, const ServiceDescriptor* descriptor, Registration* registration);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `descriptor` | in | `const ServiceDescriptor*` | yes | no | borrowed | Value for descriptor. |
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
// Publish a versioned service implementation owned by the calling mod.
Result result = api->register_service(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="find_service" data-api-status="stable">

## `find_service`

Resolve a service implementation by its exact contract version.

### Signatur

```c
Result (CALL* find_service)(const ServiceRequest* request, const void** interface_pointer);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `request` | in | `const ServiceRequest*` | yes | no | borrowed | Value for request. |
| `interface_pointer` | out | `const void**` | yes | no | caller-owned | Value for interface pointer. |

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
// Resolve a service implementation by its exact contract version.
Result result = api->find_service(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="release_registration" data-api-status="stable">

## `release_registration`

Release one registration handle.

### Signatur

```c
Result (CALL* release_registration)(Registration registration);
```

### Parameter

| Parameter | Richtung | Typ | Pflicht | Null erlaubt | Ownership | Beschreibung |
|---|---|---|---|---|---|---|
| `registration` | in | `Registration` | yes | – | value | Receives or identifies an ownership-bound registration. |

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
// Release one registration handle.
Result result = api->release_registration(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Zugehöriger Vertrag:** `api/shroudtopia.h`

</section>

## Kanonische Quelle

`api/shroudtopia.h` is the single public ABI header.
