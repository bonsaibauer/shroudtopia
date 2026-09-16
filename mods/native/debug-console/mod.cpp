#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "shroudtopia/api.h"
#include <string>
#include <cstring>
#include <cstddef>
#include <cmath>

namespace {
ST_StringView View(const char* v) { return {v, std::strlen(v)}; }
constexpr char Owner[] = "mod.debug-console";
const ST_UiTextApiV1* ui = nullptr;
const ST_LogReadApiV1* logs = nullptr;
ST_TextWindow window = 0;
double elapsed = 1;
bool failureReported = false;
std::string logBuffer(1024 * 1024, '\0');
template<class T> ST_Result Find(const ST_HostApiV1* host, const char* id, const T** out) {
    ST_ServiceRequest request{sizeof(request), View(id), 1, 0};
    const void* result = nullptr;
    auto code = host->find_service(&request, &result);
    if (code != ST_RESULT_OK) return code;
    *out = static_cast<const T*>(result);
    return *out && (*out)->struct_size >= sizeof(T) && (*out)->abi_version == ST_ABI_VERSION_1 ? ST_RESULT_OK : ST_RESULT_VERSION_MISMATCH;
}
ST_Result ST_CALL Load(const ST_HostApiV1* host, void*) {
    if (!host || host->abi_version != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    return ST_RESULT_OK;
}
ST_Result ST_CALL Activate(const ST_HostApiV1* host, void*) {
    if (window) return ST_RESULT_ALREADY_EXISTS;
    auto result = Find(host, ST_UI_TEXT_SERVICE_ID, &ui);
    if (result != ST_RESULT_OK) return result;
    result = Find(host, ST_LOG_READ_SERVICE_ID, &logs);
    if (result != ST_RESULT_OK) return result;
    double key = 121;
    if (host->struct_size >= offsetof(ST_HostApiV1, get_setting_number) + sizeof(host->get_setting_number) && host->get_setting_number)
        host->get_setting_number(View(Owner), View("toggleKey"), 121, &key);
    if (!std::isfinite(key) || key < 1 || key > 255 || std::floor(key) != key) key = 121;
    const ST_StringView tabs[]{View("Enshrouded Log"), View("Shroudtopia Debug Log")};
    ST_TextWindowDescriptorV1 descriptor{sizeof(descriptor), View("Shroudtopia | Debug Console"), tabs, 2, static_cast<uint32_t>(key)};
    elapsed = 1; failureReported = false;
    result = ui->create(View(Owner), &descriptor, &window);
    if (result == ST_RESULT_OK) host->log(View(Owner), ST_LOG_INFO, View("Debug Console loaded. Default hotkey F10; reads existing log files through the API."));
    return result;
}
ST_Result ST_CALL Update(const ST_HostApiV1* host, void*, double delta) {
    if (!window) return ST_RESULT_OK;
    elapsed += delta; if (elapsed < 0.5) return ST_RESULT_OK; elapsed = 0;
    ST_TextWindowStatusV1 status{};
    if (ui->get_status(View(Owner), window, &status) != ST_RESULT_OK || status == ST_TEXT_FAILED) {
        if (!failureReported) host->log(View(Owner), ST_LOG_ERROR, View("Debug Console window unavailable; inspect the existing shroudtopia.log for details."));
        failureReported = true; return ST_RESULT_OK;
    }
    try {
        for (const auto source : {ST_LOG_SOURCE_LOADER, ST_LOG_SOURCE_GAME}) {
            size_t written = 0;
            const auto result = logs->read_tail(source, logBuffer.data(), logBuffer.size(), &written);
            const auto text = result == ST_RESULT_OK ? ST_StringView{logBuffer.data(), written} :
                View(result == ST_RESULT_NOT_FOUND ? "Log file is not available yet." : "Log file could not be read.");
            const auto update = ui->set_text(View(Owner), window, source == ST_LOG_SOURCE_GAME ? 0 : 1, text);
            if (update != ST_RESULT_OK) return update;
        }
        return ST_RESULT_OK;
    } catch (...) { return ST_RESULT_INTERNAL_ERROR; }
}
ST_Result ST_CALL Deactivate(const ST_HostApiV1*, void*) {
    if (window && ui) { const auto result = ui->destroy(View(Owner), window); if (result != ST_RESULT_OK) return result; }
    window = 0; return ST_RESULT_OK;
}
ST_Result ST_CALL Unload(const ST_HostApiV1* host, void*) {
    Deactivate(host, nullptr); ui = nullptr; logs = nullptr;
    return host->release_owner(View(Owner));
}
}
extern "C" __declspec(dllexport) ST_Result ST_CALL ShroudtopiaCreateModV1(uint32_t abi, ST_ModDescriptorV1* descriptor) {
    if (abi != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    if (!descriptor || descriptor->struct_size < sizeof(*descriptor)) return ST_RESULT_INVALID_ARGUMENT;
    *descriptor = {sizeof(*descriptor), View(Owner), nullptr, Load, Activate, Update, Deactivate, Unload};
    return ST_RESULT_OK;
}
BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}
