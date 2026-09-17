#include "pch.h"
#include "ui_text.h"
#include "utils.h"
#include <atomic>
#include <algorithm>
#include <cwctype>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {
constexpr wchar_t ClassName[] = L"Shroudtopia.Api.TextWindow.1";
constexpr COLORREF Background = RGB(15,25,29), Surface = RGB(22,36,40);
constexpr COLORREF Gold = RGB(201,173,110), Ink = RGB(226,226,213), Muted = RGB(149,170,172);
struct Panel {
    std::string owner;
    std::wstring title, shownText;
    std::vector<std::wstring> labels, pending;
    std::mutex mutex;
    std::thread thread;
    std::atomic<bool> stop{false};
    std::atomic<TextWindowStatus> status{TEXT_PENDING};
    HWND window = nullptr, game = nullptr, edit = nullptr, search = nullptr;
    HFONT heading = nullptr, font = nullptr, mono = nullptr;
    HBRUSH background = nullptr, surface = nullptr;
    uint32_t key = 0;
    size_t tab = 0, level = 0;
    ULONGLONG copiedUntil = 0;
    bool shown = false, keyDown = false, hotkeyRegistered = false;
    bool paused = false, follow = true, dirty = true;
    ~Panel() {
        for (auto f : {heading, font, mono}) if (f) DeleteObject(f);
        for (auto b : {background, surface}) if (b) DeleteObject(b);
    }
};
std::mutex panelsMutex;
std::unordered_map<uint64_t, std::shared_ptr<Panel>> panels;
uint64_t nextId = 1;
HMODULE Instance() {
    HMODULE module = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&Instance), &module);
    return module;
}
std::wstring Wide(StringView value) {
    if (!value.size) return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data, static_cast<int>(value.size), nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data, static_cast<int>(value.size), result.data(), size);
    return result;
}
bool Valid(StringView v, size_t max, bool empty = false) {
    return v.size <= max && (empty ? (!v.size || v.data) : (v.data && v.size));
}
BOOL CALLBACK FindGame(HWND w, LPARAM target) {
    DWORD pid = 0; GetWindowThreadProcessId(w, &pid);
    wchar_t name[100]{}; GetClassNameW(w, name, 100);
    if (pid == GetCurrentProcessId() && IsWindowVisible(w) && !GetWindow(w, GW_OWNER) &&
        wcscmp(name, ClassName) != 0 && !(GetWindowLongPtrW(w, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) {
        *reinterpret_cast<HWND*>(target) = w; return FALSE;
    }
    return TRUE;
}
void Layout(Panel* p) {
    if (!p->edit) return;
    RECT r{}; GetClientRect(p->window, &r);
    MoveWindow(p->search, 86, 68, (std::max)(100L, r.right - 571), 25, TRUE);
    MoveWindow(p->edit, 18, 108, (std::max)(100L, r.right - 36), (std::max)(40L, r.bottom - 160), TRUE);
    InvalidateRect(p->window, nullptr, FALSE);
}
void Text(HDC dc, HFONT font, COLORREF color, RECT rect, const wchar_t* value, UINT align = DT_LEFT) {
    SelectObject(dc, font); SetTextColor(dc, color); SetBkMode(dc, TRANSPARENT);
    DrawTextW(dc, value, -1, &rect, align | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
}
void Button(HDC dc, Panel* p, RECT rect, const wchar_t* value, bool active = false) {
    FillRect(dc, &rect, p->surface);
    HBRUSH outline = CreateSolidBrush(active ? Gold : RGB(70,91,92));
    FrameRect(dc, &rect, outline); DeleteObject(outline);
    Text(dc, p->font, active ? Gold : Ink, rect, value, DT_CENTER);
}
const wchar_t* LevelName(size_t level) {
    static constexpr const wchar_t* names[]{L"Level: All", L"Level: Trace", L"Level: Debug", L"Level: Info", L"Level: Warning", L"Level: Error"};
    return names[level < std::size(names) ? level : 0];
}
bool MatchesLevel(const std::wstring& line, size_t level) {
    if (level == 0) return true;
    const wchar_t* markers[][3]{
        {L"[TRACE]", L"[T ", nullptr},
        {L"[DEBUG]", L"[D ", nullptr},
        {L"[INFO]", L"[I ", nullptr},
        {L"[WARN]", L"[WARNING]", L"[W "},
        {L"[ERROR]", L"[E ", nullptr}
    };
    for (const auto* marker : markers[level - 1])
        if (marker && line.find(marker) != std::wstring::npos) return true;
    return false;
}
bool CopyVisibleLog(Panel* p) {
    const size_t bytes = (p->shownText.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!memory) return false;
    void* target = GlobalLock(memory);
    if (!target) { GlobalFree(memory); return false; }
    std::memcpy(target, p->shownText.c_str(), bytes); GlobalUnlock(memory);
    if (!OpenClipboard(p->window)) { GlobalFree(memory); return false; }
    EmptyClipboard();
    const bool success = SetClipboardData(CF_UNICODETEXT, memory) != nullptr;
    CloseClipboard();
    if (!success) GlobalFree(memory);
    return success;
}
void Paint(Panel* p) {
    PAINTSTRUCT ps{}; HDC dc = BeginPaint(p->window, &ps);
    RECT r{}; GetClientRect(p->window, &r); FillRect(dc, &r, p->background);
    HBRUSH border = CreateSolidBrush(Gold); FrameRect(dc, &r, border);
    RECT line{18,56,r.right-18,57}; FillRect(dc, &line, border);
    Text(dc, p->heading, Gold, {20,7,r.right-65,53}, p->title.c_str());
    Text(dc, p->font, Gold, {r.right-48,10,r.right-12,48}, L"\x00D7", DT_CENTER);
    Text(dc, p->font, Muted, {20,65,86,97}, L"Search");
    Button(dc, p, {r.right-475,65,r.right-355,97}, LevelName(p->level), p->level != 0);
    Button(dc, p, {r.right-345,65,r.right-255,97}, p->copiedUntil ? L"Copied" : L"Copy Logs", p->copiedUntil != 0);
    Button(dc, p, {r.right-245,65,r.right-155,97}, p->paused ? L"Resume" : L"Pause", p->paused);
    Button(dc, p, {r.right-145,65,r.right-18,97}, p->follow ? L"Auto-Scroll: On" : L"Auto-Scroll: Off", p->follow);
    const LONG width = (r.right-36) / static_cast<LONG>(p->labels.size());
    for (size_t i = 0; i < p->labels.size(); ++i) {
        RECT tab{18+static_cast<LONG>(i)*width,r.bottom-42,18+static_cast<LONG>(i+1)*width,r.bottom-10};
        if (p->tab == i) { FillRect(dc, &tab, p->surface); RECT bar{tab.left,tab.top,tab.right,tab.top+2}; FillRect(dc, &bar, border); }
        Text(dc, p->font, p->tab == i ? Gold : Muted, tab, p->labels[i].c_str(), DT_CENTER);
    }
    DeleteObject(border); EndPaint(p->window, &ps);
}
void RefreshText(Panel* p) {
    if (p->paused) return;
    std::wstring source;
    { std::scoped_lock lock(p->mutex); source = p->pending[p->tab]; }
    wchar_t query[256]{}; GetWindowTextW(p->search, query, 256);
    std::wstring needle(query); std::transform(needle.begin(), needle.end(), needle.begin(), towlower);
    std::wstring output;
    for (size_t start = 0; start < source.size();) {
        const auto end = source.find(L'\n', start);
        std::wstring line = source.substr(start, end == std::wstring::npos ? end : end-start);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        auto lower = line;
        if (!needle.empty()) std::transform(lower.begin(), lower.end(), lower.begin(), towlower);
        if (MatchesLevel(line, p->level) && (needle.empty() || lower.find(needle) != std::wstring::npos)) output += line + L"\r\n";
        if (end == std::wstring::npos) break;
        start = end + 1;
    }
    if (!p->dirty && p->shownText == output) return;
    const auto scroll = SendMessageW(p->edit, EM_GETFIRSTVISIBLELINE, 0, 0);
    DWORD begin = 0, end = 0; SendMessageW(p->edit, EM_GETSEL, reinterpret_cast<WPARAM>(&begin), reinterpret_cast<LPARAM>(&end));
    SendMessageW(p->edit, WM_SETREDRAW, FALSE, 0); SetWindowTextW(p->edit, output.c_str());
    if (p->follow) {
        const auto length = static_cast<LPARAM>(output.size());
        SendMessageW(p->edit, EM_SETSEL, length, length);
        SendMessageW(p->edit, WM_VSCROLL, SB_BOTTOM, 0);
        SendMessageW(p->edit, EM_SCROLLCARET, 0, 0);
    }
    else { SendMessageW(p->edit, EM_SETSEL, begin, end); SendMessageW(p->edit, EM_LINESCROLL, 0, scroll); }
    SendMessageW(p->edit, WM_SETREDRAW, TRUE, 0); InvalidateRect(p->edit, nullptr, TRUE);
    p->shownText = std::move(output); p->dirty = false;
}
void Refresh(Panel* p) {
    if (p->copiedUntil && GetTickCount64() >= p->copiedUntil) { p->copiedUntil = 0; InvalidateRect(p->window, nullptr, FALSE); }
    if (!IsWindow(p->game)) {
        p->game = nullptr; EnumWindows(FindGame, reinterpret_cast<LPARAM>(&p->game));
        if (p->game) {
            SetWindowLongPtrW(p->window, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(p->game));
            RECT r{}; GetWindowRect(p->game, &r);
            SetWindowPos(p->window, nullptr, r.left+40, r.top+60, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
        }
    }
    DWORD pid = 0; GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    const bool focused = pid == GetCurrentProcessId();
    const bool down = !p->hotkeyRegistered && p->key &&
        (GetAsyncKeyState(static_cast<int>(p->key)) & 0x8000);
    if (focused && p->game && down && !p->keyDown) { p->shown = !p->shown; if (!p->shown) SetForegroundWindow(p->game); }
    p->keyDown = down;
    const bool visible = p->shown && focused && p->game && !IsIconic(p->game);
    if (visible != (IsWindowVisible(p->window) != FALSE)) ShowWindow(p->window, visible ? SW_SHOWNOACTIVATE : SW_HIDE);
    if (visible) RefreshText(p);
}
LRESULT CALLBACK WindowProc(HWND w, UINT m, WPARAM wp, LPARAM lp) {
    auto* p = reinterpret_cast<Panel*>(GetWindowLongPtrW(w, GWLP_USERDATA));
    if (m == WM_NCCREATE) { p = static_cast<Panel*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams); SetWindowLongPtrW(w, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p)); }
    if (p) switch (m) {
    case WM_CLOSE: p->shown = false; ShowWindow(w, SW_HIDE); if (p->game) SetForegroundWindow(p->game); return 0;
    case WM_GETMINMAXINFO: reinterpret_cast<MINMAXINFO*>(lp)->ptMinTrackSize = {760,320}; return 0;
    case WM_SIZE: Layout(p); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: Paint(p); return 0;
    case WM_CTLCOLOREDIT: case WM_CTLCOLORSTATIC:
        SetTextColor(reinterpret_cast<HDC>(wp), Ink); SetBkColor(reinterpret_cast<HDC>(wp), Surface); return reinterpret_cast<LRESULT>(p->surface);
    case WM_COMMAND: if (HIWORD(wp) == EN_CHANGE && reinterpret_cast<HWND>(lp) == p->search) p->dirty = true; return 0;
    case WM_HOTKEY:
        if (wp == 1 && p->game) {
            DWORD foregroundPid = 0;
            GetWindowThreadProcessId(GetForegroundWindow(), &foregroundPid);
            if (foregroundPid == GetCurrentProcessId()) {
                p->shown = !p->shown;
                if (!p->shown) SetForegroundWindow(p->game);
            }
        }
        return 0;
    case WM_LBUTTONDOWN: {
        RECT r{}; GetClientRect(w, &r);
        const int x = static_cast<short>(LOWORD(lp)), y = static_cast<short>(HIWORD(lp));
        if (y < 56) {
            if (x > r.right-55) PostMessageW(w, WM_CLOSE, 0, 0);
            else { ReleaseCapture(); SendMessageW(w, WM_NCLBUTTONDOWN, HTCAPTION, 0); }
        } else if (y >= 65 && y <= 97) {
            if (x >= r.right-475 && x < r.right-355) p->level = (p->level + 1) % 6;
            else if (x >= r.right-345 && x < r.right-255) {
                if (CopyVisibleLog(p)) p->copiedUntil = GetTickCount64() + 1500;
            } else if (x >= r.right-245 && x < r.right-155) p->paused = !p->paused;
            else if (x >= r.right-145) p->follow = !p->follow;
            p->dirty = true;
        } else if (y >= r.bottom-42 && y <= r.bottom-10 && x >= 18 && x < r.right-18) {
            const auto tab = static_cast<size_t>((x-18) / ((r.right-36) / static_cast<LONG>(p->labels.size())));
            if (tab < p->labels.size()) { p->tab = tab; p->paused = false; p->dirty = true; }
        }
        InvalidateRect(w, nullptr, FALSE); return 0;
    }
    case WM_TIMER:
        if (p->stop) { DestroyWindow(w); return 0; }
        try { Refresh(p); } catch (...) { p->status = TEXT_FAILED; DestroyWindow(w); }
        return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(w, m, wp, lp);
}
void Run(Panel* p) {
    p->background = CreateSolidBrush(Background); p->surface = CreateSolidBrush(Surface);
    p->heading = CreateFontW(-25,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Georgia");
    p->font = CreateFontW(-15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    p->mono = CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Consolas");
    WNDCLASSW wc{}; wc.lpfnWndProc = WindowProc; wc.hInstance = Instance(); wc.lpszClassName = ClassName;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); RegisterClassW(&wc);
    p->window = CreateWindowExW(WS_EX_TOOLWINDOW, ClassName, p->title.c_str(), WS_POPUP | WS_THICKFRAME | WS_CLIPCHILDREN,
        80,80,960,580,nullptr,nullptr,Instance(),p);
    if (p->window) {
        p->edit = CreateWindowExW(0,L"EDIT",L"",WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
            0,0,0,0,p->window,nullptr,Instance(),nullptr);
        p->search = CreateWindowExW(0,L"EDIT",L"",WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,0,0,0,0,p->window,nullptr,Instance(),nullptr);
    }
    if (!p->window || !p->edit || !p->search || !SetTimer(p->window,1,100,nullptr)) {
        p->status = TEXT_FAILED;
        Utils::Log(Utils::ERRR,"Native UI creation failed for %s (Win32 %lu)",p->owner.c_str(),GetLastError());
    } else {
        SendMessageW(p->edit,WM_SETFONT,reinterpret_cast<WPARAM>(p->mono),TRUE);
        SendMessageW(p->edit,EM_SETLIMITTEXT,2*1024*1024,0);
        SendMessageW(p->search,WM_SETFONT,reinterpret_cast<WPARAM>(p->font),TRUE);
        SendMessageW(p->search,EM_SETLIMITTEXT,255,0);
        if (p->key) {
            p->hotkeyRegistered = RegisterHotKey(p->window, 1, MOD_NOREPEAT, p->key) != FALSE;
            Utils::Log(Utils::DEBUG, "Native text window hotkey: owner=%s key=%u registered=%s",
                p->owner.c_str(), p->key, p->hotkeyRegistered ? "true" : "false (polling fallback)");
        }
        Layout(p); p->status = TEXT_READY;
        MSG message{};
        while (GetMessageW(&message,nullptr,0,0) > 0) {
            TranslateMessage(&message); DispatchMessageW(&message);
        }
    }
    if (p->hotkeyRegistered) UnregisterHotKey(p->window, 1);
    if (IsWindow(p->window)) DestroyWindow(p->window);
}
Result CALL Create(StringView owner, const TextWindowOptions* d, TextWindow* out) {
    if (out) *out = 0;
    if (!out || !Valid(owner,256) || !d || d->struct_size < sizeof(*d) || !Valid(d->title,128) ||
        !d->tabs || !d->tab_count || d->tab_count > 8 || d->toggle_key > 255) return RESULT_INVALID_ARGUMENT;
    try {
        auto p = std::make_shared<Panel>(); p->owner.assign(owner.data,owner.size); p->title = Wide(d->title); p->key = d->toggle_key;
        for (size_t i=0; i<d->tab_count; ++i) { if (!Valid(d->tabs[i],128)) return RESULT_INVALID_ARGUMENT; p->labels.push_back(Wide(d->tabs[i])); }
        p->pending.resize(d->tab_count);
        std::scoped_lock lock(panelsMutex); const auto id = nextId++; panels.emplace(id,p);
        try { p->thread = std::thread([p] { Run(p.get()); }); }
        catch (...) { panels.erase(id); return RESULT_INTERNAL_ERROR; }
        *out = id; return RESULT_OK;
    } catch (...) { return RESULT_INTERNAL_ERROR; }
}
Result CALL SetText(StringView owner, TextWindow id, size_t tab, StringView text) {
    if (!Valid(owner,256) || !Valid(text,1024*1024,true)) return RESULT_INVALID_ARGUMENT;
    try {
        std::scoped_lock all(panelsMutex); const auto it = panels.find(id); if (it == panels.end()) return RESULT_NOT_FOUND;
        auto& p = *it->second; if (p.owner.compare(0,std::string::npos,owner.data,owner.size) != 0) return RESULT_PERMISSION_DENIED;
        if (tab >= p.pending.size()) return RESULT_INVALID_ARGUMENT;
        auto value = Wide(text); std::scoped_lock lock(p.mutex); p.pending[tab] = std::move(value); return RESULT_OK;
    } catch (...) { return RESULT_INTERNAL_ERROR; }
}
Result CALL Status(StringView owner, TextWindow id, TextWindowStatus* out) {
    if (!Valid(owner,256) || !out) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(panelsMutex); const auto it = panels.find(id); if (it == panels.end()) return RESULT_NOT_FOUND;
    if (it->second->owner.compare(0,std::string::npos,owner.data,owner.size) != 0) return RESULT_PERMISSION_DENIED;
    *out = it->second->status; return RESULT_OK;
}
Result CALL Destroy(StringView owner, TextWindow id) {
    if (!Valid(owner,256)) return RESULT_INVALID_ARGUMENT;
    std::shared_ptr<Panel> p;
    { std::scoped_lock lock(panelsMutex); const auto it = panels.find(id); if (it == panels.end()) return RESULT_NOT_FOUND;
      if (it->second->owner.compare(0,std::string::npos,owner.data,owner.size) != 0) return RESULT_PERMISSION_DENIED;
      p = it->second; panels.erase(it); }
    p->stop = true; if (p->thread.joinable()) p->thread.join(); return RESULT_OK;
}
const UiApi api{sizeof(api),API_VERSION,Create,SetText,Status,Destroy};
}
namespace UiText {
const UiApi* Api() { return &api; }
void ReleaseOwner(StringView owner) {
    if (!Valid(owner,256)) return;
    for (;;) {
        uint64_t id = 0;
        { std::scoped_lock lock(panelsMutex); for (const auto& [key,p] : panels)
            if (p->owner.compare(0,std::string::npos,owner.data,owner.size) == 0) { id = key; break; } }
        if (!id) return;
        Destroy(owner,id);
    }
}
void Shutdown() {
    for (;;) {
        std::string owner;
        { std::scoped_lock lock(panelsMutex); if (panels.empty()) break; owner = panels.begin()->second->owner; }
        ReleaseOwner({owner.data(),owner.size()});
    }
    UnregisterClassW(ClassName,Instance());
}
}
