#include "pch.h"
#include "config.h"
#include "platform_api.h"
#include "runtime_patches.h"
#include "assets_engine.h"
#include "utils.h"
#include "ui_text.h"
#include "shroudtopia.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
    bool Valid(StringView value) {
        return value.data != nullptr && value.size != 0;
    }

    std::string Copy(StringView value) {
        return Valid(value) ? std::string(value.data, value.size) : std::string();
    }

    struct ServiceEntry {
        Registration handle;
        std::string owner;
        std::string contract;
        uint32_t major;
        uint32_t minor;
        const void* interfacePointer;
    };

    struct EventEntry {
        Registration handle;
        std::string owner;
        std::string eventId;
        EventCallback callback;
        void* userData;
    };

    struct CommandEntry {
        Registration handle;
        std::string owner;
        std::string commandId;
        std::string description;
        CommandCallback callback;
        void* userData;
    };

    struct ActionEntry {
        Registration handle;
        std::string owner;
        std::string id;
        std::string title;
        std::string description;
        std::string inputSchema;
        ActionHandler invoke;
        void* userData;
        ActionState state;
    };

    struct CapabilityEntry {
        uint32_t major;
        uint32_t minor;
        uint64_t flags;
        bool available;
    };

    class PlatformRegistry {
    public:
        Result RegisterService(StringView owner, const ServiceDescriptor* descriptor, Registration* registration) {
            if (!Valid(owner) || descriptor == nullptr || registration == nullptr ||
                descriptor->struct_size < sizeof(ServiceDescriptor) || !Valid(descriptor->contract_id) ||
                descriptor->version_major == 0 || descriptor->interface_pointer == nullptr) {
                return RESULT_INVALID_ARGUMENT;
            }

            std::scoped_lock lock(mutex_);
            const auto contract = Copy(descriptor->contract_id);
            const auto duplicate = std::find_if(services_.begin(), services_.end(), [&](const auto& pair) {
                return pair.second.contract == contract && pair.second.major == descriptor->version_major &&
                    pair.second.minor == descriptor->version_minor;
            });
            if (duplicate != services_.end()) return RESULT_CONFLICT;

            const auto handle = NextHandle();
            services_.emplace(handle, ServiceEntry{handle, Copy(owner), contract, descriptor->version_major,
                descriptor->version_minor, descriptor->interface_pointer});
            *registration = handle;
            return RESULT_OK;
        }

        Result FindService(const ServiceRequest* request, const void** interfacePointer) {
            if (request == nullptr || interfacePointer == nullptr || request->struct_size < sizeof(ServiceRequest) ||
                !Valid(request->contract_id) || request->version_major == 0) {
                return RESULT_INVALID_ARGUMENT;
            }

            *interfacePointer = nullptr;
            std::scoped_lock lock(mutex_);
            const auto contract = Copy(request->contract_id);
            for (const auto& [handle, service] : services_) {
                if (service.contract == contract && service.major == request->version_major &&
                    service.minor == request->version_minor) {
                    *interfacePointer = service.interfacePointer;
                    return RESULT_OK;
                }
            }
            return RESULT_NOT_FOUND;
        }

        Result SubscribeEvent(StringView owner, const EventSubscription* subscription, Registration* registration) {
            if (!Valid(owner) || subscription == nullptr || registration == nullptr ||
                subscription->struct_size < sizeof(EventSubscription) || !Valid(subscription->event_id) ||
                subscription->callback == nullptr) {
                return RESULT_INVALID_ARGUMENT;
            }

            std::scoped_lock lock(mutex_);
            const auto handle = NextHandle();
            events_.emplace(handle, EventEntry{handle, Copy(owner), Copy(subscription->event_id),
                subscription->callback, subscription->user_data});
            *registration = handle;
            return RESULT_OK;
        }

        Result PublishEvent(StringView owner, const Event* eventData) {
            if (!Valid(owner) || eventData == nullptr || eventData->struct_size < sizeof(Event) ||
                !Valid(eventData->event_id) || (eventData->payload_size != 0 && eventData->payload == nullptr)) {
                return RESULT_INVALID_ARGUMENT;
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
                    if (event.callback(eventData, event.userData) != RESULT_OK) return RESULT_CALLBACK_FAILED;
                } catch (...) {
                    return RESULT_CALLBACK_FAILED;
                }
            }
            return RESULT_OK;
        }

        Result RegisterCommand(StringView owner, const CommandDescriptor* descriptor, Registration* registration) {
            if (!Valid(owner) || descriptor == nullptr || registration == nullptr ||
                descriptor->struct_size < sizeof(CommandDescriptor) || !Valid(descriptor->command_id) ||
                descriptor->callback == nullptr) {
                return RESULT_INVALID_ARGUMENT;
            }

            std::scoped_lock lock(mutex_);
            const auto commandId = Copy(descriptor->command_id);
            const auto duplicate = std::find_if(commands_.begin(), commands_.end(), [&](const auto& pair) {
                return pair.second.commandId == commandId;
            });
            if (duplicate != commands_.end()) return RESULT_CONFLICT;

            const auto handle = NextHandle();
            commands_.emplace(handle, CommandEntry{handle, Copy(owner), commandId, Copy(descriptor->description),
                descriptor->callback, descriptor->user_data});
            *registration = handle;
            return RESULT_OK;
        }

        Result InvokeCommand(StringView owner, StringView commandId, StringView arguments) {
            if (!Valid(owner) || !Valid(commandId) || (arguments.size != 0 && arguments.data == nullptr)) {
                return RESULT_INVALID_ARGUMENT;
            }

            CommandEntry command{};
            {
                std::scoped_lock lock(mutex_);
                const auto id = Copy(commandId);
                const auto entry = std::find_if(commands_.begin(), commands_.end(), [&](const auto& pair) {
                    return pair.second.commandId == id;
                });
                if (entry == commands_.end()) return RESULT_NOT_FOUND;
                command = entry->second;
            }

            try {
                return command.callback(arguments, command.userData);
            } catch (...) {
                return RESULT_CALLBACK_FAILED;
            }
        }

        Result RegisterAction(StringView owner, const Action* action, Registration* registration) {
            if (!Valid(owner) || action == nullptr || registration == nullptr ||
                action->struct_size < sizeof(Action) || !Valid(action->id) ||
                !Valid(action->title) || action->invoke == nullptr) return RESULT_INVALID_ARGUMENT;

            std::scoped_lock lock(mutex_);
            const auto id = Copy(action->id);
            const auto duplicate = std::find_if(actions_.begin(), actions_.end(), [&](const auto& pair) {
                return pair.second.id == id;
            });
            if (duplicate != actions_.end()) return RESULT_CONFLICT;
            const auto handle = NextHandle();
            actions_.emplace(handle, ActionEntry{handle, Copy(owner), id, Copy(action->title),
                Copy(action->description), Copy(action->input_schema_json), action->invoke,
                action->user_data, ACTION_AVAILABLE});
            *registration = handle;
            return RESULT_OK;
        }

        Result InvokeAction(StringView owner, StringView actionId, StringView inputJson) {
            if (!Valid(owner) || !Valid(actionId) || (inputJson.size != 0 && inputJson.data == nullptr)) {
                return RESULT_INVALID_ARGUMENT;
            }
            ActionEntry action{};
            {
                std::scoped_lock lock(mutex_);
                const auto id = Copy(actionId);
                const auto entry = std::find_if(actions_.begin(), actions_.end(), [&](const auto& pair) {
                    return pair.second.id == id;
                });
                if (entry == actions_.end()) return RESULT_NOT_FOUND;
                if (entry->second.state != ACTION_AVAILABLE) return RESULT_NOT_AVAILABLE;
                action = entry->second;
                entry->second.state = ACTION_RUNNING;
            }
            Result result = RESULT_CALLBACK_FAILED;
            try { result = action.invoke(inputJson, action.userData); }
            catch (...) {}
            {
                std::scoped_lock lock(mutex_);
                const auto entry = actions_.find(action.handle);
                if (entry != actions_.end()) {
                    entry->second.state = result == RESULT_OK ? ACTION_AVAILABLE : ACTION_FAILED;
                }
            }
            return result;
        }

        Result GetActionState(StringView actionId, ActionState* state) {
            if (!Valid(actionId) || state == nullptr) return RESULT_INVALID_ARGUMENT;
            std::scoped_lock lock(mutex_);
            const auto id = Copy(actionId);
            const auto entry = std::find_if(actions_.begin(), actions_.end(), [&](const auto& pair) {
                return pair.second.id == id;
            });
            if (entry == actions_.end()) return RESULT_NOT_FOUND;
            *state = entry->second.state;
            return RESULT_OK;
        }

        Result Release(Registration registration) {
            if (registration == 0) return RESULT_INVALID_ARGUMENT;
            std::scoped_lock lock(mutex_);
            const auto removed = services_.erase(registration) + events_.erase(registration) +
                commands_.erase(registration) + actions_.erase(registration);
            return removed == 0 ? RESULT_NOT_FOUND : RESULT_OK;
        }

        Result ReleaseOwner(StringView owner) {
            if (!Valid(owner)) return RESULT_INVALID_ARGUMENT;
            const auto ownerId = Copy(owner);
            RuntimePatches::ReleaseOwner(ownerId);
            AssetsEngine::Reset(owner);
            std::scoped_lock lock(mutex_);
            EraseOwner(services_, ownerId);
            EraseOwner(events_, ownerId);
            EraseOwner(commands_, ownerId);
            EraseOwner(actions_, ownerId);
            permissions_.erase(ownerId);
            return RESULT_OK;
        }

        Result QueryCapability(StringView capabilityId, CapabilityInfo* information) {
            if (!Valid(capabilityId) || information == nullptr || information->struct_size < sizeof(CapabilityInfo)) {
                return RESULT_INVALID_ARGUMENT;
            }

            const auto capability = Copy(capabilityId);
            if (capability == CAPABILITY_ASSETS_READ || capability == CAPABILITY_ASSETS_WRITE) {
                information->version_major = 1;
                information->version_minor = 1;
                information->flags = 0;
                information->available = AssetsEngine::Available() ? 1 : 0;
                std::fill(std::begin(information->reserved), std::end(information->reserved), 0);
                return RESULT_OK;
            }

            static const std::unordered_map<std::string, CapabilityEntry> capabilities{
                {CAPABILITY_LIFECYCLE_NATIVE, {1, 1, 0, true}},
                {CAPABILITY_REGISTRY_SERVICES, {1, 1, 0, true}},
                {CAPABILITY_REGISTRY_EVENTS, {1, 1, 0, true}},
                {CAPABILITY_REGISTRY_COMMANDS, {1, 1, 0, true}},
                {CAPABILITY_RUNTIME_PATCHES, {1, 1, 0, true}},
            };

            const auto entry = capabilities.find(capability);
            if (entry == capabilities.end()) return RESULT_NOT_FOUND;
            information->version_major = entry->second.major;
            information->version_minor = entry->second.minor;
            information->flags = entry->second.flags;
            information->available = entry->second.available ? 1 : 0;
            std::fill(std::begin(information->reserved), std::end(information->reserved), 0);
            return RESULT_OK;
        }

        Result CheckPermission(StringView owner, StringView permissionId, uint8_t* allowed) {
            if (!Valid(owner) || !Valid(permissionId) || allowed == nullptr) return RESULT_INVALID_ARGUMENT;
            const auto ownerId = Copy(owner);
            const auto permission = Copy(permissionId);
            std::scoped_lock lock(mutex_);
            const auto grants = permissions_.find(ownerId);
            *allowed = ownerId == "shroudtopia.core" || permission.starts_with("shroudtopia.registry.") ||
                permission == "shroudtopia.lifecycle.native" ||
                (grants != permissions_.end() && grants->second.contains(permission)) ? 1 : 0;
            return RESULT_OK;
        }

        void GrantCapabilities(const std::string& owner, const std::vector<std::string>& capabilities) {
            std::scoped_lock lock(mutex_);
            auto& grants = permissions_[owner];
            grants.clear();
            grants.insert(capabilities.begin(), capabilities.end());
        }

    private:
        Registration NextHandle() { return nextHandle_.fetch_add(1, std::memory_order_relaxed); }

        template <typename Map>
        static void EraseOwner(Map& entries, const std::string& owner) {
            std::erase_if(entries, [&](const auto& pair) { return pair.second.owner == owner; });
        }

        std::mutex mutex_;
        std::atomic<Registration> nextHandle_{1};
        std::unordered_map<Registration, ServiceEntry> services_;
        std::unordered_map<Registration, EventEntry> events_;
        std::unordered_map<Registration, CommandEntry> commands_;
        std::unordered_map<Registration, ActionEntry> actions_;
        std::unordered_map<std::string, std::unordered_set<std::string>> permissions_;
    };

    PlatformRegistry registry;

    Result CALL RegisterService(StringView owner, const ServiceDescriptor* descriptor, Registration* registration) {
        return registry.RegisterService(owner, descriptor, registration);
    }
    Result CALL FindService(const ServiceRequest* request, const void** interfacePointer) {
        return registry.FindService(request, interfacePointer);
    }
    Result CALL SubscribeEvent(StringView owner, const EventSubscription* subscription, Registration* registration) {
        return registry.SubscribeEvent(owner, subscription, registration);
    }
    Result CALL PublishEvent(StringView owner, const Event* eventData) { return registry.PublishEvent(owner, eventData); }
    Result CALL RegisterCommand(StringView owner, const CommandDescriptor* descriptor, Registration* registration) {
        return registry.RegisterCommand(owner, descriptor, registration);
    }
    Result CALL InvokeCommand(StringView owner, StringView commandId, StringView arguments) {
        return registry.InvokeCommand(owner, commandId, arguments);
    }
    Result CALL RegisterAction(StringView owner, const Action* action, Registration* registration) {
        return registry.RegisterAction(owner, action, registration);
    }
    Result CALL InvokeAction(StringView owner, StringView actionId, StringView inputJson) {
        return registry.InvokeAction(owner, actionId, inputJson);
    }
    Result CALL GetActionState(StringView actionId, ActionState* state) {
        return registry.GetActionState(actionId, state);
    }
    Result CALL ReleaseRegistration(Registration registration) { return registry.Release(registration); }
    Result CALL QueryCapability(StringView capabilityId, CapabilityInfo* information) {
        return registry.QueryCapability(capabilityId, information);
    }
    Result CALL CheckPermission(StringView owner, StringView permissionId, uint8_t* allowed) {
        return registry.CheckPermission(owner, permissionId, allowed);
    }
    bool HasRuntimePatchPermission(StringView owner) {
        uint8_t allowed = 0;
        return registry.CheckPermission(
            owner,
            {CAPABILITY_RUNTIME_PATCHES, sizeof(CAPABILITY_RUNTIME_PATCHES) - 1},
            &allowed) == RESULT_OK && allowed != 0;
    }
    Result CALL CreateRuntimePatch(
        StringView owner, const RuntimePatchOptions* descriptor, RuntimePatch* patch) {
        if (!Valid(owner)) return RESULT_INVALID_ARGUMENT;
        if (!HasRuntimePatchPermission(owner)) return RESULT_PERMISSION_DENIED;
        const auto ownerId = Copy(owner);
        const auto result = RuntimePatches::Create(ownerId, descriptor, patch);
        Utils::LogAs(LOG_DEBUG, ownerId.c_str(), "Patch created: result=%d handle=%llu kind=%d overwrite=%zu payload=%zu",
            static_cast<int>(result), static_cast<unsigned long long>(patch ? *patch : 0),
            descriptor ? static_cast<int>(descriptor->kind) : -1,
            descriptor ? descriptor->overwrite_size : 0, descriptor ? descriptor->payload_size : 0);
        return result;
    }
    Result CALL SetRuntimePatchEnabled(StringView owner, RuntimePatch patch, uint8_t enabled) {
        if (!Valid(owner)) return RESULT_INVALID_ARGUMENT;
        if (!HasRuntimePatchPermission(owner)) return RESULT_PERMISSION_DENIED;
        const auto ownerId = Copy(owner);
        const auto result = RuntimePatches::SetEnabled(ownerId, patch, enabled != 0);
        Utils::LogAs(LOG_DEBUG, ownerId.c_str(), "Patch state: handle=%llu enabled=%u result=%d",
            static_cast<unsigned long long>(patch), enabled != 0, static_cast<int>(result));
        return result;
    }
    Result CALL GetRuntimePatchState(
        StringView owner, RuntimePatch patch, RuntimePatchState* state) {
        if (!Valid(owner)) return RESULT_INVALID_ARGUMENT;
        return RuntimePatches::GetState(Copy(owner), patch, state);
    }
    Result CALL ReleaseRuntimePatch(StringView owner, RuntimePatch patch) {
        if (!Valid(owner)) return RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto result = RuntimePatches::Release(ownerId, patch);
        Utils::LogAs(LOG_DEBUG, ownerId.c_str(), "Patch released: handle=%llu result=%d",
            static_cast<unsigned long long>(patch), static_cast<int>(result));
        return result;
    }
    bool HasPermission(StringView owner, const char* capability) {
        uint8_t allowed = 0;
        return Valid(owner) && registry.CheckPermission(owner,
            {capability, std::strlen(capability)}, &allowed) == RESULT_OK && allowed != 0;
    }
    Result CALL ListAssets(StringView owner, StringView typeName,
        AssetVisitor visitor, void* userData) {
        if (!Valid(owner) || !Valid(typeName) || visitor == nullptr) return RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, CAPABILITY_ASSETS_READ)) return RESULT_PERMISSION_DENIED;
        return AssetsEngine::List(typeName, visitor, userData);
    }
    Result CALL GetAsset(StringView owner, const AssetId* asset,
        char* buffer, size_t capacity, size_t* requiredSize) {
        if (!Valid(owner) || asset == nullptr || asset->struct_size < sizeof(AssetId) ||
            !Valid(asset->guid) || !Valid(asset->type_name) || requiredSize == nullptr ||
            (capacity != 0 && buffer == nullptr)) return RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, CAPABILITY_ASSETS_READ)) return RESULT_PERMISSION_DENIED;
        return AssetsEngine::Get(owner, asset, buffer, capacity, requiredSize);
    }
    Result CALL UpdateAsset(StringView owner, const AssetId* asset,
        StringView json) {
        if (!Valid(owner) || asset == nullptr || asset->struct_size < sizeof(AssetId) ||
            !Valid(asset->guid) || !Valid(asset->type_name) || !Valid(json)) return RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, CAPABILITY_ASSETS_WRITE)) return RESULT_PERMISSION_DENIED;
        return AssetsEngine::Update(owner, asset, json);
    }
    Result CALL SetAssetField(StringView owner, const AssetId* asset,
        StringView path, StringView json) {
        if (!Valid(owner) || asset == nullptr || asset->struct_size < sizeof(AssetId) ||
            !Valid(asset->guid) || !Valid(asset->type_name) || !Valid(path) || !Valid(json)) return RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, CAPABILITY_ASSETS_WRITE)) return RESULT_PERMISSION_DENIED;
        return AssetsEngine::Set(owner, asset, path, json);
    }
    Result CALL CreateAsset(StringView owner, StringView typeName, StringView json,
        AssetVisitor visitor, void* userData) {
        if (!Valid(owner) || !Valid(typeName) || !Valid(json) || visitor == nullptr) return RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, CAPABILITY_ASSETS_WRITE)) return RESULT_PERMISSION_DENIED;
        return AssetsEngine::Create(owner, typeName, json, visitor, userData);
    }
    Result CALL ResetAssets(StringView owner) {
        if (!Valid(owner)) return RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, CAPABILITY_ASSETS_WRITE)) return RESULT_PERMISSION_DENIED;
        return AssetsEngine::Reset(owner);
    }
    Result CALL SaveAssets(StringView owner) {
        if (!Valid(owner)) return RESULT_INVALID_ARGUMENT;
        if (!HasPermission(owner, CAPABILITY_ASSETS_WRITE)) return RESULT_PERMISSION_DENIED;
        return AssetsEngine::Save();
    }
    Result CALL Log(StringView owner, LogLevel level, StringView message) {
        if (!Valid(owner) || (message.size != 0 && message.data == nullptr)) return RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto text = Copy(message);
        Utils::LogAs(level >= LOG_ERROR ? LOG_ERROR :
            level == LOG_WARNING ? LOG_WARNING :
            level == LOG_DEBUG ? LOG_DEBUG :
            level == LOG_TRACE ? LOG_TRACE : LOG_INFO,
            ownerId.c_str(), "%s", text.c_str());
        return RESULT_OK;
    }
    Result CALL GetModSettingBool(StringView owner, StringView key, uint8_t fallback, uint8_t* value) {
        if (!Valid(owner) || !Valid(key) || value == nullptr) return RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto settingKey = Copy(key);
        *value = Config::modGet<bool>(ownerId.c_str(), settingKey.c_str(), fallback != 0) ? 1 : 0;
        return RESULT_OK;
    }
    Result CALL GetModSettingNumber(StringView owner, StringView key, double fallback, double* value) {
        if (!Valid(owner) || !Valid(key) || value == nullptr || !std::isfinite(fallback)) return RESULT_INVALID_ARGUMENT;
        const auto ownerId = Copy(owner);
        const auto settingKey = Copy(key);
        const auto result = Config::modGet<double>(ownerId.c_str(), settingKey.c_str(), fallback);
        if (!std::isfinite(result)) return RESULT_INVALID_ARGUMENT;
        *value = result;
        return RESULT_OK;
    }
    Api api{
        sizeof(Api), API_VERSION,
        RegisterService, FindService,
        SubscribeEvent, PublishEvent,
        RegisterCommand, InvokeCommand,
        RegisterAction, InvokeAction, GetActionState,
        ReleaseRegistration,
        QueryCapability, CheckPermission,
        Log, Utils::ReadLogTail,
        GetModSettingBool,
        GetModSettingNumber,
        CreateRuntimePatch, SetRuntimePatchEnabled, GetRuntimePatchState, ReleaseRuntimePatch,
        ListAssets, GetAsset, UpdateAsset, CreateAsset, ResetAssets, SaveAssets, SetAssetField,
        UiText::Create, UiText::SetText, UiText::Status, UiText::Destroy
    };
}

namespace PlatformApi {
void Initialize() {}

void GrantCapabilities(const std::string& owner, const std::vector<std::string>& capabilities) {
    registry.GrantCapabilities(owner, capabilities);
}

void ReleaseOwner(const std::string& owner) {
    const StringView view{owner.data(), owner.size()};
    UiText::ReleaseOwner(view);
    registry.ReleaseOwner(view);
}

void Shutdown() {
    UiText::Shutdown();
    RuntimePatches::Shutdown();
    AssetsEngine::Shutdown();
}
}

extern "C" API_EXPORT Result CALL ShroudtopiaGetApi(uint32_t requestedApiVersion, const Api** api) {
    if (api == nullptr) return RESULT_INVALID_ARGUMENT;
    *api = nullptr;
    if (requestedApiVersion != API_VERSION) return RESULT_VERSION_MISMATCH;
    PlatformApi::Initialize();
    *api = &::api;
    return RESULT_OK;
}
