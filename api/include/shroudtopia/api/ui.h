#pragma once
#include "shroudtopia/api/base.h"
#ifdef __cplusplus
extern "C" {
#endif
#define ST_UI_TEXT_SERVICE_ID "shroudtopia.ui.text"
typedef uint64_t ST_TextWindow;
/* Native Windows overlay for windowed/borderless clients, hidden initially.
   No exclusive-fullscreen rendering. UTF-8 strings copied, 1..8 tabs.
   Title/labels <= 128 bytes. Hotkey is a Windows virtual key (0 disables).
   Creation asynchronous; destroy joins the UI thread. No mod callbacks. */
typedef struct ST_TextWindowDescriptorV1 {
    size_t struct_size;
    ST_StringView title;
    const ST_StringView* tabs;
    size_t tab_count;
    uint32_t toggle_key;
} ST_TextWindowDescriptorV1;
typedef enum ST_TextWindowStatusV1 { ST_TEXT_PENDING = 0, ST_TEXT_READY = 1, ST_TEXT_FAILED = 2 } ST_TextWindowStatusV1;
typedef struct ST_UiTextApiV1 {
    size_t struct_size;
    uint32_t abi_version;
    ST_Result (ST_CALL* create)(ST_StringView owner, const ST_TextWindowDescriptorV1*, ST_TextWindow*);
    /* Replace one tab's snapshot, <=1 MiB UTF-8; empty clears it.
       Thread safe, coalesced; not an event queue. */
    ST_Result (ST_CALL* set_text)(ST_StringView owner, ST_TextWindow, size_t tab, ST_StringView text);
    ST_Result (ST_CALL* get_status)(ST_StringView owner, ST_TextWindow, ST_TextWindowStatusV1*);
    ST_Result (ST_CALL* destroy)(ST_StringView owner, ST_TextWindow);
} ST_UiTextApiV1;
#ifdef __cplusplus
}
#endif
