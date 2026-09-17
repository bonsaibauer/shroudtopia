<!-- Generated from docs/contracts/host-api-v1.yaml; do not edit by hand. -->
# Host API

Core registries, permissions, settings, and logging exposed to every native mod.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia loader
- **Capability:** None
- **Header:** `api/include/shroudtopia/api/host.h`

</div>

## Types

### `ST_StringView`

Borrowed byte string; no NUL terminator is required.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `data` | `const char*` | First byte of the string. | May be null only when size is zero. |
| `size` | `size_t` | String length in bytes. | — |

### `ST_Result`

Result returned by every fallible API operation.

| Values | C type / value | Meaning | Rules |
|---|---|---|---|
| `ST_RESULT_OK` | `0` | Operation completed. | — |
| `ST_RESULT_INVALID_ARGUMENT` | `1` | Invalid input. | — |
| `ST_RESULT_ALREADY_EXISTS` | `2` | Unique value or owned range conflicts. | — |
| `ST_RESULT_NOT_FOUND` | `3` | Requested item is absent. | — |
| `ST_RESULT_VERSION_MISMATCH` | `4` | Versions are incompatible. | — |
| `ST_RESULT_PERMISSION_DENIED` | `5` | Required permission is absent. | — |
| `ST_RESULT_UNSUPPORTED` | `6` | Known operation is unavailable. | — |
| `ST_RESULT_CALLBACK_FAILED` | `7` | A callback failed or threw. | — |
| `ST_RESULT_INTERNAL_ERROR` | `8` | Host failure. | — |

### `ST_Registration`

Opaque nonzero owner-bound registry handle.

### `ST_ServiceDescriptor`

Published service identity and interface.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `contract_id` | `ST_StringView` | Globally unique contract ID. | — |
| `version_major` | `uint32_t` | Breaking version. | — |
| `version_minor` | `uint32_t` | Backward-compatible version. | — |
| `interface_pointer` | `const void*` | Provider-owned interface table. | — |

### `ST_ServiceRequest`

Compatible service lookup request.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `contract_id` | `ST_StringView` | Requested contract ID. | — |
| `version_major` | `uint32_t` | Required major version. | — |
| `minimum_minor` | `uint32_t` | Minimum compatible minor version. | — |

### `ST_Event`

Published synchronous event.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `event_id` | `ST_StringView` | Global event ID. | — |
| `payload` | `const void*` | Opaque payload bytes. | Borrowed during dispatch. The publisher owns it. |
| `payload_size` | `size_t` | Payload length in bytes. | — |

### `ST_CommandDescriptor`

Global command registration.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `command_id` | `ST_StringView` | Global command ID. | — |
| `description` | `ST_StringView` | Human-readable help text. | — |
| `callback` | `ST_CommandCallback` | Execution callback. | — |
| `user_data` | `void*` | Opaque callback context. | — |

### `ST_SettingsDescriptor`

Settings schema and defaults.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `settings_id` | `ST_StringView` | Unique settings ID. | — |
| `schema_json` | `ST_StringView` | JSON Schema document. | — |
| `defaults_json` | `ST_StringView` | JSON defaults object. | — |

### `ST_CapabilityInfoV1`

Runtime feature availability.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `version_major` | `uint32_t` | Capability major version. | — |
| `version_minor` | `uint32_t` | Capability minor version. | — |
| `flags` | `uint64_t` | Capability-defined flags; zero if none. | — |
| `available` | `uint8_t` | One when usable, otherwise zero. | — |
| `reserved` | `uint8_t[7]` | Reserved and zero-initialized. | — |

### `ST_EventSubscription`

Event subscription callback and context.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure size. | — |
| `event_id` | `ST_StringView` | Subscribed event ID. | — |
| `callback` | `ST_EventCallback` | Synchronous callback. | — |
| `user_data` | `void*` | Opaque callback context. | — |

### `ST_HostApiV1`

Versioned host function table supplied to lifecycle callbacks.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Available table bytes. | — |
| `abi_version` | `uint32_t` | Host ABI version. | — |
| `register_service` | `function pointer` | register service operation. | — |
| `find_service` | `function pointer` | find service operation. | — |
| `subscribe_event` | `function pointer` | subscribe event operation. | — |
| `publish_event` | `function pointer` | publish event operation. | — |
| `register_command` | `function pointer` | register command operation. | — |
| `execute_command` | `function pointer` | execute command operation. | — |
| `register_settings` | `function pointer` | register settings operation. | — |
| `release_registration` | `function pointer` | release registration operation. | — |
| `release_owner` | `function pointer` | release owner operation. | — |
| `query_capability` | `function pointer` | query capability operation. | — |
| `check_permission` | `function pointer` | check permission operation. | — |
| `log` | `function pointer` | log operation. | — |
| `get_setting_bool` | `function pointer` | get setting bool operation. | — |
| `get_setting_number` | `function pointer` | get setting number operation. | — |

## Functions

<section class="api-function" data-api-name="register_service" data-api-status="stable">

### `register_service`

Publish a versioned service provider.

```c
ST_Result register_service(ST_StringView owner_id, const ST_ServiceDescriptor* descriptor, ST_Registration* registration);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** Adds an owner-bound service registration.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Stable mod owner ID. |
| `descriptor` | `const ST_ServiceDescriptor*` | in | yes | Initialized service description. |
| `registration` | `ST_Registration*` | out | yes | Receives the release handle. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_ALREADY_EXISTS` | The contract and major version already have a provider. |

#### Example

```cpp
// Initialize owner_id, descriptor, registration according to the parameter table.
const ST_Result status = api->register_service(owner_id, descriptor, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="find_service" data-api-status="stable">

### `find_service`

Find the highest compatible service provider.

```c
ST_Result find_service(const ST_ServiceRequest* request, const void** interface_pointer);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `request` | `const ST_ServiceRequest*` | in | yes | Requested contract and version. |
| `interface_pointer` | `const void**` | out | yes | Receives the provider-owned table. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_NOT_FOUND` | No compatible provider is registered. |
| `ST_RESULT_VERSION_MISMATCH` | The contract exists with an incompatible version. |

#### Example

```cpp
// Initialize request, interface_pointer according to the parameter table.
const ST_Result status = api->find_service(request, interface_pointer);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="subscribe_event" data-api-status="stable">

### `subscribe_event`

Subscribe a callback to an event ID.

```c
ST_Result subscribe_event(ST_StringView owner_id, const ST_EventSubscription* subscription, ST_Registration* registration);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Subscriber owner ID. |
| `subscription` | `const ST_EventSubscription*` | in | yes | Event ID, callback, and context. |
| `registration` | `ST_Registration*` | out | yes | Receives the subscription handle. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, subscription, registration according to the parameter table.
const ST_Result status = api->subscribe_event(owner_id, subscription, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="publish_event" data-api-status="stable">

### `publish_event`

Deliver an event synchronously to a subscriber snapshot.

```c
ST_Result publish_event(ST_StringView owner_id, const ST_Event* event_data);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronously invokes callbacks on the publishing thread.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** Invokes current subscriber callbacks.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Publisher owner ID. |
| `event_data` | `const ST_Event*` | in | yes | Event and borrowed payload. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_CALLBACK_FAILED` | A subscriber failed or threw. |

#### Example

```cpp
// Initialize owner_id, event_data according to the parameter table.
const ST_Result status = api->publish_event(owner_id, event_data);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="register_command" data-api-status="stable">

### `register_command`

Register a globally unique command.

```c
ST_Result register_command(ST_StringView owner_id, const ST_CommandDescriptor* descriptor, ST_Registration* registration);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Command owner. |
| `descriptor` | `const ST_CommandDescriptor*` | in | yes | Command callback and metadata. |
| `registration` | `ST_Registration*` | out | yes | Receives the registration. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_ALREADY_EXISTS` | The command ID is already registered. |

#### Example

```cpp
// Initialize owner_id, descriptor, registration according to the parameter table.
const ST_Result status = api->register_command(owner_id, descriptor, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="execute_command" data-api-status="stable">

### `execute_command`

Execute a registered command with raw arguments.

```c
ST_Result execute_command(ST_StringView owner_id, ST_StringView command_id, ST_StringView arguments);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `command_id` | `ST_StringView` | in | yes | Command to execute. |
| `arguments` | `ST_StringView` | in | yes | Borrowed raw argument text. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_NOT_FOUND` | The command is not registered. |
| `ST_RESULT_CALLBACK_FAILED` | The command callback failed. |

#### Example

```cpp
// Initialize owner_id, command_id, arguments according to the parameter table.
const ST_Result status = api->execute_command(owner_id, command_id, arguments);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="register_settings" data-api-status="stable">

### `register_settings`

Register settings schema and defaults.

```c
ST_Result register_settings(ST_StringView owner_id, const ST_SettingsDescriptor* descriptor, ST_Registration* registration);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Settings owner. |
| `descriptor` | `const ST_SettingsDescriptor*` | in | yes | Schema and defaults JSON. |
| `registration` | `ST_Registration*` | out | yes | Receives the registration. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, descriptor, registration according to the parameter table.
const ST_Result status = api->register_settings(owner_id, descriptor, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="release_registration" data-api-status="stable">

### `release_registration`

Release one registry resource.

```c
ST_Result release_registration(ST_Registration registration);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** Removes the represented registration.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `registration` | `ST_Registration` | in | yes | Valid owner-bound handle. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_NOT_FOUND` | The handle is unknown or already released. |

#### Example

```cpp
// Initialize registration according to the parameter table.
const ST_Result status = api->release_registration(registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="release_owner" data-api-status="stable">

### `release_owner`

Release all resources associated with an owner.

```c
ST_Result release_owner(ST_StringView owner_id);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** Removes registry resources and supported subsystem resources.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Owner to clean up. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id according to the parameter table.
const ST_Result status = api->release_owner(owner_id);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="query_capability" data-api-status="stable">

### `query_capability`

Query whether a known capability is available.

```c
ST_Result query_capability(ST_StringView capability_id, ST_CapabilityInfoV1* information);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `capability_id` | `ST_StringView` | in | yes | Capability identifier. |
| `information` | `ST_CapabilityInfoV1*` | inout | yes | Initialized output information. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_NOT_FOUND` | The capability ID is unknown. |

#### Example

```cpp
// Initialize capability_id, information according to the parameter table.
const ST_Result status = api->query_capability(capability_id, information);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="check_permission" data-api-status="stable">

### `check_permission`

Check a manifest-bound owner permission.

```c
ST_Result check_permission(ST_StringView owner_id, ST_StringView permission_id, uint8_t* allowed);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Owner to inspect. |
| `permission_id` | `ST_StringView` | in | yes | Permission/capability ID. |
| `allowed` | `uint8_t*` | out | yes | Receives one or zero. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, permission_id, allowed according to the parameter table.
const ST_Result status = api->check_permission(owner_id, permission_id, allowed);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="log" data-api-status="stable">

### `log`

Write an owner-tagged log message.

```c
ST_Result log(ST_StringView owner_id, ST_LogLevel level, ST_StringView message);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Message owner. |
| `level` | `ST_LogLevel` | in | yes | Log severity. |
| `message` | `ST_StringView` | in | yes | UTF-8 message; copied during the call. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, level, message according to the parameter table.
const ST_Result status = api->log(owner_id, level, message);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_setting_bool" data-api-status="stable">

### `get_setting_bool`

Read a Boolean setting or its fallback.

```c
ST_Result get_setting_bool(ST_StringView owner_id, ST_StringView key, uint8_t fallback, uint8_t* value);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Settings owner. |
| `key` | `ST_StringView` | in | yes | Setting key. |
| `fallback` | `uint8_t` | in | yes | Zero or one fallback. |
| `value` | `uint8_t*` | out | yes | Receives zero or one. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, key, fallback, value according to the parameter table.
const ST_Result status = api->get_setting_bool(owner_id, key, fallback, value);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_setting_number" data-api-status="stable">

### `get_setting_number`

Read a finite numeric setting or its fallback.

```c
ST_Result get_setting_number(ST_StringView owner_id, ST_StringView key, double fallback, double* value);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** None
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Settings owner. |
| `key` | `ST_StringView` | in | yes | Setting key. |
| `fallback` | `double` | in | yes | Finite fallback. |
| `value` | `double*` | out | yes | Receives a finite number. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, key, fallback, value according to the parameter table.
const ST_Result status = api->get_setting_number(owner_id, key, fallback, value);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
