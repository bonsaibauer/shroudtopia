#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ui_module.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {
constexpr char Owner[] = "shroudtopia.ui";
constexpr wchar_t ClassName[] = L"Shroudtopia.Shroudforge.Native.1";
constexpr COLORREF Background = RGB(15, 25, 29), Surface = RGB(22, 36, 40);
constexpr COLORREF Gold = RGB(201, 173, 110), Ink = RGB(226, 226, 213), Muted = RGB(149, 170, 172);
constexpr UINT_PTR RefreshTimer = 1;
constexpr UINT PageListId = 1000, TabBaseId = 2000, ControlBaseId = 3000;

const Api* host{};
Registration about_registration{};

struct ControlModel {
    std::string id, label, description;
    UiControlType type{};
    double minimum{}, maximum{}, step{}, number{};
    uint8_t boolean{};
    UiStatusTone tone{};
};
struct TabModel { std::string id, title; std::vector<ControlModel> controls; };
struct PageModel {
    std::string owner, id, title, description;
    std::vector<TabModel> tabs;
    bool custom{};
};
struct Binding { size_t page{}, tab{}, control{}; HWND window{}; };
struct NativeUi {
    std::thread thread;
    std::atomic<bool> stop{false}, ready{false};
    HWND window{}, game{}, page_list{};
    HFONT heading{}, font{}, small{};
    HBRUSH background{}, surface{};
    std::vector<HWND> children;
    std::vector<PageModel> pages;
    std::unordered_map<UINT, Binding> bindings;
    std::string selected_owner, selected_page, signature;
    size_t selected_index{}, selected_tab{};
    bool shown{}, escape_down{};
};
NativeUi ui;

HMODULE Instance() {
    HMODULE module{};
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&Instance), &module);
    return module;
}
StringView View(const char* value) { return {value, std::strlen(value)}; }
std::string Copy(StringView value) { return value.data ? std::string(value.data, value.size) : std::string{}; }
std::wstring Wide(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
    return result;
}

BOOL CALLBACK FindGame(HWND window, LPARAM output) {
    DWORD pid{}; GetWindowThreadProcessId(window, &pid);
    wchar_t name[128]{}; GetClassNameW(window, name, static_cast<int>(std::size(name)));
    if (pid == GetCurrentProcessId() && IsWindowVisible(window) && !GetWindow(window, GW_OWNER) &&
        wcscmp(name, ClassName) != 0 && !(GetWindowLongPtrW(window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) {
        *reinterpret_cast<HWND*>(output) = window; return FALSE;
    }
    return TRUE;
}

Result CALL CollectPage(StringView owner, UiPageDescriptor* page, void* data) {
    auto& pages = *static_cast<std::vector<PageModel>*>(data);
    PageModel model;
    model.owner = Copy(owner); model.id = Copy(page->id); model.title = Copy(page->title);
    model.description = Copy(page->description); model.custom = page->render != nullptr;
    for (size_t t = 0; t < page->tab_count; ++t) {
        const auto& source_tab = page->tabs[t];
        TabModel tab{Copy(source_tab.id), Copy(source_tab.title)};
        tab.controls.reserve(source_tab.control_count);
        for (size_t c = 0; c < source_tab.control_count; ++c) {
            const auto& source = source_tab.controls[c];
            tab.controls.push_back({Copy(source.id), Copy(source.label), Copy(source.description), source.type,
                source.minimum, source.maximum, source.step, source.number_value, source.bool_value, source.status_tone});
        }
        model.tabs.push_back(std::move(tab));
    }
    pages.push_back(std::move(model));
    return RESULT_OK;
}

std::string Signature(const std::vector<PageModel>& pages) {
    std::string result;
    for (const auto& page : pages) {
        result += page.owner + '\n' + page.id + '\n' + page.title + '\n';
        for (const auto& tab : page.tabs) {
            result += tab.id + '\n' + tab.title + '\n';
            for (const auto& control : tab.controls)
                result += control.id + ':' + std::to_string(control.type) + ':' +
                    std::to_string(control.boolean) + ':' + std::to_string(control.number) + '\n';
        }
    }
    return result;
}

void DestroyChildren() {
    for (HWND child : ui.children) if (IsWindow(child)) DestroyWindow(child);
    ui.children.clear(); ui.bindings.clear(); ui.page_list = nullptr;
}
HWND Child(const wchar_t* type, const std::wstring& text, DWORD style, int x, int y, int width, int height, UINT id) {
    HWND child = CreateWindowExW(0, type, text.c_str(), WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, ui.window, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)), Instance(), nullptr);
    if (child) { SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(ui.font), TRUE); ui.children.push_back(child); }
    return child;
}

void RebuildControls() {
    DestroyChildren();
    RECT client{}; GetClientRect(ui.window, &client);
    const int rail = static_cast<int>(std::clamp(client.right / 4L, 210L, 320L));
    ui.page_list = Child(L"LISTBOX", L"", LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_VSCROLL,
        18, 92, rail - 28, (std::max)(100L, client.bottom - 112), PageListId);
    for (const auto& page : ui.pages) {
        const auto title = Wide(page.title);
        SendMessageW(ui.page_list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(title.c_str()));
    }
    if (ui.pages.empty()) return;
    ui.selected_index = (std::min)(ui.selected_index, ui.pages.size() - 1);
    SendMessageW(ui.page_list, LB_SETCURSEL, ui.selected_index, 0);
    auto& page = ui.pages[ui.selected_index];
    ui.selected_owner = page.owner; ui.selected_page = page.id;
    const int content_x = rail + 18, content_width = static_cast<int>((std::max)(240L, client.right - content_x - 18L));
    int y = 92;
    if (page.custom) {
        Child(L"STATIC", L"Benutzerdefinierte Mod-Ansicht (native Darstellung nicht verfügbar).",
            SS_LEFT, content_x, y, content_width, 50, ControlBaseId);
        return;
    }
    if (page.tabs.empty()) return;
    ui.selected_tab = (std::min)(ui.selected_tab, page.tabs.size() - 1);
    const int tab_width = (std::max)(100, content_width / static_cast<int>(page.tabs.size()));
    for (size_t t = 0; t < page.tabs.size(); ++t)
        Child(L"BUTTON", Wide(page.tabs[t].title), BS_PUSHBUTTON, content_x + static_cast<int>(t) * tab_width,
            y, tab_width - 6, 30, TabBaseId + static_cast<UINT>(t));
    y += 46;
    auto& tab = page.tabs[ui.selected_tab];
    for (size_t c = 0; c < tab.controls.size() && y < client.bottom - 42; ++c) {
        auto& control = tab.controls[c]; const UINT id = ControlBaseId + static_cast<UINT>(c); HWND child{};
        switch (control.type) {
        case UI_CONTROL_SEPARATOR:
            child = Child(L"STATIC", Wide(control.label), SS_ETCHEDHORZ, content_x, y + 9, content_width, 18, id); y += 30; break;
        case UI_CONTROL_BOOL:
            child = Child(L"BUTTON", Wide(control.label), BS_AUTOCHECKBOX, content_x, y, content_width, 28, id);
            if (child) SendMessageW(child, BM_SETCHECK, control.boolean ? BST_CHECKED : BST_UNCHECKED, 0); y += 38; break;
        case UI_CONTROL_NUMBER: {
            Child(L"STATIC", Wide(control.label), SS_LEFT, content_x, y + 4, content_width / 2, 24, id + 500);
            wchar_t number[64]{}; swprintf_s(number, L"%.3g", control.number);
            child = Child(L"EDIT", number, WS_BORDER | ES_AUTOHSCROLL, content_x + content_width / 2, y,
                content_width / 2, 27, id); y += 38; break;
        }
        case UI_CONTROL_BUTTON:
            child = Child(L"BUTTON", Wide(control.label), BS_PUSHBUTTON, content_x, y, (std::min)(260, content_width), 30, id); y += 40; break;
        case UI_CONTROL_STATUS: case UI_CONTROL_TEXT:
            child = Child(L"STATIC", Wide(control.label), SS_LEFT, content_x, y, content_width, 32, id); y += 38; break;
        }
        if (child) ui.bindings[id] = {ui.selected_index, ui.selected_tab, c, child};
        if (!control.description.empty() && y < client.bottom - 28) {
            HWND description = Child(L"STATIC", Wide(control.description), SS_LEFT, content_x + 20, y - 8, content_width - 20, 25, id + 700);
            if (description) SendMessageW(description, WM_SETFONT, reinterpret_cast<WPARAM>(ui.small), TRUE);
            y += 25;
        }
    }
    InvalidateRect(ui.window, nullptr, TRUE);
}

void RefreshModel() {
    std::vector<PageModel> pages;
    if (!host || host->visit_ui_pages(View(Owner), CollectPage, &pages) != RESULT_OK) return;
    const auto signature = Signature(pages);
    if (signature == ui.signature) return;
    const auto owner = ui.selected_owner, page_id = ui.selected_page;
    ui.pages = std::move(pages); ui.signature = signature;
    const auto selected = std::find_if(ui.pages.begin(), ui.pages.end(), [&](const auto& page) {
        return page.owner == owner && page.id == page_id;
    });
    ui.selected_index = selected == ui.pages.end() ? 0 : static_cast<size_t>(selected - ui.pages.begin());
    RebuildControls();
}

Result CALL ApplyVisitor(StringView owner, UiPageDescriptor* page, void* data) {
    const auto& binding = *static_cast<const Binding*>(data);
    if (binding.page >= ui.pages.size()) return RESULT_OK;
    const auto& expected = ui.pages[binding.page];
    if (Copy(owner) != expected.owner || Copy(page->id) != expected.id || binding.tab >= page->tab_count ||
        binding.control >= page->tabs[binding.tab].control_count) return RESULT_OK;
    auto& control = const_cast<UiControlDescriptor&>(page->tabs[binding.tab].controls[binding.control]);
    if (control.type == UI_CONTROL_BOOL)
        control.bool_value = SendMessageW(binding.window, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0;
    else if (control.type == UI_CONTROL_NUMBER) {
        wchar_t text[64]{}; GetWindowTextW(binding.window, text, static_cast<int>(std::size(text)));
        wchar_t* end{}; const double value = wcstod(text, &end);
        if (end != text && std::isfinite(value)) control.number_value = std::clamp(value, control.minimum, control.maximum);
    }
    return page->on_control ? page->on_control(control.id, control.bool_value, control.number_value, page->user_data) : RESULT_OK;
}
void Apply(const Binding& binding) {
    if (host) host->visit_ui_pages(View(Owner), ApplyVisitor, const_cast<Binding*>(&binding));
    ui.signature.clear();
}

void Paint() {
    PAINTSTRUCT paint{}; HDC dc = BeginPaint(ui.window, &paint);
    RECT rect{}; GetClientRect(ui.window, &rect); FillRect(dc, &rect, ui.background);
    HBRUSH border = CreateSolidBrush(Gold); FrameRect(dc, &rect, border);
    RECT line{18, 62, rect.right - 18, 64}; FillRect(dc, &line, border);
    SetBkMode(dc, TRANSPARENT); SetTextColor(dc, Gold); SelectObject(dc, ui.heading);
    RECT heading{22, 8, rect.right - 220, 58}; DrawTextW(dc, L"SHROUDFORGE", -1, &heading, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SetTextColor(dc, Muted); SelectObject(dc, ui.small);
    const auto version = Wide("Native UI " + std::string(ShroudforgeUi::Version) + "  |  Shroudtopia API 1.2");
    RECT version_rect{220, 12, rect.right - 64, 56}; DrawTextW(dc, version.c_str(), -1, &version_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SetTextColor(dc, Gold); SelectObject(dc, ui.font);
    RECT close{rect.right - 50, 10, rect.right - 12, 50}; DrawTextW(dc, L"\x00D7", -1, &close, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SetTextColor(dc, RGB(79, 189, 237));
    RECT label{20, 66, 330, 91}; DrawTextW(dc, L"MODS-EINSTELLUNGEN", -1, &label, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(border); EndPaint(ui.window, &paint);
}

void PositionAndVisibility() {
    if (!IsWindow(ui.game)) { ui.game = nullptr; EnumWindows(FindGame, reinterpret_cast<LPARAM>(&ui.game)); }
    if (!ui.game) return;
    SetWindowLongPtrW(ui.window, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(ui.game));
    RECT client{}; GetClientRect(ui.game, &client); POINT origin{}; ClientToScreen(ui.game, &origin);
    const int width = client.right, height = client.bottom;
    const int rail = std::clamp(static_cast<int>(width * 0.18), 280, 420);
    SetWindowPos(ui.window, HWND_TOPMOST, origin.x + rail + 16, origin.y + 64,
        (std::max)(640, width - rail - 48), (std::max)(420, height - 128), SWP_NOACTIVATE);
    DWORD foreground_pid{}; GetWindowThreadProcessId(GetForegroundWindow(), &foreground_pid);
    const bool focused = foreground_pid == GetCurrentProcessId();
    const SHORT key = GetAsyncKeyState(VK_ESCAPE); const bool down = (key & 0x8000) != 0;
    if (focused && ((key & 1) != 0 || (down && !ui.escape_down))) ui.shown = !ui.shown;
    ui.escape_down = down;
    const bool visible = ui.shown && focused && !IsIconic(ui.game);
    if (visible != (IsWindowVisible(ui.window) != FALSE)) ShowWindow(ui.window, visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wp, LPARAM lp) {
    switch (message) {
    case WM_CLOSE: ui.shown = false; ShowWindow(window, SW_HIDE); if (ui.game) SetForegroundWindow(ui.game); return 0;
    case WM_GETMINMAXINFO: reinterpret_cast<MINMAXINFO*>(lp)->ptMinTrackSize = {700, 420}; return 0;
    case WM_SIZE:
        // CreateWindowEx sends WM_SIZE before ui.window receives its return value.
        if (ui.window) RebuildControls();
        return 0;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: Paint(); return 0;
    case WM_CTLCOLORLISTBOX: case WM_CTLCOLORBTN: case WM_CTLCOLORSTATIC: case WM_CTLCOLOREDIT:
        SetTextColor(reinterpret_cast<HDC>(wp), Ink); SetBkColor(reinterpret_cast<HDC>(wp), Surface);
        return reinterpret_cast<LRESULT>(ui.surface);
    case WM_COMMAND: {
        const UINT id = LOWORD(wp), notification = HIWORD(wp);
        if (id == PageListId && notification == LBN_SELCHANGE) {
            const auto selected = SendMessageW(ui.page_list, LB_GETCURSEL, 0, 0);
            if (selected != LB_ERR) { ui.selected_index = static_cast<size_t>(selected); ui.selected_tab = 0; RebuildControls(); }
        } else if (id >= TabBaseId && id < ControlBaseId && notification == BN_CLICKED) {
            ui.selected_tab = id - TabBaseId; RebuildControls();
        } else if (const auto binding = ui.bindings.find(id); binding != ui.bindings.end() &&
            (notification == BN_CLICKED || notification == EN_KILLFOCUS)) Apply(binding->second);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        RECT rect{}; GetClientRect(window, &rect);
        const int x = static_cast<short>(LOWORD(lp)), y = static_cast<short>(HIWORD(lp));
        if (y < 62) {
            if (x > rect.right - 58) PostMessageW(window, WM_CLOSE, 0, 0);
            else { ReleaseCapture(); SendMessageW(window, WM_NCLBUTTONDOWN, HTCAPTION, 0); }
        }
        return 0;
    }
    case WM_TIMER:
        if (ui.stop) { DestroyWindow(window); return 0; }
        try { RefreshModel(); PositionAndVisibility(); } catch (...) { DestroyWindow(window); }
        return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(window, message, wp, lp);
}

void Run() {
    ui.background = CreateSolidBrush(Background); ui.surface = CreateSolidBrush(Surface);
    ui.heading = CreateFontW(-26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Georgia");
    ui.font = CreateFontW(-17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    ui.small = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    WNDCLASSW window_class{}; window_class.lpfnWndProc = WindowProcedure; window_class.hInstance = Instance();
    window_class.lpszClassName = ClassName; window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW); RegisterClassW(&window_class);
    ui.window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, ClassName, L"Shroudforge",
        WS_POPUP | WS_THICKFRAME | WS_CLIPCHILDREN, 100, 80, 1000, 650, nullptr, nullptr, Instance(), nullptr);
    if (!ui.window || !SetTimer(ui.window, RefreshTimer, 50, nullptr)) return;
    ui.ready = true; MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
    DestroyChildren();
    if (ui.heading) DeleteObject(ui.heading); if (ui.font) DeleteObject(ui.font); if (ui.small) DeleteObject(ui.small);
    if (ui.background) DeleteObject(ui.background); if (ui.surface) DeleteObject(ui.surface);
    ui.window = nullptr; ui.ready = false;
}
}

namespace ShroudforgeUi {
Result Initialize(const Api* api) {
    if (!api || api->api_version != API_VERSION || !api->register_ui_page || !api->visit_ui_pages) return RESULT_VERSION_MISMATCH;
    host = api;
    static const UiControlDescriptor controls[]{
        {sizeof(UiControlDescriptor), View("version"), View("Shroudforge Native UI 0.2.0"), {}, UI_CONTROL_STATUS, 0, 0, 0, 0, 0, {}, 0, UI_STATUS_SUCCESS},
        {sizeof(UiControlDescriptor), View("api"), View("Shroudtopia API 1.2"), {}, UI_CONTROL_TEXT, 0, 0, 0, 0, 0, {}, 0, UI_STATUS_NEUTRAL}
    };
    static const UiTabDescriptor tabs[]{ {sizeof(UiTabDescriptor), View("about"), View("Allgemein"), controls, std::size(controls)} };
    static const UiPageDescriptor about{sizeof(UiPageDescriptor), View("about"), View("Shroudforge"),
        View("Native Mod-Einstellungen"), 0, tabs, std::size(tabs), nullptr, nullptr, nullptr};
    const auto result = api->register_ui_page(View(Owner), &about, &about_registration);
    if (result != RESULT_OK) return result;
    ui.stop = false;
    try { ui.thread = std::thread(Run); }
    catch (...) { api->release_registration(about_registration); about_registration = 0; host = nullptr; return RESULT_INTERNAL_ERROR; }
    return RESULT_OK;
}
void Tick() {}
void Shutdown() {
    ui.stop = true; if (ui.window) PostMessageW(ui.window, WM_TIMER, RefreshTimer, 0);
    if (ui.thread.joinable()) ui.thread.join();
    if (host && about_registration) host->release_registration(about_registration);
    about_registration = 0; host = nullptr; UnregisterClassW(ClassName, Instance());
}
const char* RendererStatus() { return ui.ready ? "native Win32 ready" : "native Win32 starting"; }
}
