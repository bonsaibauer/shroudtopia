# Native Text UI API

`shroudtopia.ui.text@1.0` gives mods a native Windows text window. It uses no browser
runtime, network content, or external UI assets.

## Status

| Status | Feature |
|:---:|---|
| ✅ | Async native window |
| ✅ | Tabs, search, level view, copy, pause, and auto-scroll |
| ✅ | Owner-bound handles and cleanup |
| ✅ | Windowed and borderless client |
| ❌ | Exclusive-fullscreen renderer integration |
| ❌ | World overlay and ghosts |

## Find and create

```cpp
ST_ServiceRequest request{sizeof(request), View(ST_UI_TEXT_SERVICE_ID), 1, 0};
const void* value = nullptr;
if (host->find_service(&request, &value) != ST_RESULT_OK) return;
const auto* ui = static_cast<const ST_UiTextApiV1*>(value);

const ST_StringView tabs[]{View("Overview"), View("Details")};
ST_TextWindowDescriptorV1 descriptor{
    sizeof(descriptor), View("My Mod"), tabs, 2, VK_F10};
ST_TextWindow window = 0;
ui->create(owner, &descriptor, &window);
```

| Limit | Value |
|---|---:|
| Tabs | 1 to 8 |
| Title or tab label | 128 UTF-8 bytes |
| One tab snapshot | 1 MiB UTF-8 |
| Toggle key | Windows virtual key 1 to 255. Zero disables it |

`create` is asynchronous. Poll `get_status` for `PENDING`, `READY`, or `FAILED`.
`set_text` copies and replaces one complete tab snapshot. `destroy` joins the UI
thread. A handle can be used only by its owner.

| Result | Typical cause |
|---|---|
| `ST_RESULT_INVALID_ARGUMENT` | Invalid descriptor, owner, tab, key, or text size |
| `ST_RESULT_NOT_FOUND` | Unknown or destroyed handle |
| `ST_RESULT_PERMISSION_DENIED` | Handle belongs to another owner |
| `ST_RESULT_INTERNAL_ERROR` | Native window or thread creation failed |
