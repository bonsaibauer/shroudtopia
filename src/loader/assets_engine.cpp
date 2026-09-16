#include "pch.h"
#include "assets_engine.h"
#include "utils.h"

#include <filesystem>
#include <mutex>
#include <string>

namespace {
using OpenFn = ST_Result (ST_CALL*)(ST_StringView, ST_StringView);
using CloseFn = void (ST_CALL*)();
using VisitFn = ST_Result (ST_CALL*)(ST_StringView, ST_AssetResourceVisitorV1, void*);
using ReadFn = ST_Result (ST_CALL*)(ST_StringView, const ST_AssetResourceKeyV1*, char*, size_t, size_t*);
using ReplaceFn = ST_Result (ST_CALL*)(ST_StringView, const ST_AssetResourceKeyV1*, ST_StringView);
using SetFieldFn = ST_Result (ST_CALL*)(ST_StringView, const ST_AssetResourceKeyV1*, ST_StringView, ST_StringView);
using CreateFn = ST_Result (ST_CALL*)(ST_StringView, ST_StringView, ST_StringView, ST_AssetResourceVisitorV1, void*);
using DiscardFn = ST_Result (ST_CALL*)(ST_StringView);
using FlushFn = ST_Result (ST_CALL*)();

struct Functions {
    HMODULE module{};
    OpenFn open{};
    CloseFn close{};
    VisitFn visit{};
    ReadFn read{};
    ReplaceFn replace{};
    CreateFn create{};
    SetFieldFn setField{};
    DiscardFn discard{};
    FlushFn flush{};
    bool available{};
};

Functions functions;
std::once_flag initializeOnce;

ST_StringView View(const std::string& value) { return {value.data(), value.size()}; }

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
    const auto libraryPath = ModulePath().parent_path() / L"shroudtopia-assets.dll";
    functions.module = LoadLibraryW(libraryPath.c_str());
    if (functions.module == nullptr) {
        Utils::Log(Utils::DEBUG, "Asset engine library unavailable: %s", Utf8(libraryPath).c_str());
        return;
    }
    functions.open = Function<OpenFn>("ShroudtopiaAssetsOpen");
    functions.close = Function<CloseFn>("ShroudtopiaAssetsClose");
    functions.visit = Function<VisitFn>("ShroudtopiaAssetsVisit");
    functions.read = Function<ReadFn>("ShroudtopiaAssetsReadJson");
    functions.replace = Function<ReplaceFn>("ShroudtopiaAssetsReplaceJson");
    functions.setField = Function<SetFieldFn>("ShroudtopiaAssetsSetFieldJson");
    functions.create = Function<CreateFn>("ShroudtopiaAssetsCreateJson");
    functions.discard = Function<DiscardFn>("ShroudtopiaAssetsDiscard");
    functions.flush = Function<FlushFn>("ShroudtopiaAssetsFlush");
    if (functions.open == nullptr || functions.close == nullptr || functions.visit == nullptr ||
        functions.read == nullptr || functions.replace == nullptr || functions.create == nullptr ||
        functions.discard == nullptr || functions.flush == nullptr || functions.setField == nullptr) {
        Utils::Log(Utils::DEBUG, "Asset engine rejected: required exports are incomplete");
        return;
    }

    const auto executable = ProcessPath();
    const auto directory = Utf8(executable.parent_path());
    const auto stem = Utf8(executable.stem());
    functions.available = functions.open(View(directory), View(stem)) == ST_RESULT_OK;
    Utils::Log(Utils::DEBUG, "Asset engine initialize: executable=%s available=%s",
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

ST_Result Visit(ST_StringView typeName, ST_AssetResourceVisitorV1 visitor, void* userData) {
    return Available() ? functions.visit(typeName, visitor, userData) : ST_RESULT_NOT_FOUND;
}
ST_Result ReadJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource,
    char* buffer, size_t capacity, size_t* requiredSize) {
    return Available() ? functions.read(owner, resource, buffer, capacity, requiredSize) : ST_RESULT_NOT_FOUND;
}
ST_Result ReplaceJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource, ST_StringView json) {
    return Available() ? functions.replace(owner, resource, json) : ST_RESULT_NOT_FOUND;
}
ST_Result SetFieldJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource, ST_StringView path, ST_StringView json) {
    return Available() ? functions.setField(owner, resource, path, json) : ST_RESULT_NOT_FOUND;
}
ST_Result CreateJson(ST_StringView owner, ST_StringView typeName, ST_StringView json,
    ST_AssetResourceVisitorV1 visitor, void* userData) {
    return Available() ? functions.create(owner, typeName, json, visitor, userData) : ST_RESULT_NOT_FOUND;
}
ST_Result Discard(ST_StringView owner) {
    return Available() ? functions.discard(owner) : ST_RESULT_NOT_FOUND;
}
ST_Result Flush() { return Available() ? functions.flush() : ST_RESULT_NOT_FOUND; }
}
