#include "pch.h"
#include "assets_engine.h"
#include "utils.h"

#include <filesystem>
#include <mutex>
#include <string>

namespace {
using OpenFn = Result (CALL*)(StringView, StringView);
using CloseFn = void (CALL*)();
using ListFn = Result (CALL*)(StringView, AssetVisitor, void*);
using GetFn = Result (CALL*)(StringView, const AssetId*, char*, size_t, size_t*);
using UpdateFn = Result (CALL*)(StringView, const AssetId*, StringView);
using SetFn = Result (CALL*)(StringView, const AssetId*, StringView, StringView);
using CreateFn = Result (CALL*)(StringView, StringView, StringView, AssetVisitor, void*);
using ResetFn = Result (CALL*)(StringView);
using SaveFn = Result (CALL*)();

struct Functions {
    HMODULE module{};
    OpenFn open{};
    CloseFn close{};
    ListFn list{};
    GetFn get{};
    UpdateFn update{};
    CreateFn create{};
    SetFn set{};
    ResetFn reset{};
    SaveFn save{};
    bool available{};
};

Functions functions;
std::once_flag initializeOnce;

StringView View(const std::string& value) { return {value.data(), value.size()}; }

template <typename T>
T Function(const char* name) {
    return reinterpret_cast<T>(GetProcAddress(functions.module, name));
}

std::filesystem::path ModulePath() {
    HMODULE module = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&ModulePath), &module);
    std::wstring buffer(32768, L'\0');
    const auto length = GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));
    buffer.resize(length);
    return buffer;
}

std::filesystem::path ProcessPath() {
    std::wstring buffer(32768, L'\0');
    const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    buffer.resize(length);
    return buffer;
}

std::string Utf8(const std::filesystem::path& path) {
    const auto wide = path.wstring();
    const auto required = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
        nullptr, 0, nullptr, nullptr);
    std::string value(static_cast<size_t>(required), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
        value.data(), required, nullptr, nullptr);
    return value;
}

void InitializeOnce() {
    const auto libraryPath = ModulePath().parent_path() / L"shroudtopia.dll";
    functions.module = LoadLibraryW(libraryPath.c_str());
    if (functions.module == nullptr) {
        Utils::Log(LOG_DEBUG, "Asset engine library unavailable: %s", Utf8(libraryPath).c_str());
        return;
    }
    functions.open = Function<OpenFn>("ShroudtopiaAssetsOpen");
    functions.close = Function<CloseFn>("ShroudtopiaAssetsClose");
    functions.list = Function<ListFn>("ShroudtopiaListAssets");
    functions.get = Function<GetFn>("ShroudtopiaGetAssetJson");
    functions.update = Function<UpdateFn>("ShroudtopiaUpdateAssetJson");
    functions.set = Function<SetFn>("ShroudtopiaSetAssetFieldJson");
    functions.create = Function<CreateFn>("ShroudtopiaCreateAssetJson");
    functions.reset = Function<ResetFn>("ShroudtopiaResetAssets");
    functions.save = Function<SaveFn>("ShroudtopiaSaveAssets");
    if (functions.open == nullptr || functions.close == nullptr || functions.list == nullptr ||
        functions.get == nullptr || functions.update == nullptr || functions.create == nullptr ||
        functions.reset == nullptr || functions.save == nullptr || functions.set == nullptr) {
        Utils::Log(LOG_DEBUG, "Asset engine rejected: required exports are incomplete");
        return;
    }

    const auto executable = ProcessPath();
    const auto directory = Utf8(executable.parent_path());
    const auto stem = Utf8(executable.stem());
    functions.available = functions.open(View(directory), View(stem)) == RESULT_OK;
    Utils::Log(LOG_DEBUG, "Asset engine initialize: executable=%s available=%s",
        Utf8(executable).c_str(), functions.available ? "true" : "false");
}
}

namespace AssetsEngine {
bool Initialize() {
    std::call_once(initializeOnce, InitializeOnce);
    return functions.available;
}

bool Available() { return Initialize(); }

void Shutdown() {
    if (functions.available && functions.close != nullptr) functions.close();
    functions.available = false;
    if (functions.module != nullptr) {
        FreeLibrary(functions.module);
        functions.module = nullptr;
    }
}

Result List(StringView typeName, AssetVisitor visitor, void* userData) {
    return Available() ? functions.list(typeName, visitor, userData) : RESULT_NOT_FOUND;
}
Result Get(StringView owner, const AssetId* asset,
    char* buffer, size_t capacity, size_t* requiredSize) {
    return Available() ? functions.get(owner, asset, buffer, capacity, requiredSize) : RESULT_NOT_FOUND;
}
Result Update(StringView owner, const AssetId* asset, StringView json) {
    return Available() ? functions.update(owner, asset, json) : RESULT_NOT_FOUND;
}
Result Set(StringView owner, const AssetId* asset, StringView path, StringView json) {
    return Available() ? functions.set(owner, asset, path, json) : RESULT_NOT_FOUND;
}
Result Create(StringView owner, StringView typeName, StringView json,
    AssetVisitor visitor, void* userData) {
    return Available() ? functions.create(owner, typeName, json, visitor, userData) : RESULT_NOT_FOUND;
}
Result Reset(StringView owner) {
    return Available() ? functions.reset(owner) : RESULT_NOT_FOUND;
}
Result Save() { return Available() ? functions.save() : RESULT_NOT_FOUND; }
}
