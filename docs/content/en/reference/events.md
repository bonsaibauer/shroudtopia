<!-- Generated from public headers and docs/api metadata. Do not edit by hand. -->
# Events

Publishes typed event payloads and manages event subscriptions.

<div class="api-meta">

- **Status:** Stable
- **Header:** `shroudtopia.h`
- **Service version:** 1.1
- **Available since:** API 1.1
- **Threading:** Publisher thread; callbacks are synchronous
- **Capabilities:** `shroudtopia.registry.events`

</div>

## Functions

| Function | Purpose | Status |
|---|---|---|
| [`EventCallback`](#eventcallback) | Execute . | Stable |
| [`subscribe_event`](#subscribe_event) | Subscribe to event. | Stable |
| [`publish_event`](#publish_event) | Publish event. | Stable |

## Types

### `Event`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `event_id` | `StringView` | Value |
| `payload` | `const void*` | Borrowed or caller-owned; see operation |
| `payload_size` | `size_t` | Value |

### `EventSubscription`

| Field | Type | Ownership |
|---|---|---|
| `struct_size` | `size_t` | Value |
| `event_id` | `StringView` | Value |
| `callback` | `EventCallback` | Value |
| `user_data` | `void*` | Borrowed or caller-owned; see operation |

<section class="api-function" data-api-name="eventcallback" data-api-status="stable">

## `EventCallback`

Execute .

### Signature

```c
Result (CALL* EventCallback)(const Event* event_data, void* user_data);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `event_data` | in | `const Event*` | yes | no | borrowed | Value for event data. |
| `user_data` | in | `void*` | yes | no | borrowed | Opaque context forwarded unchanged to the callback. |

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
// Execute .
Result result = EventCallback(/* callback parameters */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="subscribe_event" data-api-status="stable">

## `subscribe_event`

Subscribe to event.

### Signature

```c
Result (CALL* subscribe_event)(StringView owner_id, const EventSubscription* subscription, Registration* registration);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `subscription` | in | `const EventSubscription*` | yes | no | borrowed | Value for subscription. |
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
// Subscribe to event.
Result result = api->subscribe_event(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

<section class="api-function" data-api-name="publish_event" data-api-status="stable">

## `publish_event`

Publish event.

### Signature

```c
Result (CALL* publish_event)(StringView owner_id, const Event* event_data);
```

### Parameters

| Parameter | Direction | Type | Required | Nullable | Ownership | Description |
|---|---|---|---|---|---|---|
| `owner_id` | in | `StringView` | yes | – | borrowed | Stable identifier of the calling mod. |
| `event_data` | in | `const Event*` | yes | no | borrowed | Value for event data. |

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
// Publish event.
Result result = api->publish_event(/* initialize every parameter above */);
if (result != RESULT_OK) {
    // Log context and stop or degrade gracefully.
}
```

**Related contract:** `api/shroudtopia.h`

</section>

## Canonical source

`api/shroudtopia.h` is the single public ABI header.
