#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "shroudtopia/api.h"
#include <string>
#include <cstring>
#include <cstddef>
#include <cmath>

namespace {
StringView View(const char* v) { return {v, std::strlen(v)}; }
constexpr char Owner[] = "mod.debug-console";
const UiApi* ui = nullptr;
const LogApi* logs = nullptr;
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
    ui = api->ui;
    logs = api->logs;
    if (!ui || !logs) return RESULT_NOT_FOUND;
    auto result = RESULT_OK;
    double key = 121;
    if (api->struct_size >= offsetof(Api, get_mod_setting_number) + sizeof(api->get_mod_setting_number) && api->get_mod_setting_number)
        api->get_mod_setting_number(View(Owner), View("toggleKey"), 121, &key);
    if (!std::isfinite(key) || key < 1 || key > 255 || std::floor(key) != key) key = 121;
    const StringView tabs[]{View("Enshrouded Log"), View("Shroudtopia Debug Log")};
    TextWindowOptions descriptor{sizeof(descriptor), View("Shroudtopia | Debug Console"), tabs, 2, static_cast<uint32_t>(key)};
    elapsed = 1; failureReported = false;
    result = ui->create(View(Owner), &descriptor, &window);
    if (result == RESULT_OK) api->log(View(Owner), LOG_INFO, View("Debug Console loaded. Default hotkey F10; reads existing log files through the API."));
    return result;
}
Result CALL Update(const Api* api, void*, double delta) {
    if (!window) return RESULT_OK;
    elapsed += delta; if (elapsed < 0.5) return RESULT_OK; elapsed = 0;
    TextWindowStatus status{};
    if (ui->get_status(View(Owner), window, &status) != RESULT_OK || status == TEXT_FAILED) {
        if (!failureReported) api->log(View(Owner), LOG_ERROR, View("Debug Console window unavailable; inspect the existing shroudtopia.log for details."));
        failureReported = true; return RESULT_OK;
    }
    try {
        for (const auto source : {LOG_SOURCE_LOADER, LOG_SOURCE_GAME}) {
            size_t written = 0;
            const auto result = logs->read_tail(source, logBuffer.data(), logBuffer.size(), &written);
            const auto text = result == RESULT_OK ? StringView{logBuffer.data(), written} :
                View(result == RESULT_NOT_FOUND ? "Log file is not available yet." : "Log file could not be read.");
            const auto update = ui->set_text(View(Owner), window, source == LOG_SOURCE_GAME ? 0 : 1, text);
            if (update != RESULT_OK) return update;
        }
        return RESULT_OK;
    } catch (...) { return RESULT_INTERNAL_ERROR; }
}
Result CALL Deactivate(const Api*, void*) {
    if (window && ui) { const auto result = ui->destroy(View(Owner), window); if (result != RESULT_OK) return result; }
    window = 0; return RESULT_OK;
}
Result CALL Unload(const Api* api, void*) {
    Deactivate(api, nullptr); ui = nullptr; logs = nullptr;
    return api->release_owner(View(Owner));
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
