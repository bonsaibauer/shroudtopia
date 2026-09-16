#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shroudtopia/api.h"
#include <nlohmann/json.hpp>

#include <cstring>
#include <string>
#include <vector>

namespace {
constexpr char ModId[] = "mod.unlock-blueprints";
constexpr char RecipeType[] = "keen::RecipeRegistryResource";
constexpr std::int64_t UnlockKnowledgeId = 1715248921;

const ST_AssetsApiV1* Assets = nullptr;
size_t Visited = 0;

ST_StringView View(const char* value) { return {value, std::strlen(value)}; }
ST_StringView View(const std::string& value) { return {value.data(), value.size()}; }

ST_Result ST_CALL UnlockRegistry(const ST_AssetResourceKeyV1* resource, void*) {
    if (resource == nullptr || resource->struct_size < sizeof(ST_AssetResourceKeyV1)) return ST_RESULT_INVALID_ARGUMENT;
    ++Visited;
    size_t required = 0;
    auto result = Assets->read_resource_json(View(ModId), resource, nullptr, 0, &required);
    if (result != ST_RESULT_OK) return result;
    std::vector<char> bytes(required);
    result = Assets->read_resource_json(View(ModId), resource, bytes.data(), bytes.size(), &required);
    if (result != ST_RESULT_OK) return result;

    try {
        auto data = nlohmann::json::parse(bytes.begin(), bytes.end());
        if (!data.contains("recipes") || !data["recipes"].is_array()) return ST_RESULT_OK;
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
        if (!changed) return ST_RESULT_OK;
        const auto json = data.dump();
        return Assets->replace_resource_json(View(ModId), resource, View(json));
    } catch (...) {
        return ST_RESULT_CALLBACK_FAILED;
    }
}

ST_Result ST_CALL Load(const ST_HostApiV1* host, void*) {
    if (host == nullptr || host->abi_version != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    const ST_ServiceRequest request{
        sizeof(request), View(ST_ASSETS_SERVICE_ID),
        ST_ASSETS_SERVICE_VERSION_MAJOR, ST_ASSETS_SERVICE_VERSION_MINOR
    };
    const void* service = nullptr;
    const auto found = host->find_service(&request, &service);
    if (found != ST_RESULT_OK) return found;
    Assets = static_cast<const ST_AssetsApiV1*>(service);
    return Assets != nullptr && Assets->struct_size >= sizeof(ST_AssetsApiV1)
        ? ST_RESULT_OK : ST_RESULT_VERSION_MISMATCH;
}

ST_Result ST_CALL Activate(const ST_HostApiV1* host, void*) {
    if (Assets == nullptr || host == nullptr) return ST_RESULT_NOT_FOUND;
    Visited = 0;
    const auto result = Assets->visit_resources(View(ModId), View(RecipeType), UnlockRegistry, nullptr);
    if (result != ST_RESULT_OK) return result;
    if (Visited == 0) return ST_RESULT_NOT_FOUND;
    return Assets->flush(View(ModId));
}

ST_Result ST_CALL Update(const ST_HostApiV1*, void*, double) { return ST_RESULT_OK; }

ST_Result ST_CALL Deactivate(const ST_HostApiV1*, void*) {
    if (Assets == nullptr) return ST_RESULT_OK;
    const auto discarded = Assets->discard_changes(View(ModId));
    if (discarded == ST_RESULT_NOT_FOUND) return ST_RESULT_OK;
    if (discarded != ST_RESULT_OK) return discarded;
    return Assets->flush(View(ModId));
}

ST_Result ST_CALL Unload(const ST_HostApiV1* host, void*) {
    if (host == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    Assets = nullptr;
    return host->release_owner(View(ModId));
}
}

extern "C" __declspec(dllexport) ST_Result ST_CALL ShroudtopiaCreateModV1(
    uint32_t abi, ST_ModDescriptorV1* descriptor) {
    if (abi != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    if (descriptor == nullptr || descriptor->struct_size < sizeof(ST_ModDescriptorV1)) return ST_RESULT_INVALID_ARGUMENT;
    *descriptor = {sizeof(ST_ModDescriptorV1), View(ModId), nullptr,
        Load, Activate, Update, Deactivate, Unload};
    return ST_RESULT_OK;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(module);
    return TRUE;
}
