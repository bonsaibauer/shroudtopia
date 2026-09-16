#include "pch.h"
#include "config.h"
#include "platform_api.h"
#include "runtime_patches.h"
#include "assets_engine.h"
#include "utils.h"
#include "ui_text.h"
#include "shroudtopia/api.h"
#include "shroudtopia/capabilities.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
    bool Valid(ST_StringView value) {
        return value.data != nullptr && value.size != 0;
    }

    std::string Copy(ST_StringView value) {
        return Valid(value) ? std::string(value.data, value.size) : std::string();
    }

    struct ServiceEntry {
        ST_Registration handle;
        std::string owner;
        std::string contract;
        uint32_t major;
        uint32_t minor;
        const void* interfacePointer;
    };

    struct EventEntry {
        ST_Registration handle;
        std::string owner;
        std::string eventId;
        ST_EventCallback callback;
        void* userData;
    };

    struct CommandEntry {
        ST_Registration handle;
        std::string owner;
        std::string commandId;
        std::string description;
        ST_CommandCallback callback;
        void* userData;
    };

    struct SettingsEntry {
        ST_Registration handle;
        std::string owner;
        std::string settingsId;
        std::string schema;
        std::string defaults;
    };

    struct CapabilityEntry {
        uint32_t major;
        uint32_t minor;
        uint64_t flags;
        bool available;
    };

    class PlatformRegistry {
    public:
        ST_Result RegisterService(ST_StringView owner, const ST_ServiceDescriptor* descriptor, ST_Registration* registration) {
            if (!Valid(owner) || descriptor == nullptr || registration == nullptr ||
                descriptor->struct_size < sizeof(ST_ServiceDescriptor) || !Valid(descriptor->contract_id) ||
                descriptor->version_major == 0 || descriptor->interface_pointer == nullptr) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            std::scoped_lock lock(mutex_);
            const auto contract = Copy(descriptor->contract_id);
            const auto duplicate = std::find_if(services_.begin(), services_.end(), [&](const auto& pair) {
                return pair.second.contract == contract && pair.second.major == descriptor->version_major;
            });
            if (duplicate != services_.end()) return ST_RESULT_ALREADY_EXISTS;

            const auto handle = NextHandle();
            services_.emplace(handle, ServiceEntry{handle, Copy(owner), contract, descriptor->version_major,
                descriptor->version_minor, descriptor->interface_pointer});
            *registration = handle;
            return ST_RESULT_OK;
        }

        ST_Result FindService(const ST_ServiceRequest* request, const void** interfacePointer) {
            if (request == nullptr || interfacePointer == nullptr || request->struct_size < sizeof(ST_ServiceRequest) ||
                !Valid(request->contract_id) || request->version_major == 0) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            *interfacePointer = nullptr;
            std::scoped_lock lock(mutex_);
            const auto contract = Copy(request->contract_id);
            const ServiceEntry* best = nullptr;
            for (const auto& [handle, service] : services_) {
                if (service.contract == contract && service.major == request->version_major &&
                    service.minor >= request->minimum_minor && (best == nullptr || service.minor > best->minor)) {
                    best = &service;
                }
            }
            if (best == nullptr) return ST_RESULT_NOT_FOUND;
            *interfacePointer = best->interfacePointer;
            return ST_RESULT_OK;
        }

        ST_Result SubscribeEvent(ST_StringView owner, const ST_EventSubscription* subscription, ST_Registration* registration) {
            if (!Valid(owner) || subscription == nullptr || registration == nullptr ||
                subscription->struct_size < sizeof(ST_EventSubscription) || !Valid(subscription->event_id) ||
                subscription->callback == nullptr) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            std::scoped_lock lock(mutex_);
            const auto handle = NextHandle();
            events_.emplace(handle, EventEntry{handle, Copy(owner), Copy(subscription->event_id),
                subscription->callback, subscription->user_data});
            *registration = handle;
            return ST_RESULT_OK;
        }

        ST_Result PublishEvent(ST_StringView owner, const ST_Event* eventData) {
            if (!Valid(owner) || eventData == nullptr || eventData->struct_size < sizeof(ST_Event) ||
                !Valid(eventData->event_id) || (eventData->payload_size != 0 && eventData->payload == nullptr)) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            std::vector<EventEntry> callbacks;
            {
                std::scoped_lock lock(mutex_);
                const auto eventId = Copy(eventData->event_id);
                for (const auto& [handle, event] : events_) {
                    if (event.eventId == eventId) callbacks.push_back(event);
                }
            }

            for (const auto& event : callbacks) {
                try {
                    if (event.callback(eventData, event.userData) != ST_RESULT_OK) return ST_RESULT_CALLBACK_FAILED;
                } catch (...) {
                    return ST_RESULT_CALLBACK_FAILED;
                }
            }
            return ST_RESULT_OK;
        }

        ST_Result RegisterCommand(ST_StringView owner, const ST_CommandDescriptor* descriptor, ST_Registration* registration) {
            if (!Valid(owner) || descriptor == nullptr || registration == nullptr ||
                descriptor->struct_size < sizeof(ST_CommandDescriptor) || !Valid(descriptor->command_id) ||
                descriptor->callback == nullptr) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            std::scoped_lock lock(mutex_);
            const auto commandId = Copy(descriptor->command_id);
            const auto duplicate = std::find_if(commands_.begin(), commands_.end(), [&](const auto& pair) {
                return pair.second.commandId == commandId;
            });
            if (duplicate != commands_.end()) return ST_RESULT_ALREADY_EXISTS;

            const auto handle = NextHandle();
            commands_.emplace(handle, CommandEntry{handle, Copy(owner), commandId, Copy(descriptor->description),
                descriptor->callback, descriptor->user_data});
            *registration = handle;
            return ST_RESULT_OK;
        }

        ST_Result ExecuteCommand(ST_StringView owner, ST_StringView commandId, ST_StringView arguments) {
            if (!Valid(owner) || !Valid(commandId) || (arguments.size != 0 && arguments.data == nullptr)) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            CommandEntry command{};
            {
                std::scoped_lock lock(mutex_);
                const auto id = Copy(commandId);
                const auto entry = std::find_if(commands_.begin(), commands_.end(), [&](const auto& pair) {
                    return pair.second.commandId == id;
                });
                if (entry == commands_.end()) return ST_RESULT_NOT_FOUND;
                command = entry->second;
            }

            try {
                return command.callback(arguments, command.userData);
            } catch (...) {
                return ST_RESULT_CALLBACK_FAILED;
            }
        }

        ST_Result RegisterSettings(ST_StringView owner, const ST_SettingsDescriptor* descriptor, ST_Registration* registration) {
            if (!Valid(owner) || descriptor == nullptr || registration == nullptr ||
                descriptor->struct_size < sizeof(ST_SettingsDescriptor) || !Valid(descriptor->settings_id) ||
                !Valid(descriptor->schema_json) || !Valid(descriptor->defaults_json)) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            std::scoped_lock lock(mutex_);
            const auto settingsId = Copy(descriptor->settings_id);
            const auto duplicate = std::find_if(settings_.begin(), settings_.end(), [&](const auto& pair) {
                return pair.second.settingsId == settingsId;
            });
            if (duplicate != settings_.end()) return ST_RESULT_ALREADY_EXISTS;

            const auto handle = NextHandle();
            settings_.emplace(handle, SettingsEntry{handle, Copy(owner), settingsId, Copy(descriptor->schema_json),
                Copy(descriptor->defaults_json)});
            *registration = handle;
            return ST_RESULT_OK;
        }

        ST_Result Release(ST_Registration registration) {
            if (registration == 0) return ST_RESULT_INVALID_ARGUMENT;
            std::scoped_lock lock(mutex_);
            const auto removed = services_.erase(registration) + events_.erase(registration) +
                commands_.erase(registration) + settings_.erase(registration);
            return removed == 0 ? ST_RESULT_NOT_FOUND : ST_RESULT_OK;
        }

        ST_Result ReleaseOwner(ST_StringView owner) {
            if (!Valid(owner)) return ST_RESULT_INVALID_ARGUMENT;
            const auto ownerId = Copy(owner);
            RuntimePatches::ReleaseOwner(ownerId);
            AssetsEngine::Discard(owner);
            std::scoped_lock lock(mutex_);
            EraseOwner(services_, ownerId);
            EraseOwner(events_, ownerId);
            EraseOwner(commands_, ownerId);
            EraseOwner(settings_, ownerId);
            permissions_.erase(ownerId);
            return ST_RESULT_OK;
        }

        ST_Result QueryCapability(ST_StringView capabilityId, ST_CapabilityInfoV1* information) {
            if (!Valid(capabilityId) || information == nullptr || information->struct_size < sizeof(ST_CapabilityInfoV1)) {
                return ST_RESULT_INVALID_ARGUMENT;
            }

            const auto capability = Copy(capabilityId);
            if (capability == ST_CAPABILITY_ASSETS_READ || capability == ST_CAPABILITY_ASSETS_WRITE) {
                information->version_major = 1;
                information->version_minor = 0;
                information->flags = 0;
                information->available = AssetsEngine::Available() ? 1 : 0;
                std::fill(std::begin(information->reserved), std::end(information->reserved), 0);
                return ST_RESULT_OK;
            }

            static const std::unordered_map<std::string, CapabilityEntry> capabilities{
                {ST_CAPABILITY_LIFECYCLE_NATIVE, {1, 0, 0, true}},
                {ST_CAPABILITY_REGISTRY_SERVICES, {1, 0, 0, true}},
                {ST_CAPABILITY_REGISTRY_EVENTS, {1, 0, 0, true}},
                {ST_CAPABILITY_REGISTRY_COMMANDS, {1, 0, 0, true}},
                {ST_CAPABILITY_REGISTRY_SETTINGS, {1, 0, 0, true}},
                {ST_CAPABILITY_RUNTIME_PATCHES, {1, 0, 0, true}},
                {ST_CAPABILITY_GAME_TARGETING, {1, 0, 0, false}},
                {ST_CAPABILITY_WORLD_ENTITIES_READ, {1, 0, 0, false}},
                {ST_CAPABILITY_WORLD_ENTITIES_WRITE, {1, 0, 0, false}},
                {ST_CAPABILITY_WORLD_VOXELS_READ, {1, 0, 0, false}},
                {ST_CAPABILITY_WORLD_VOXELS_WRITE, {1, 0, 0, false}},
                {ST_CAPABILITY_UI_OVERLAY, {1, 0, 0, false}}
            };

            const auto entry = capabilities.find(capability);
            if (entry == capabilities.end()) return ST_RESULT_NOT_FOUND;
            information->version_major = entry->second.major;
            information->version_minor = entry->second.minor;
            information->flags = entry->second.flags;
            information->available = entry->second.available ? 1 : 0;
            std::fill(std::begin(information->reserved), std::end(information->reserved), 0);
            return ST_RESULT_OK;
        }

        ST_Result CheckPermission(ST_StringView owner, ST_StringView permissionId, uint8_t* allowed) {
            if (!Valid(owner) || !Valid(permissionId) || allowed == nullptr) return ST_RESULT_INVALID_ARGUMENT;
            const auto ownerId = Copy(owner);
            const auto permission = Copy(permissionId);
            std::scoped_lock lock(mutex_);
            const auto grants = permissions_.find(ownerId);
            *allowed = ownerId == "shroudtopia.core" || permission.starts_with("shroudtopia.registry.") ||
                permission == "shroudtopia.lifecycle.native" ||
                (grants != permissions_.end() && grants->second.contains(permission)) ? 1 : 0;
            return ST_RESULT_OK;
        }

        void GrantCapabilities(const std::string& owner, const std::vector<std::string>& capabilities) {
            std::scoped_lock lock(mutex_);
            auto& grants = permissions_[owner];
            grants.clear();
            grants.insert(capabilities.begin(), capabilities.end());
        }

    private:
        ST_Registration NextHandle() { return nextHandle_.fetch_add(1, std::memory_order_relaxed); }

        template <typename Map>
        static void EraseOwner(Map& entries, const std::string& owner) {
            std::erase_if(entries, [&](const auto& pair) { return pair.second.owner == owner; });
        }

        std::mutex mutex_;
        std::atomic<ST_Registration> nextHandle_{1};
        std::unordered_map<ST_Registration, ServiceEntry> services_;
        std::unordered_map<ST_Registration, EventEntry> events_;
        std::unordered_map<ST_Registration, CommandEntry> commands_;
        std::unordered_map<ST_Registration, SettingsEntry> settings_;
        std::unordered_map<std::string, std::unordered_set<std::string>> permissions_;
    };

    PlatformRegistry registry;

    ST_Result ST_CALL RegisterService(ST_StringView owner, const ST_ServiceDescriptor* descriptor, ST_Registration* registration) {
        return registry.RegisterService(owner, descriptor, registration);
    }
    ST_Result ST_CALL FindService(const ST_ServiceRequest* request, const void** interfacePointer) {
        return registry.FindService(request, interfacePointer);
    }
    ST_Result ST_CALL SubscribeEvent(ST_StringView owner, const ST_EventSubscription* subscription, ST_Registration* registration) {
        return registry.SubscribeEvent(owner, subscription, registration);
    }
    ST_Result ST_CALL PublishEvent(ST_StringView owner, const ST_Event* eventData) { return registry.PublishEvent(owner, eventData); }
    ST_Result ST_CALL RegisterCommand(ST_StringView owner, const ST_CommandDescriptor* descriptor, ST_Registration* registration) {
        return registry.RegisterCommand(owner, descriptor, registration);
    }
    ST_Result ST_CALL ExecuteCommand(ST_StringView owner, ST_StringView commandId, ST_StringView arguments) {
        return registry.ExecuteCommand(owner, commandId, arguments);
    }
    ST_Result ST_CALL RegisterSettings(ST_StringView owner, const ST_SettingsDescriptor* descriptor, ST_Registration* registration) {
        return registry.RegisterSettings(owner, descriptor, registration);
    }
    ST_Result ST_CALL ReleaseRegistration(ST_Registration registration) { return registry.Release(registration); }
    ST_Result ST_CALL ReleaseOwner(ST_StringView owner) { UiText::ReleaseOwner(owner); return registry.ReleaseOwner(owner); }
    ST_Result ST_CALL QueryCapability(ST_StringView capabilityId, ST_CapabilityInfoV1* information) {
        return registry.QueryCapability(capabilityId, information);
    }
    ST_Result ST_CALL CheckPermission(ST_StringView owner, ST_StringView permissionId, uint8_t* allowed) {
        return registry.CheckPermission(owner, permissionId, allowed);
    }
    bool HasRuntimePatchPermission(ST_StringView owner) {
        uint8_t allowed = 0;
        return registry.CheckPermission(
            owner,
            {ST_CAPABILITY_RUNTIME_PATCHES, sizeof(ST_CAPABILITY_RUNTIME_PATCHES) - 1},
            &allowed) == ST_RESULT_OK && allowed != 0;
    }
    ST_Result ST_CALL CreateRuntimePatch(
        ST_StringView owner, const ST_RuntimePatchDescriptorV1* descriptor, ST_RuntimePatch* patch) {
        if (!Valid(owner)) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasRuntimePatchPermission(owner)) return ST_RESULT_PERMISSION_DENIED;
        const auto ownerId = Copy(owner);
        const auto result = RuntimePatches::Create(ownerId, descriptor, patch);
        Utils::Log(Utils::DEBUG, "Runtime patch create: owner=%s result=%d handle=%llu kind=%d overwrite=%zu payload=%zu",
            ownerId.c_str(), static_cast<int>(result), static_cast<unsigned long long>(patch ? *patch : 0),
            descriptor ? static_cast<int>(descriptor->kind) : -1,
            descriptor ? descriptor->overwrite_size : 0, descriptor ? descriptor->payload_size : 0);
        return result;
    }
    ST_Result ST_CALL SetRuntimePatchEnabled(ST_StringView owner, ST_RuntimePatch patch, uint8_t enabled) {
        if (!Valid(owner)) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasRuntimePatchPermission(owner)) return ST_RESULT_PERMISSION_DENIED;
        const auto ownerId = Copy(owner);
        const auto result = RuntimePatches::SetEnabled(ownerId, patch, enabled != 0);
        Utils::Log(Utils::DEBUG, "Runtime patch state: owner=%s handle=%llu enabled=%u result=%d",
            ownerId.c_str(), static_cast<unsigned long long>(patch), enabled != 0, static_cast<int>(result));
        return result;
    }
    ST_Result ST_CALL GetRuntimePatchState(
        ST_StringView owner, ST_RuntimePatch patch, ST_RuntimePatchStateV1* state) {
        if (!Valid(owner)) return ST_RESULT_INVALID_ARGUMENT;
        return RuntimePatches::GetState(Copy(owner), patch, state);
    }
    ST_Result ST_CALL ReleaseRuntimePatch(ST_StringView owner, ST_RuntimePatch patch) {
        if (!Valid(owner)) return ST_RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto result = RuntimePatches::Release(ownerId, patch);
        Utils::Log(Utils::DEBUG, "Runtime patch release: owner=%s handle=%llu result=%d",
            ownerId.c_str(), static_cast<unsigned long long>(patch), static_cast<int>(result));
        return result;
    }
    const ST_RuntimePatchesApiV1 runtimePatchesApi{
        sizeof(ST_RuntimePatchesApiV1),
        CreateRuntimePatch,
        SetRuntimePatchEnabled,
        GetRuntimePatchState,
        ReleaseRuntimePatch
    };

    bool HasPermission(ST_StringView owner, const char* capability) {
        uint8_t allowed = 0;
        return Valid(owner) && registry.CheckPermission(owner,
            {capability, std::strlen(capability)}, &allowed) == ST_RESULT_OK && allowed != 0;
    }
    ST_Result ST_CALL VisitAssets(ST_StringView owner, ST_StringView typeName,
        ST_AssetResourceVisitorV1 visitor, void* userData) {
        if (!Valid(owner) || !Valid(typeName) || visitor == nullptr) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, ST_CAPABILITY_ASSETS_READ)) return ST_RESULT_PERMISSION_DENIED;
        return AssetsEngine::Visit(typeName, visitor, userData);
    }
    ST_Result ST_CALL ReadAssetJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource,
        char* buffer, size_t capacity, size_t* requiredSize) {
        if (!Valid(owner) || resource == nullptr || resource->struct_size < sizeof(ST_AssetResourceKeyV1) ||
            !Valid(resource->guid) || !Valid(resource->type_name) || requiredSize == nullptr ||
            (capacity != 0 && buffer == nullptr)) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, ST_CAPABILITY_ASSETS_READ)) return ST_RESULT_PERMISSION_DENIED;
        return AssetsEngine::ReadJson(owner, resource, buffer, capacity, requiredSize);
    }
    ST_Result ST_CALL ReplaceAssetJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource,
        ST_StringView json) {
        if (!Valid(owner) || resource == nullptr || resource->struct_size < sizeof(ST_AssetResourceKeyV1) ||
            !Valid(resource->guid) || !Valid(resource->type_name) || !Valid(json)) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, ST_CAPABILITY_ASSETS_WRITE)) return ST_RESULT_PERMISSION_DENIED;
        return AssetsEngine::ReplaceJson(owner, resource, json);
    }
    ST_Result ST_CALL SetAssetFieldJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource,
        ST_StringView path, ST_StringView json) {
        if (!Valid(owner) || resource == nullptr || resource->struct_size < sizeof(ST_AssetResourceKeyV1) ||
            !Valid(resource->guid) || !Valid(resource->type_name) || !Valid(path) || !Valid(json)) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, ST_CAPABILITY_ASSETS_WRITE)) return ST_RESULT_PERMISSION_DENIED;
        return AssetsEngine::SetFieldJson(owner, resource, path, json);
    }
    ST_Result ST_CALL CreateAssetJson(ST_StringView owner, ST_StringView typeName, ST_StringView json,
        ST_AssetResourceVisitorV1 visitor, void* userData) {
        if (!Valid(owner) || !Valid(typeName) || !Valid(json) || visitor == nullptr) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, ST_CAPABILITY_ASSETS_WRITE)) return ST_RESULT_PERMISSION_DENIED;
        return AssetsEngine::CreateJson(owner, typeName, json, visitor, userData);
    }
    ST_Result ST_CALL DiscardAssetChanges(ST_StringView owner) {
        if (!Valid(owner)) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, ST_CAPABILITY_ASSETS_WRITE)) return ST_RESULT_PERMISSION_DENIED;
        return AssetsEngine::Discard(owner);
    }
    ST_Result ST_CALL FlushAssets(ST_StringView owner) {
        if (!Valid(owner)) return ST_RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, ST_CAPABILITY_ASSETS_WRITE)) return ST_RESULT_PERMISSION_DENIED;
        return AssetsEngine::Flush();
    }
    const ST_AssetsApiV1 assetsApi{
        sizeof(ST_AssetsApiV1),
        VisitAssets,
        ReadAssetJson,
        ReplaceAssetJson,
        CreateAssetJson,
        DiscardAssetChanges,
        FlushAssets,
        SetAssetFieldJson
    };
    ST_Result ST_CALL Log(ST_StringView owner, ST_LogLevel level, ST_StringView message) {
        if (!Valid(owner) || (message.size != 0 && message.data == nullptr)) return ST_RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto text = Copy(message);
        Utils::Log(level >= ST_LOG_ERROR ? Utils::ERRR :
            level == ST_LOG_WARNING ? Utils::WARN :
            level == ST_LOG_DEBUG ? Utils::DEBUG :
            level == ST_LOG_TRACE ? Utils::VERBOSE : Utils::INFO,
            "[%s] %s", ownerId.c_str(), text.c_str());
        return ST_RESULT_OK;
    }
    ST_Result ST_CALL GetSettingBool(ST_StringView owner, ST_StringView key, uint8_t fallback, uint8_t* value) {
        if (!Valid(owner) || !Valid(key) || value == nullptr) return ST_RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto settingKey = Copy(key);
        *value = Config::modGet<bool>(ownerId.c_str(), settingKey.c_str(), fallback != 0) ? 1 : 0;
        return ST_RESULT_OK;
    }
    ST_Result ST_CALL GetSettingNumber(ST_StringView owner, ST_StringView key, double fallback, double* value) {
        if (!Valid(owner) || !Valid(key) || value == nullptr || !std::isfinite(fallback)) return ST_RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto settingKey = Copy(key);
        const auto result = Config::modGet<double>(ownerId.c_str(), settingKey.c_str(), fallback);
        if (!std::isfinite(result)) return ST_RESULT_INVALID_ARGUMENT;
        *value = result;
        return ST_RESULT_OK;
    }
    const ST_HostApiV1 hostApi{
        sizeof(ST_HostApiV1), ST_ABI_VERSION_1,
        RegisterService, FindService,
        SubscribeEvent, PublishEvent,
        RegisterCommand, ExecuteCommand,
        RegisterSettings,
        ReleaseRegistration, ReleaseOwner,
        QueryCapability, CheckPermission,
        Log,
        GetSettingBool,
        GetSettingNumber
    };
}

namespace PlatformApi {
void Initialize() {
    static const ST_LogReadApiV1 logRead{sizeof(logRead), ST_ABI_VERSION_1, Utils::ReadLogTail};
    const ST_ServiceDescriptor readDescriptor{sizeof(ST_ServiceDescriptor),
        {ST_LOG_READ_SERVICE_ID, sizeof(ST_LOG_READ_SERVICE_ID) - 1}, 1, 0, &logRead};
    const ST_ServiceDescriptor textDescriptor{sizeof(ST_ServiceDescriptor),
        {ST_UI_TEXT_SERVICE_ID, sizeof(ST_UI_TEXT_SERVICE_ID) - 1}, 1, 0, UiText::Api()};
    ST_Registration uiRegistration = 0;
    registry.RegisterService({"shroudtopia.core", 15}, &readDescriptor, &uiRegistration);
    registry.RegisterService({"shroudtopia.core", 15}, &textDescriptor, &uiRegistration);
    const ST_ServiceDescriptor runtimeDescriptor{
        sizeof(ST_ServiceDescriptor),
        {ST_RUNTIME_PATCHES_SERVICE_ID, sizeof(ST_RUNTIME_PATCHES_SERVICE_ID) - 1},
        ST_RUNTIME_PATCHES_VERSION_MAJOR,
        ST_RUNTIME_PATCHES_VERSION_MINOR,
        &runtimePatchesApi
    };
    ST_Registration registration = 0;
    const auto result = registry.RegisterService(
        {"shroudtopia.core", sizeof("shroudtopia.core") - 1}, &runtimeDescriptor, &registration);
    if (result != ST_RESULT_OK && result != ST_RESULT_ALREADY_EXISTS) {
        Utils::Log(Utils::ERRR, "Could not register built-in runtime patches service: %d", static_cast<int>(result));
    }
    const ST_ServiceDescriptor assetsDescriptor{
        sizeof(ST_ServiceDescriptor),
        {ST_ASSETS_SERVICE_ID, sizeof(ST_ASSETS_SERVICE_ID) - 1},
        ST_ASSETS_SERVICE_VERSION_MAJOR,
        ST_ASSETS_SERVICE_VERSION_MINOR,
        &assetsApi
    };
    registration = 0;
    const auto assetsResult = registry.RegisterService(
        {"shroudtopia.core", sizeof("shroudtopia.core") - 1}, &assetsDescriptor, &registration);
    if (assetsResult != ST_RESULT_OK && assetsResult != ST_RESULT_ALREADY_EXISTS) {
        Utils::Log(Utils::ERRR, "Could not register built-in assets service: %d", static_cast<int>(assetsResult));
    }
}

void GrantCapabilities(const std::string& owner, const std::vector<std::string>& capabilities) {
    registry.GrantCapabilities(owner, capabilities);
}

void Shutdown() {
    UiText::Shutdown();
    RuntimePatches::Shutdown();
    AssetsEngine::Shutdown();
}
}

extern "C" ST_API ST_Result ST_CALL ShroudtopiaGetApi(uint32_t requestedAbi, const ST_HostApiV1** api) {
    if (api == nullptr) return ST_RESULT_INVALID_ARGUMENT;
    *api = nullptr;
    if (requestedAbi != ST_ABI_VERSION_1) return ST_RESULT_VERSION_MISMATCH;
    PlatformApi::Initialize();
    *api = &hostApi;
    return ST_RESULT_OK;
}
