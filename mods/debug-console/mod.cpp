#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "shroudtopia.h"
#include <string>
#include <cstring>
#include <cstddef>
#include <cmath>

namespace {
StringView View(const char* v) { return {v, std::strlen(v)}; }
constexpr char Owner[] = "mod.debug-console";
const Api* api = nullptr;
TextWindow window = 0;
double elapsed = 1;
bool failureReported = false;
std::string logBuffer(1024 * 1024, '\0');
Result CALL Load(const Api* api, void*) {
    if (!api || api->api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    return RESULT_OK;
}
Result CALL Activate(const Api* api, void*) {
    if (window) return RESULT_CONFLICT;
    if (!api || !api->create_text_window || !api->set_text_window_text ||
        !api->get_text_window_status || !api->destroy_text_window || !api->read_log_tail) return RESULT_NOT_FOUND;
    ::api = api;
    auto result = RESULT_OK;
    double key = 121;
    if (api->struct_size >= offsetof(Api, get_mod_setting_number) + sizeof(api->get_mod_setting_number) && api->get_mod_setting_number)
        api->get_mod_setting_number(View(Owner), View("toggleKey"), 121, &key);
    if (!std::isfinite(key) || key < 1 || key > 255 || std::floor(key) != key) key = 121;
    const StringView tabs[]{View("Enshrouded Log"), View("Shroudtopia Debug Log")};
    TextWindowOptions descriptor{sizeof(descriptor), View("Shroudtopia | Debug Console"), tabs, 2, static_cast<uint32_t>(key)};
    elapsed = 1; failureReported = false;
    result = api->create_text_window(View(Owner), &descriptor, &window);
    if (result == RESULT_OK) {
        const auto message = "Debug Console loaded. Configured virtual-key code: " +
            std::to_string(static_cast<uint32_t>(key)) + ". Reads existing log files through the API.";
        api->log(View(Owner), LOG_INFO, {message.data(), message.size()});
    }
    return result;
}
Result CALL Update(const Api* api, void*, double delta) {
    if (!window) return RESULT_OK;
    elapsed += delta; if (elapsed < 0.5) return RESULT_OK; elapsed = 0;
    TextWindowStatus status{};
    if (api->get_text_window_status(View(Owner), window, &status) != RESULT_OK || status == TEXT_FAILED) {
        if (!failureReported) api->log(View(Owner), LOG_ERROR, View("Debug Console window unavailable; inspect the existing shroudtopia.log for details."));
        failureReported = true; return RESULT_OK;
    }
    try {
        for (const auto source : {LOG_SOURCE_LOADER, LOG_SOURCE_GAME}) {
            size_t written = 0;
            const auto result = api->read_log_tail(source, logBuffer.data(), logBuffer.size(), &written);
            const auto text = result == RESULT_OK ? StringView{logBuffer.data(), written} :
                View(result == RESULT_NOT_FOUND ? "Log file is not available yet." : "Log file could not be read.");
            const auto update = api->set_text_window_text(View(Owner), window, source == LOG_SOURCE_GAME ? 0 : 1, text);
            if (update != RESULT_OK) return update;
        }
        return RESULT_OK;
    } catch (...) { return RESULT_INTERNAL_ERROR; }
}
Result CALL Deactivate(const Api*, void*) {
    if (window && api) { const auto result = api->destroy_text_window(View(Owner), window); if (result != RESULT_OK) return result; }
    window = 0; return RESULT_OK;
}
Result CALL Unload(const Api* value, void*) {
    Deactivate(value, nullptr); api = nullptr;
    return RESULT_OK;
}
}
extern "C" __declspec(dllexport) Result CALL CreateMod(uint32_t api_version, ModDescriptor* descriptor) {
    if (api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    if (!descriptor || descriptor->struct_size < sizeof(*descriptor)) return RESULT_INVALID_ARGUMENT;
    *descriptor = {sizeof(*descriptor), View(Owner), nullptr, Load, Activate, Update, Deactivate, Unload};
    return RESULT_OK;
}
BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}
