# Logging API

Every mod can write through `ST_HostApiV1::log`. Every mod can request
`shroudtopia.logging.read@1.0` and read the current Shroudtopia or Enshrouded log.

## Status

| Status | Feature |
|:---:|---|
| ✅ | Owner-tagged writes |
| ✅ | TRACE, DEBUG, INFO, WARNING, and ERROR |
| ✅ | Read current Shroudtopia log |
| ✅ | Read current Enshrouded log |
| ✅ | Per-process session rotation |
| ✅ | Build a native live viewer through `shroudtopia.ui.text` |
| 🚧 | Enumerate archived sessions |
| ❌ | Subscribe to log-line events |

## Data flow

```text
Mod A --host->log--\
Mod B --host->log----> shroudtopia.log <--- ST_LogReadApiV1 --- any mod
Loader --------------/                             │
enshrouded.exe ----------> enshrouded.log <--------┘
```

## Write a message

```cpp
const ST_Result result = host->log(
    View("mod.author.weather"),
    ST_LOG_INFO,
    View("Weather profile loaded"));
```

Shroudtopia copies owner and message during the call. Strings do not require null
termination. The writer serializes concurrent calls. Keep messages below 2 KiB. The
current formatter truncates longer lines. All generated levels are written when
`enableLogging` is true. UI level selection changes the view only.

```text
[2026-09-17T00:14:02Z][shroudtopia][INFO] [mod.author.weather] Weather profile loaded
```

| Level | Intended use |
|---|---|
| `ST_LOG_TRACE` | High-frequency data for one focused investigation |
| `ST_LOG_DEBUG` | IDs, decisions, state, and technical steps |
| `ST_LOG_INFO` | Successful user-relevant state change |
| `ST_LOG_WARNING` | Restricted or unsupported operation |
| `ST_LOG_ERROR` | Requested operation failed |

## Find the reader

```cpp
ST_ServiceRequest request{sizeof(request),
    View(ST_LOG_READ_SERVICE_ID), 1, 0};
const void* value = nullptr;
if (host->find_service(&request, &value) != ST_RESULT_OK) return;
const auto* logs = static_cast<const ST_LogReadApiV1*>(value);
if (!logs || logs->struct_size < sizeof(*logs) ||
    logs->abi_version != ST_ABI_VERSION_1) return;
```

## Read a snapshot

```cpp
std::vector<char> buffer(1024 * 1024);
size_t written = 0;
const ST_Result result = logs->read_tail(
    ST_LOG_SOURCE_LOADER, buffer.data(), buffer.size(), &written);
if (result == ST_RESULT_OK) {
    std::string_view text(buffer.data(), written); // no null terminator
}
```

| Source | File | Content |
|---|---|---|
| `ST_LOG_SOURCE_LOADER` | `shroudtopia.log` | Loader and all API-writing mods |
| `ST_LOG_SOURCE_GAME` | `enshrouded.log` | Game-generated messages |

`read_tail` accepts 1 to 1,048,576 bytes of caller-owned storage. A small file is
returned completely. A larger file returns its tail and discards a partial first
line. The output has no null terminator. A missing file returns
`ST_RESULT_NOT_FOUND` with `written = 0`.

## Sessions

`shroudtopia.log` contains the current process instance. At the next start, a
non-empty file moves to:

```text
shroudtopia_logs/shroudtopia-YYYYMMDD-HHMMSS-client.log
shroudtopia_logs/shroudtopia-YYYYMMDD-HHMMSS-server.log
```

The timestamp is UTC. The read service exposes current logs only. Archive filenames
are storage details until archive enumeration becomes a public API.
