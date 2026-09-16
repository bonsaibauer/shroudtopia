#include <windows.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "shroudtopia/api.h"

namespace {
std::string Copy(ST_StringView value) {
    return value.data == nullptr ? std::string{} : std::string(value.data, value.size);
}

struct ExpectedPatch {
    const char* signature;
    std::int64_t match_offset;
    ST_RuntimePatchKindV1 kind;
    size_t overwrite_size;
    std::vector<std::uint8_t> payload;
    size_t return_offset;
};

const std::unordered_map<std::string, ExpectedPatch> ExpectedPatches{
    {"mod.flight", {"F3 0F 10 05 ?? ?? ?? ?? F2 0F 11 4C", 0, ST_RUNTIME_PATCH_DETOUR, 8,
        {0xF3,0x0F,0x10,0x05,0x05,0,0,0,0xE9,0,0,0,0,0xC3,0xF5,0xC8,0xBF}, 9}},
    {"mod.no-stamina-loss", {"8B 04 91 89 44 24 40", 0, ST_RUNTIME_PATCH_DETOUR, 7,
        {0x53,0x8B,0x5C,0x91,0x08,0x89,0x1C,0x91,0x5B,0x8B,0x04,0x91,0x89,0x44,0x24,0x40,0xE9,0,0,0,0}, 17}},
    {"mod.no-fall-damage", {"45 89 0C 88 48 8B CB", 0, ST_RUNTIME_PATCH_DETOUR, 7,
        {0x45,0x01,0x0C,0x88,0x48,0x8B,0xCB,0xE9,0,0,0,0}, 8}},
    {"mod.infinite-item-use", {
        "0F B6 85 ?? ?? 00 00 48 8D 4C 24 40 88 44 24 30 44 8B CE 8B 85 ?? ?? 00 00 4C 8B C3",
        0, ST_RUNTIME_PATCH_DIRECT, 7, {0xB8,0,0,0,0,0x90,0x90}, 0}},
    {"mod.no-resource-cost", {"44 8B A5 08 01 00 00 4C 8B F9", 0,
        ST_RUNTIME_PATCH_DETOUR, 7, {0x41,0xBC,0,0,0,0,0xE9,0,0,0,0}, 7}},
    {"mod.infinite-item-split", {"29 6E 04 EB C5", 0, ST_RUNTIME_PATCH_DIRECT, 3,
        {0x90,0x90,0x90}, 0}}
};

struct State {
    std::string expected_owner;
    ExpectedPatch captured_patch{};
    bool patch_created = false;
    bool enabled = false;
    bool patch_released = false;
    bool owner_released = false;
    size_t settings_registered = 0;
    size_t registrations_released = 0;
    std::vector<ST_CommandDescriptor> commands;
    std::string executed_owner;
    std::string executed_command;
    std::string executed_arguments;
    size_t log_count = 0;
    size_t assets_visited = 0;
    size_t assets_replaced = 0;
    size_t assets_discarded = 0;
    size_t assets_flushed = 0;
    std::string replaced_json;
} state;
bool SimulateMissingSignature = false;
bool textWindow = false;
size_t textUpdates = 0;
ST_Result ST_CALL TextCreate(ST_StringView owner, const ST_TextWindowDescriptorV1* d, ST_TextWindow* out) {
    if (Copy(owner) != state.expected_owner || !d || d->tab_count != 2 || d->toggle_key != 121 ||
        Copy(d->tabs[0]) != "Enshrouded Log" || Copy(d->tabs[1]) != "Shroudtopia Debug Log") return ST_RESULT_INVALID_ARGUMENT;
    textWindow = true; *out = 1; return ST_RESULT_OK;
}
ST_Result ST_CALL TextSet(ST_StringView, ST_TextWindow id, size_t tab, ST_StringView text) {
    if (!textWindow || id != 1 || tab > 1 || Copy(text) != (tab == 0 ? "game log" : "loader log")) return ST_RESULT_INVALID_ARGUMENT;
    ++textUpdates; return ST_RESULT_OK;
}
ST_Result ST_CALL TextStatus(ST_StringView, ST_TextWindow, ST_TextWindowStatusV1* out) { *out = ST_TEXT_READY; return ST_RESULT_OK; }
ST_Result ST_CALL TextDestroy(ST_StringView, ST_TextWindow) { textWindow = false; return ST_RESULT_OK; }
ST_Result ST_CALL ReadLog(ST_LogSourceV1 source, char* buffer, size_t capacity, size_t* written) {
    const std::string text = source == ST_LOG_SOURCE_GAME ? "game log" : "loader log";
    if (capacity < text.size()) return ST_RESULT_INVALID_ARGUMENT;
    std::memcpy(buffer, text.data(), text.size()); *written = text.size(); return ST_RESULT_OK;
}
const ST_UiTextApiV1 TextApi{sizeof(TextApi), ST_ABI_VERSION_1, TextCreate, TextSet, TextStatus, TextDestroy};
const ST_LogReadApiV1 LogApi{sizeof(LogApi), ST_ABI_VERSION_1, ReadLog};

ST_Result ST_CALL PatchCreate(ST_StringView owner, const ST_RuntimePatchDescriptorV1* descriptor, ST_RuntimePatch* patch) {
    if (Copy(owner) != state.expected_owner || descriptor == nullptr || patch == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    if (SimulateMissingSignature) {
        *patch = 0;
        return ST_RESULT_NOT_FOUND;
    }
    state.captured_patch.signature = nullptr;
    state.captured_patch.match_offset = descriptor->match_offset;
    state.captured_patch.kind = descriptor->kind;
    state.captured_patch.overwrite_size = descriptor->overwrite_size;
    state.captured_patch.payload.assign(descriptor->payload, descriptor->payload + descriptor->payload_size);
    state.captured_patch.return_offset = descriptor->relocation_count == 0 ? 0 : descriptor->relocations[0].payload_offset;
    static std::string signature;
    signature = Copy(descriptor->signature);
    state.captured_patch.signature = signature.c_str();
    state.patch_created = true;
    *patch = 77;
    return ST_RESULT_OK;
}

ST_Result ST_CALL PatchEnable(ST_StringView owner, ST_RuntimePatch patch, std::uint8_t enabled) {
    if (Copy(owner) != state.expected_owner || patch != 77) return ST_RESULT_NOT_FOUND;
    state.enabled = enabled != 0;
    return ST_RESULT_OK;
}

ST_Result ST_CALL PatchState(ST_StringView owner, ST_RuntimePatch patch, ST_RuntimePatchStateV1* value) {
    if (Copy(owner) != state.expected_owner || patch != 77 || value == nullptr) return ST_RESULT_NOT_FOUND;
    value->enabled = state.enabled ? 1 : 0;
    return ST_RESULT_OK;
}

ST_Result ST_CALL PatchRelease(ST_StringView owner, ST_RuntimePatch patch) {
    if (Copy(owner) != state.expected_owner || patch != 77) return ST_RESULT_NOT_FOUND;
    state.patch_released = true;
    state.enabled = false;
    return ST_RESULT_OK;
}

const ST_RuntimePatchesApiV1 RuntimeApi{
    sizeof(RuntimeApi), PatchCreate, PatchEnable, PatchState, PatchRelease
};

ST_Result ST_CALL AssetsVisit(ST_StringView owner, ST_StringView typeName,
    ST_AssetResourceVisitorV1 visitor, void* userData) {
    if (Copy(owner) != state.expected_owner || Copy(typeName) != "keen::RecipeRegistryResource" || visitor == nullptr) {
        return ST_RESULT_INVALID_ARGUMENT;
    }
    if (SimulateMissingSignature) return ST_RESULT_NOT_FOUND;
    ++state.assets_visited;
    const ST_AssetResourceKeyV1 key{
        sizeof(key), {"00000000-0000-0000-0000-000000000001", 36}, typeName, 0
    };
    return visitor(&key, userData);
}
ST_Result ST_CALL AssetsRead(ST_StringView owner, const ST_AssetResourceKeyV1*,
    char* buffer, size_t capacity, size_t* required) {
    static constexpr char Json[] = R"({"recipes":[{"knowledgeRequirement":{"knowledgeOrQueryId":{"value":1},"compareValue":0,"compareOperator":"NotEquals","type":"Counter","isExplicitPlayerKnowledgeQuery":true}}]})";
    if (Copy(owner) != state.expected_owner || required == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    *required = sizeof(Json) - 1;
    if (capacity == 0) return ST_RESULT_OK;
    if (buffer == nullptr || capacity < sizeof(Json) - 1) return ST_RESULT_INVALID_ARGUMENT;
    std::memcpy(buffer, Json, sizeof(Json) - 1);
    return ST_RESULT_OK;
}
ST_Result ST_CALL AssetsReplace(ST_StringView owner, const ST_AssetResourceKeyV1*, ST_StringView json) {
    if (Copy(owner) != state.expected_owner) return ST_RESULT_INVALID_ARGUMENT;
    state.replaced_json = Copy(json);
    ++state.assets_replaced;
    return ST_RESULT_OK;
}
ST_Result ST_CALL AssetsCreate(ST_StringView, ST_StringView, ST_StringView,
    ST_AssetResourceVisitorV1, void*) { return ST_RESULT_UNSUPPORTED; }
ST_Result ST_CALL AssetsDiscard(ST_StringView owner) {
    if (Copy(owner) != state.expected_owner) return ST_RESULT_INVALID_ARGUMENT;
    ++state.assets_discarded;
    return ST_RESULT_OK;
}
ST_Result ST_CALL AssetsFlush(ST_StringView owner) {
    if (Copy(owner) != state.expected_owner) return ST_RESULT_INVALID_ARGUMENT;
    ++state.assets_flushed;
    return ST_RESULT_OK;
}
const ST_AssetsApiV1 AssetsApi{
    sizeof(AssetsApi), AssetsVisit, AssetsRead, AssetsReplace,
    AssetsCreate, AssetsDiscard, AssetsFlush
};

ST_Result ST_CALL RegisterService(ST_StringView, const ST_ServiceDescriptor*, ST_Registration*) { return ST_RESULT_OK; }
ST_Result ST_CALL FindService(const ST_ServiceRequest* request, const void** service) {
    if (request == nullptr || service == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    const auto contract = Copy(request->contract_id);
    if (contract == ST_RUNTIME_PATCHES_SERVICE_ID) *service = &RuntimeApi;
    else if (contract == ST_ASSETS_SERVICE_ID) *service = &AssetsApi;
    else if (contract == ST_UI_TEXT_SERVICE_ID && !SimulateMissingSignature) *service = &TextApi;
    else if (contract == ST_LOG_READ_SERVICE_ID && !SimulateMissingSignature) *service = &LogApi;
    else return ST_RESULT_NOT_FOUND;
    return ST_RESULT_OK;
}
ST_Result ST_CALL SubscribeEvent(ST_StringView, const ST_EventSubscription*, ST_Registration*) { return ST_RESULT_OK; }
ST_Result ST_CALL PublishEvent(ST_StringView, const ST_Event*) { return ST_RESULT_OK; }
ST_Result ST_CALL RegisterCommand(ST_StringView owner, const ST_CommandDescriptor* descriptor, ST_Registration* registration) {
    if (Copy(owner) != state.expected_owner || descriptor == nullptr || registration == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    state.commands.push_back(*descriptor);
    *registration = static_cast<ST_Registration>(100 + state.commands.size());
    return ST_RESULT_OK;
}
ST_Result ST_CALL ExecuteCommand(ST_StringView owner, ST_StringView command, ST_StringView arguments) {
    state.executed_owner = Copy(owner);
    state.executed_command = Copy(command);
    state.executed_arguments = Copy(arguments);
    return ST_RESULT_OK;
}
ST_Result ST_CALL RegisterSettings(ST_StringView owner, const ST_SettingsDescriptor*, ST_Registration* registration) {
    if (Copy(owner) != state.expected_owner || registration == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    *registration = 90;
    ++state.settings_registered;
    return ST_RESULT_OK;
}
ST_Result ST_CALL ReleaseRegistration(ST_Registration) { ++state.registrations_released; return ST_RESULT_OK; }
ST_Result ST_CALL ReleaseOwner(ST_StringView owner) {
    if (Copy(owner) != state.expected_owner) return ST_RESULT_INVALID_ARGUMENT;
    state.owner_released = true;
    return ST_RESULT_OK;
}
ST_Result ST_CALL QueryCapability(ST_StringView, ST_CapabilityInfoV1*) { return ST_RESULT_NOT_FOUND; }
ST_Result ST_CALL CheckPermission(ST_StringView, ST_StringView, std::uint8_t* allowed) {
    if (allowed != nullptr) *allowed = 1;
    return ST_RESULT_OK;
}
ST_Result ST_CALL Log(ST_StringView owner, ST_LogLevel, ST_StringView) {
    if (Copy(owner) != state.expected_owner) return ST_RESULT_INVALID_ARGUMENT;
    ++state.log_count;
    return ST_RESULT_OK;
}
ST_Result ST_CALL GetSettingBool(ST_StringView, ST_StringView, std::uint8_t fallback, std::uint8_t* value) {
    if (value == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    *value = fallback;
    return ST_RESULT_OK;
}

const ST_HostApiV1 HostApi{
    sizeof(HostApi), ST_ABI_VERSION_1,
    RegisterService, FindService, SubscribeEvent, PublishEvent,
    RegisterCommand, ExecuteCommand, RegisterSettings,
    ReleaseRegistration, ReleaseOwner, QueryCapability, CheckPermission,
    Log, GetSettingBool
};

bool SamePatch(const ExpectedPatch& actual, const ExpectedPatch& expected) {
    return std::string_view(actual.signature == nullptr ? "" : actual.signature) == expected.signature &&
        actual.match_offset == expected.match_offset && actual.kind == expected.kind &&
        actual.overwrite_size == expected.overwrite_size && actual.payload == expected.payload &&
        actual.return_offset == expected.return_offset;
}

int Fail(const std::string& message) {
    std::cerr << message << '\n';
    return 1;
}
}

int wmain(int argc, wchar_t** argv) {
    if (argc != 3 && argc != 4) return Fail("usage: native-mod-smoke <dll> <mod-id> [missing]");
    const std::wstring dll = argv[1];
    const int required = WideCharToMultiByte(CP_UTF8, 0, argv[2], -1, nullptr, 0, nullptr, nullptr);
    std::string id(static_cast<size_t>(required), '\0');
    WideCharToMultiByte(CP_UTF8, 0, argv[2], -1, id.data(), required, nullptr, nullptr);
    id.pop_back();
    state.expected_owner = id;
    SimulateMissingSignature = argc == 4 && std::wstring_view(argv[3]) == L"missing";

    HMODULE module = LoadLibraryW(dll.c_str());
    if (module == nullptr) return Fail("could not load mod DLL");
    const auto create = reinterpret_cast<ST_CreateModV1>(GetProcAddress(module, "ShroudtopiaCreateModV1"));
    if (create == nullptr) return Fail("entrypoint is missing");
    ST_ModDescriptorV1 descriptor{sizeof(descriptor)};
    if (create(ST_ABI_VERSION_1, &descriptor) != ST_RESULT_OK || Copy(descriptor.mod_id) != id) return Fail("descriptor ID mismatch");
    if (descriptor.on_load == nullptr || descriptor.on_activate == nullptr || descriptor.on_update == nullptr ||
        descriptor.on_deactivate == nullptr || descriptor.on_unload == nullptr) return Fail("lifecycle callback is missing");
    if (descriptor.on_load(&HostApi, descriptor.user_data) != ST_RESULT_OK) return Fail("load failed");

    if (id == "mod.commands") {
        if (state.settings_registered != 1) return Fail("Commands settings registration missing");
        if (descriptor.on_activate(&HostApi, descriptor.user_data) != ST_RESULT_OK || state.commands.size() != 2) return Fail("Commands activation failed");
        const auto execute = state.commands[0];
        const char input[] = "mod.test payload";
        if (execute.callback({input, sizeof(input) - 1}, execute.user_data) != ST_RESULT_OK ||
            state.executed_owner != id || state.executed_command != "mod.test" || state.executed_arguments != "payload") {
            return Fail("Commands routing failed");
        }
        if (descriptor.on_update(&HostApi, descriptor.user_data, 0.5) != ST_RESULT_OK) return Fail("Commands update failed");
        if (descriptor.on_deactivate(&HostApi, descriptor.user_data) != ST_RESULT_OK || state.registrations_released != 2) return Fail("Commands deactivation failed");
    } else if (id == "mod.debug-console") {
        const auto activation = descriptor.on_activate(&HostApi, nullptr);
        if (SimulateMissingSignature) {
            if (activation != ST_RESULT_NOT_FOUND || textWindow) return Fail("missing UI service not handled");
        } else {
            if (activation != ST_RESULT_OK || !textWindow) return Fail("debug window not created");
            if (descriptor.on_update(&HostApi, nullptr, 0.5) != ST_RESULT_OK || textUpdates != 2) return Fail("log sources not forwarded");
            if (descriptor.on_deactivate(&HostApi, nullptr) != ST_RESULT_OK || textWindow) return Fail("debug window not destroyed");
            if (descriptor.on_activate(&HostApi, nullptr) != ST_RESULT_OK || !textWindow) return Fail("debug window not recreated");
        }
    } else if (id == "mod.unlock-blueprints") {
        const auto activation = descriptor.on_activate(&HostApi, descriptor.user_data);
        if (SimulateMissingSignature) {
            if (activation != ST_RESULT_NOT_FOUND || state.log_count != 0 || state.assets_replaced != 0) {
                return Fail("unavailable asset system was not reported");
            }
        } else {
            if (activation != ST_RESULT_OK || state.assets_visited != 1 || state.assets_replaced != 1 ||
                state.assets_flushed != 1 || state.replaced_json.find("1715248921") == std::string::npos ||
                state.replaced_json.find("SimpleBool") == std::string::npos) {
                return Fail("Unlock Blueprints asset transformation failed");
            }
        }
        if (descriptor.on_update(&HostApi, descriptor.user_data, 0.5) != ST_RESULT_OK) return Fail("asset mod update failed");
        if (descriptor.on_deactivate(&HostApi, descriptor.user_data) != ST_RESULT_OK) return Fail("asset mod deactivation failed");
        if (!SimulateMissingSignature && (state.assets_discarded != 1 || state.assets_flushed != 2)) {
            return Fail("asset changes were not discarded");
        }
    } else {
        const auto expected = ExpectedPatches.find(id);
        if (expected == ExpectedPatches.end()) return Fail("unknown bundled mod");
        if (SimulateMissingSignature) {
            if (state.patch_created || state.log_count != 1) return Fail("missing signature was not reported");
            if (descriptor.on_activate(&HostApi, descriptor.user_data) != ST_RESULT_NOT_FOUND) return Fail("unavailable patch activated successfully");
        } else {
            if (!state.patch_created || !SamePatch(state.captured_patch, expected->second)) return Fail("patch contract mismatch");
            if (descriptor.on_activate(&HostApi, descriptor.user_data) != ST_RESULT_OK || !state.enabled) return Fail("patch activation failed");
        }
        if (descriptor.on_update(&HostApi, descriptor.user_data, 0.5) != ST_RESULT_OK) return Fail("patch update failed");
        if (descriptor.on_deactivate(&HostApi, descriptor.user_data) != ST_RESULT_OK || state.enabled) return Fail("patch deactivation failed");
    }
    if (descriptor.on_unload(&HostApi, descriptor.user_data) != ST_RESULT_OK || !state.owner_released) return Fail("unload failed");
    if (textWindow) return Fail("debug window leaked on unload");
    if (id != "mod.commands" && id != "mod.unlock-blueprints" && id != "mod.debug-console" &&
        state.patch_released == SimulateMissingSignature) return Fail("patch release state is invalid");
    FreeLibrary(module);
    std::cout << id << " contract and lifecycle passed.\n";
    return 0;
}
