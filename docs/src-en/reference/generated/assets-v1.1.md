<!-- Generated from docs/contracts/assets-v1.1.yaml; do not edit by hand. -->
# Asset API

Typed KFC3 resource discovery, JSON editing, ownership, and durable publication.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.1`
- **Provider:** Shroudtopia asset engine
- **Capability:** `shroudtopia.assets.read / shroudtopia.assets.write`
- **Header:** `api/include/shroudtopia/api/assets.h`

</div>

## Types

### `ST_AssetResourceKeyV1`

Typed resource identity.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | Initialized structure byte size. | Set to sizeof(the structure). |
| `guid` | `ST_StringView` | Resource GUID. | — |
| `type_name` | `ST_StringView` | Qualified KFC3 type. | — |
| `part` | `uint32_t` | Resource part index. | — |

### `ST_AssetsApiV1`

Versioned service function table.

| Field | C type / value | Meaning | Rules |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `visit_resources` | `function pointer` | visit resources. | — |
| `read_resource_json` | `function pointer` | read resource json. | — |
| `replace_resource_json` | `function pointer` | replace resource json. | — |
| `create_resource_json` | `function pointer` | create resource json. | — |
| `discard_changes` | `function pointer` | discard changes. | — |
| `flush` | `function pointer` | flush. | — |
| `set_resource_field_json` | `function pointer` | set resource field json. | — |

## Functions

<section class="api-function" data-api-name="visit_resources" data-api-status="stable">

### `visit_resources`

Visit a snapshot of resources of one qualified type.

```c
ST_Result visit_resources(ST_StringView owner_id, ST_StringView type_name, ST_AssetResourceVisitorV1 visitor, void* user_data);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.assets.read`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `type_name` | `ST_StringView` | in | yes | Qualified resource type. |
| `visitor` | `ST_AssetResourceVisitorV1` | in | yes | Visitor callback. |
| `user_data` | `void*` | in | yes | Opaque visitor context. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, type_name, visitor, user_data according to the parameter table.
const ST_Result status = api->visit_resources(owner_id, type_name, visitor, user_data);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="read_resource_json" data-api-status="stable">

### `read_resource_json`

Read one typed resource as JSON.

```c
ST_Result read_resource_json(ST_StringView owner_id, const ST_AssetResourceKeyV1* resource, char* buffer, size_t capacity, size_t* required_size);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.assets.read`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `resource` | `const ST_AssetResourceKeyV1*` | in | yes | Resource key. |
| `buffer` | `char*` | out | no | Caller-owned output buffer. |
| `capacity` | `size_t` | in | yes | Available buffer bytes. |
| `required_size` | `size_t*` | out | yes | Receives exact bytes, without terminator. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, resource, buffer, capacity, required_size according to the parameter table.
const ST_Result status = api->read_resource_json(owner_id, resource, buffer, capacity, required_size);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="replace_resource_json" data-api-status="stable">

### `replace_resource_json`

Replace a complete resource document.

```c
ST_Result replace_resource_json(ST_StringView owner_id, const ST_AssetResourceKeyV1* resource, ST_StringView json);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.assets.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `resource` | `const ST_AssetResourceKeyV1*` | in | yes | Existing resource key. |
| `json` | `ST_StringView` | in | yes | Typed JSON document. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, resource, json according to the parameter table.
const ST_Result status = api->replace_resource_json(owner_id, resource, json);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="create_resource_json" data-api-status="stable">

### `create_resource_json`

Create one typed resource and report its generated key.

```c
ST_Result create_resource_json(ST_StringView owner_id, ST_StringView type_name, ST_StringView json, ST_AssetResourceVisitorV1 visitor, void* user_data);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.assets.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `type_name` | `ST_StringView` | in | yes | Qualified resource type. |
| `json` | `ST_StringView` | in | yes | Typed JSON document. |
| `visitor` | `ST_AssetResourceVisitorV1` | in | yes | Receives the created key. |
| `user_data` | `void*` | in | yes | Opaque callback context. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |

#### Example

```cpp
// Initialize owner_id, type_name, json, visitor, user_data according to the parameter table.
const ST_Result status = api->create_resource_json(owner_id, type_name, json, visitor, user_data);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="discard_changes" data-api-status="stable">

### `discard_changes`

Discard all asset changes owned by one mod.

```c
ST_Result discard_changes(ST_StringView owner_id);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.assets.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |

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
const ST_Result status = api->discard_changes(owner_id);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="flush" data-api-status="stable">

### `flush`

Publish the combined asset overlay with recovery.

```c
ST_Result flush(ST_StringView owner_id);
```

- **Status:** ✅ stable
- **Since:** `1.0`
- **Permission:** `shroudtopia.assets.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |

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
const ST_Result status = api->flush(owner_id);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_resource_field_json" data-api-status="stable">

### `set_resource_field_json`

Set one existing JSON Pointer field.

```c
ST_Result set_resource_field_json(ST_StringView owner_id, const ST_AssetResourceKeyV1* resource, ST_StringView path, ST_StringView json);
```

- **Status:** ✅ stable
- **Since:** `1.1`
- **Permission:** `shroudtopia.assets.write`
- **Threading:** Synchronous on the calling thread; do not call concurrently unless stated otherwise.
- **Ownership:** Input is borrowed for the duration of the call; output ownership is stated per parameter.
- **Side effects:** No side effect beyond the described operation.

#### Parameters

| Field | C type / value | Direction | Required | Meaning |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | yes | Calling owner. |
| `resource` | `const ST_AssetResourceKeyV1*` | in | yes | Existing resource. |
| `path` | `ST_StringView` | in | yes | Existing JSON Pointer. |
| `json` | `ST_StringView` | in | yes | One typed JSON value. |

#### Results

| Result | Meaning |
|---|---|
| `ST_RESULT_OK` | The operation completed. |
| `ST_RESULT_INVALID_ARGUMENT` | A pointer, size, identifier, descriptor, or value is invalid. |
| `ST_RESULT_PERMISSION_DENIED` | The owner lacks the required permission. |
| `ST_RESULT_INTERNAL_ERROR` | The host could not complete the operation. |
| `ST_RESULT_ALREADY_EXISTS` | Another owner controls an overlapping field. |

#### Example

```cpp
// Initialize owner_id, resource, path, json according to the parameter table.
const ST_Result status = api->set_resource_field_json(owner_id, resource, path, json);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
