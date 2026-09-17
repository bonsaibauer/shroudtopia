#pragma once
#include "shroudtopia/api/result.h"
#ifdef __cplusplus
extern "C" {
#endif
#define UI_TEXT_SERVICE_ID "shroudtopia.ui.text"
#define UI_TEXT_SERVICE_VERSION_MAJOR 2u
#define UI_TEXT_SERVICE_VERSION_MINOR 0u
typedef uint64_t TextWindow;
/* Native Windows overlay for windowed/borderless clients, hidden initially.
   No exclusive-fullscreen rendering. UTF-8 strings copied, 1..8 tabs.
   Title/labels <= 128 bytes. Hotkey is a Windows virtual key (0 disables).
   Creation asynchronous; destroy joins the UI thread. No mod callbacks. */
typedef struct TextWindowOptions {
    size_t struct_size;
    StringView title;
    const StringView* tabs;
    size_t tab_count;
    uint32_t toggle_key;
} TextWindowOptions;
typedef enum TextWindowStatus { TEXT_PENDING = 0, TEXT_READY = 1, TEXT_FAILED = 2 } TextWindowStatus;
typedef struct UiApi {
    size_t struct_size;
    uint32_t api_version;
    Result (CALL* create)(StringView owner, const TextWindowOptions*, TextWindow*);
    /* Replace one tab's snapshot, <=1 MiB UTF-8; empty clears it.
       Thread safe, coalesced; not an event queue. */
    Result (CALL* set_text)(StringView owner, TextWindow, size_t tab, StringView text);
    Result (CALL* get_status)(StringView owner, TextWindow, TextWindowStatus*);
    Result (CALL* destroy)(StringView owner, TextWindow);
} UiApi;
#ifdef __cplusplus
}
#endif
