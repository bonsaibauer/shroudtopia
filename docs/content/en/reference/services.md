<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Service discovery

Publishes extension contracts and resolves providers by exact contract version.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** Main thread
- **Capabilities:** `shroudtopia.registry.services`

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|
| [`register_service`](#register_service) | Publish a versioned service implementation owned by the calling mod. | Stable |
| [`find_service`](#find_service) | Resolve a service implementation by its exact contract version. | Stable |
| [`release_registration`](#release_registration) | Release one registration handle. | Stable |

## Types

### `ServiceDescriptor`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `contract_id` | `StringView` | Value |
| `version_major` | `uint32_t` | Value |
| `version_minor` | `uint32_t` | Value |
| `interface_pointer` | `const void*` | Borrowed or caller-owned; see operation |

### `ServiceRequest`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `contract_id` | `StringView` | Value |
| `version_major` | `uint32_t` | Value |
| `version_minor` | `uint32_t` | Value |

<section class="api-function" data-api-name="register_service" data-api-status="stable">

## `register_service`

Publish a versioned service implementation owned by the calling mod.

### Signature

```c
Result (CALL* register_service)(StringView owner_id, const ServiceDescriptor* descriptor, Registration* registration);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `descriptor` | in | `const ServiceDescriptor*` | yes | no | borrowed | Value for descriptor. |
| `registration` | out | `Registration*` | yes | no | caller-owned | Receives or identifies an ownership-bound registration. |

### Results

| Result | Meaning | Recommended handling |
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

### Example

> Always initialize structures, check Result, and release ownership-bound handles.

```cpp
// Publish a versioned service implementation owned by the calling mod.
Result result = api->register_service(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="find_service" data-api-status="stable">

## `find_service`

Resolve a service implementation by its exact contract version.

### Signature

```c
Result (CALL* find_service)(const ServiceRequest* request, const void** interface_pointer);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `request` | in | `const ServiceRequest*` | yes | no | borrowed | Value for request. |
| `interface_pointer` | out | `const void**` | yes | no | caller-owned | Value for interface pointer. |

### Results

| Result | Meaning | Recommended handling |
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

### Example

> Always initialize structures, check Result, and release ownership-bound handles.

```cpp
// Resolve a service implementation by its exact contract version.
Result result = api->find_service(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="release_registration" data-api-status="stable">

## `release_registration`

Release one registration handle.

### Signature

```c
Result (CALL* release_registration)(Registration registration);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `registration` | in | `Registration` | yes | – | value | Receives or identifies an ownership-bound registration. |

### Results

| Result | Meaning | Recommended handling |
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

### Example

> Always initialize structures, check Result, and release ownership-bound handles.

```cpp
// Release one registration handle.
Result result = api->release_registration(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
