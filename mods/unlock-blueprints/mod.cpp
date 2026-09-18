#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shroudtopia.h"
#include <nlohmann/json.hpp>

#include <cstring>
#include <string>
#include <vector>

namespace {
constexpr char ModId[] = "mod.unlock-blueprints";
constexpr char RecipeType[] = "keen::RecipeRegistryResource";
constexpr std::int64_t UnlockKnowledgeId = 1715248921;

const Api* Assets = nullptr;
size_t Visited = 0;

StringView View(const char* value) { return {value, std::strlen(value)}; }
StringView View(const std::string& value) { return {value.data(), value.size()}; }

Result CALL UnlockRegistry(const AssetId* asset, void*) {
    if (asset == nullptr || asset->struct_size < sizeof(AssetId)) return RESULT_INVALID_ARGUMENT;
    ++Visited;
    size_t required = 0;
    auto result = Assets->get_asset(View(ModId), asset, nullptr, 0, &required);
    if (result != RESULT_OK) return result;
    std::vector<char> bytes(required);
    result = Assets->get_asset(View(ModId), asset, bytes.data(), bytes.size(), &required);
    if (result != RESULT_OK) return result;

    try {
        auto data = nlohmann::json::parse(bytes.begin(), bytes.end());
        if (!data.contains("recipes") || !data["recipes"].is_array()) return RESULT_OK;
        bool changed = false;
        for (auto& recipe : data["recipes"]) {
            if (!recipe.is_object() || !recipe.contains("knowledgeRequirement") ||
                !recipe["knowledgeRequirement"].is_object()) continue;
            auto& requirement = recipe["knowledgeRequirement"];
            if (!requirement.contains("knowledgeOrQueryId") || !requirement["knowledgeOrQueryId"].is_object()) continue;
            auto& knowledge = requirement["knowledgeOrQueryId"]["value"];
            changed = changed || knowledge != UnlockKnowledgeId;
            knowledge = UnlockKnowledgeId;
            changed = changed || requirement.value("compareValue", 0) != 1;
            requirement["compareValue"] = 1;
            changed = changed || requirement.value("compareOperator", "") != "Equals";
            requirement["compareOperator"] = "Equals";
            changed = changed || requirement.value("type", "") != "SimpleBool";
            requirement["type"] = "SimpleBool";
            changed = changed || requirement.value("isExplicitPlayerKnowledgeQuery", true);
            requirement["isExplicitPlayerKnowledgeQuery"] = false;
        }
        if (!changed) return RESULT_OK;
        const auto json = data.dump();
        return Assets->update_asset(View(ModId), asset, View(json));
    } catch (...) {
        return RESULT_CALLBACK_FAILED;
    }
}

Result CALL Load(const Api* api, void*) {
    if (api == nullptr || api->api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    Assets = api;
    return RESULT_OK;
}

Result CALL Activate(const Api* api, void*) {
    if (Assets == nullptr || api == nullptr) return RESULT_NOT_FOUND;
    Visited = 0;
    const auto result = Assets->list_assets(View(ModId), View(RecipeType), UnlockRegistry, nullptr);
    if (result != RESULT_OK) return result;
    if (Visited == 0) return RESULT_NOT_FOUND;
    return Assets->save_assets(View(ModId));
}

Result CALL Deactivate(const Api*, void*) {
    if (Assets == nullptr) return RESULT_OK;
    const auto discarded = Assets->reset_assets(View(ModId));
    if (discarded == RESULT_NOT_FOUND) return RESULT_OK;
    if (discarded != RESULT_OK) return discarded;
    return Assets->save_assets(View(ModId));
}

Result CALL Unload(const Api*, void*) {
    Assets = nullptr;
    return RESULT_OK;
}
}

extern "C" __declspec(dllexport) Result CALL CreateMod(
    uint32_t api_version, ModDescriptor* descriptor) {
    if (api_version != API_VERSION) return RESULT_VERSION_MISMATCH;
    if (descriptor == nullptr || descriptor->struct_size < sizeof(ModDescriptor)) return RESULT_INVALID_ARGUMENT;
    *descriptor = {sizeof(ModDescriptor), View(ModId), nullptr,
        Load, Activate, nullptr, Deactivate, Unload};
    return RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}
