# `ST_HostApiV1`

## Status

| Status | Contract |
|:---:|---|
| ✅ | `ST_ABI_VERSION_1` |
| ✅ | All table functions listed below |

The table begins with `struct_size` and `abi_version`. The loader owns the table for
the lifetime of the loaded mod.

## Functions

| Function | Purpose |
|---|---|
| `register_service` | Publish a provider for one contract and major version. |
| `find_service` | Find the highest compatible minor provider. |
| `subscribe_event` | Register a synchronous event callback. |
| `publish_event` | Deliver a borrowed payload to current subscribers. |
| `register_command` | Register a globally unique command ID. |
| `execute_command` | Invoke a command with borrowed raw arguments. |
| `register_settings` | Publish a JSON schema and defaults. |
| `release_registration` | Remove one owner-bound registration. |
| `release_owner` | Remove every registry resource owned by an ID. |
| `query_capability` | Return version and availability for a known capability. |
| `check_permission` | Check whether an owner requested a protected operation. |
| `log` | Write one owner-tagged message. |
| `get_setting_bool` | Read a Boolean mod setting with a fallback. |
| `get_setting_number` | Read a finite numeric mod setting with a fallback. |

## Optional table tail

`get_setting_number` is an appended field. Check the table boundary before reading it:

```cpp
const bool has_number = api->struct_size >=
    offsetof(ST_HostApiV1, get_setting_number) + sizeof(api->get_setting_number)
    && api->get_setting_number;
```

See the domain pages for parameter ownership, errors, and examples.
