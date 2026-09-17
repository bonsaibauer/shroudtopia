#include "pch.h"
#include "runtime_patches.h"

#include <algorithm>
#include <atomic>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {
using Bytes = std::vector<std::uint8_t>;

bool WriteMemory(std::uintptr_t address, const void* data, size_t size) {
    if (address == 0 || data == nullptr || size == 0) return false;
    DWORD previous = 0;
    if (!VirtualProtect(reinterpret_cast<void*>(address), size, PAGE_EXECUTE_READWRITE, &previous)) return false;
    std::memcpy(reinterpret_cast<void*>(address), data, size);
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(address), size);
    DWORD ignored = 0;
    return VirtualProtect(reinterpret_cast<void*>(address), size, previous, &ignored) != FALSE;
}

bool Relative(std::uintptr_t instruction, std::uintptr_t destination, std::int32_t& value) {
    const auto delta = static_cast<std::int64_t>(destination) -
        static_cast<std::int64_t>(instruction + 5);
    if (delta < (std::numeric_limits<std::int32_t>::min)() ||
        delta > (std::numeric_limits<std::int32_t>::max)()) return false;
    value = static_cast<std::int32_t>(delta);
    return true;
}

void* AllocateNear(std::uintptr_t target, size_t size) {
    SYSTEM_INFO system{};
    GetSystemInfo(&system);
    const auto granularity = static_cast<std::uintptr_t>(system.dwAllocationGranularity);
    constexpr std::uintptr_t range = 0x7FFF0000;
    const auto minimum = (std::max)(reinterpret_cast<std::uintptr_t>(system.lpMinimumApplicationAddress),
        target > range ? target - range : 0);
    const auto maximum = (std::min)(reinterpret_cast<std::uintptr_t>(system.lpMaximumApplicationAddress),
        target <= (std::numeric_limits<std::uintptr_t>::max)() - range ? target + range : target);

    for (std::uintptr_t cursor = minimum; cursor < maximum;) {
        MEMORY_BASIC_INFORMATION information{};
        if (VirtualQuery(reinterpret_cast<void*>(cursor), &information, sizeof(information)) == 0) break;
        const auto base = reinterpret_cast<std::uintptr_t>(information.BaseAddress);
        const auto end = base + information.RegionSize;
        if (information.State == MEM_FREE) {
            const auto candidate = (base + granularity - 1) & ~(granularity - 1);
            if (candidate >= minimum && candidate + size <= end && candidate + size <= maximum) {
                if (void* memory = VirtualAlloc(reinterpret_cast<void*>(candidate), size,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)) {
                    std::int32_t ignored = 0;
                    if (Relative(target, reinterpret_cast<std::uintptr_t>(memory), ignored)) return memory;
                    VirtualFree(memory, 0, MEM_RELEASE);
                }
            }
        }
        if (end <= cursor) break;
        cursor = end;
    }
    return nullptr;
}

std::vector<int> ParseSignature(std::string_view text) {
    std::vector<int> signature;
    std::istringstream stream{std::string(text)};
    std::string token;
    while (stream >> token) {
        if (token == "?" || token == "??") {
            signature.push_back(-1);
            continue;
        }
        unsigned int value = 0;
        const auto result = std::from_chars(token.data(), token.data() + token.size(), value, 16);
        if (result.ec != std::errc{} || result.ptr != token.data() + token.size() || value > 0xFF) return {};
        signature.push_back(static_cast<int>(value));
    }
    return signature;
}

std::uintptr_t FindSignature(std::string_view text) {
    const auto signature = ParseSignature(text);
    if (signature.empty()) return 0;
    const auto module = reinterpret_cast<std::uint8_t*>(GetModuleHandleW(nullptr));
    if (module == nullptr) return 0;
    const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
    const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(module + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;

    std::uintptr_t found = 0;
    const auto sections = IMAGE_FIRSECTION(nt);
    for (WORD index = 0; index < nt->FileHeader.NumberOfSections; ++index) {
        const auto& section = sections[index];
        if ((section.Characteristics & IMAGE_SCN_MEM_EXECUTE) == 0 ||
            section.VirtualAddress >= nt->OptionalHeader.SizeOfImage) continue;
        const size_t remaining = nt->OptionalHeader.SizeOfImage - section.VirtualAddress;
        const size_t size = (std::min)(static_cast<size_t>(section.Misc.VirtualSize), remaining);
        if (size < signature.size()) continue;
        const auto start = module + section.VirtualAddress;
        for (size_t offset = 0; offset <= size - signature.size(); ++offset) {
            bool matches = true;
            for (size_t byte = 0; byte < signature.size(); ++byte) {
                if (signature[byte] >= 0 && start[offset + byte] != signature[byte]) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                if (found != 0) return 0; // Ambiguous signatures must never patch arbitrary code.
                found = reinterpret_cast<std::uintptr_t>(start + offset);
            }
        }
    }
    return found;
}

class Patch final {
public:
    Patch(std::uintptr_t target, Bytes replacement, void* shell = nullptr)
        : target_(target), replacement_(std::move(replacement)), shell_(shell) {
        original_.resize(replacement_.size());
        std::memcpy(original_.data(), reinterpret_cast<const void*>(target_), original_.size());
    }

    ~Patch() {
        if (enabled_) Enable(false);
        // Keep the shell allocated if restoring original bytes failed. A leak is
        // safer than leaving a live jump into released executable memory.
        if (shell_ != nullptr && !enabled_) VirtualFree(shell_, 0, MEM_RELEASE);
    }

    bool Enable(bool enabled) {
        if (enabled == enabled_) return true;
        const auto& expected = enabled ? original_ : replacement_;
        if (std::memcmp(reinterpret_cast<const void*>(target_), expected.data(), expected.size()) != 0) return false;
        const auto& data = enabled ? replacement_ : original_;
        if (!WriteMemory(target_, data.data(), data.size())) return false;
        enabled_ = enabled;
        return true;
    }

    bool enabled() const { return enabled_; }
    bool Overlaps(const Patch& other) const {
        return target_ < other.target_ + other.replacement_.size() &&
            other.target_ < target_ + replacement_.size();
    }

private:
    std::uintptr_t target_;
    Bytes replacement_;
    Bytes original_;
    void* shell_;
    bool enabled_ = false;
};

struct Entry {
    std::string owner;
    std::unique_ptr<Patch> patch;
};

std::mutex mutex;
std::atomic<RuntimePatch> nextHandle{1};
std::unordered_map<RuntimePatch, Entry> patches;

Result BuildPatch(const RuntimePatchOptions& descriptor, std::unique_ptr<Patch>& result) {
    if (descriptor.signature.data == nullptr || descriptor.signature.size == 0 ||
        descriptor.payload == nullptr || descriptor.payload_size == 0 ||
        descriptor.payload_size > 4096 || descriptor.overwrite_size == 0 || descriptor.overwrite_size > 4096) {
        return RESULT_INVALID_ARGUMENT;
    }
    const std::string_view signature(descriptor.signature.data, descriptor.signature.size);
    const auto match = FindSignature(signature);
    if (match == 0) return RESULT_NOT_FOUND;
    if (match > static_cast<std::uintptr_t>((std::numeric_limits<std::int64_t>::max)())) {
        return RESULT_INTERNAL_ERROR;
    }
    const auto signedMatch = static_cast<std::int64_t>(match);
    if ((descriptor.match_offset > 0 && signedMatch > (std::numeric_limits<std::int64_t>::max)() - descriptor.match_offset) ||
        (descriptor.match_offset < 0 && signedMatch < (std::numeric_limits<std::int64_t>::min)() - descriptor.match_offset)) {
        return RESULT_INVALID_ARGUMENT;
    }
    const auto signedTarget = signedMatch + descriptor.match_offset;
    if (signedTarget <= 0) return RESULT_INVALID_ARGUMENT;
    const auto target = static_cast<std::uintptr_t>(signedTarget);
    MEMORY_BASIC_INFORMATION targetMemory{};
    if (VirtualQuery(reinterpret_cast<const void*>(target), &targetMemory, sizeof(targetMemory)) == 0 ||
        targetMemory.State != MEM_COMMIT ||
        (targetMemory.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0 ||
        target > (std::numeric_limits<std::uintptr_t>::max)() - descriptor.overwrite_size ||
        target + descriptor.overwrite_size > reinterpret_cast<std::uintptr_t>(targetMemory.BaseAddress) + targetMemory.RegionSize) {
        return RESULT_INVALID_ARGUMENT;
    }

    if (descriptor.kind == RUNTIME_PATCH_DIRECT) {
        if (descriptor.overwrite_size != descriptor.payload_size || descriptor.relocation_count != 0) {
            return RESULT_INVALID_ARGUMENT;
        }
        result = std::make_unique<Patch>(
            target, Bytes(descriptor.payload, descriptor.payload + descriptor.payload_size));
        return RESULT_OK;
    }
    if (descriptor.kind != RUNTIME_PATCH_DETOUR || descriptor.overwrite_size < 5) {
        return RESULT_INVALID_ARGUMENT;
    }

    void* allocation = AllocateNear(target, descriptor.payload_size);
    if (allocation == nullptr) return RESULT_INTERNAL_ERROR;
    const auto shell = reinterpret_cast<std::uintptr_t>(allocation);
    Bytes payload(descriptor.payload, descriptor.payload + descriptor.payload_size);
    for (size_t index = 0; index < descriptor.relocation_count; ++index) {
        const auto& relocation = descriptor.relocations[index];
        if (relocation.struct_size < sizeof(RuntimeRelocation) ||
            relocation.kind != RUNTIME_RELOCATION_REL32_RETURN ||
            relocation.payload_offset == 0 ||
            relocation.payload_offset + sizeof(std::int32_t) > payload.size() ||
            (payload[relocation.payload_offset - 1] != 0xE9 &&
             payload[relocation.payload_offset - 1] != 0xE8)) {
            VirtualFree(allocation, 0, MEM_RELEASE);
            return RESULT_INVALID_ARGUMENT;
        }
        std::int32_t relative = 0;
        if (!Relative(shell + relocation.payload_offset - 1,
            target + descriptor.overwrite_size, relative)) {
            VirtualFree(allocation, 0, MEM_RELEASE);
            return RESULT_INTERNAL_ERROR;
        }
        std::memcpy(payload.data() + relocation.payload_offset, &relative, sizeof(relative));
    }
    std::memcpy(allocation, payload.data(), payload.size());
    FlushInstructionCache(GetCurrentProcess(), allocation, payload.size());

    Bytes detour(descriptor.overwrite_size, 0x90);
    std::int32_t relative = 0;
    if (!Relative(target, shell, relative)) {
        VirtualFree(allocation, 0, MEM_RELEASE);
        return RESULT_INTERNAL_ERROR;
    }
    detour[0] = 0xE9;
    std::memcpy(detour.data() + 1, &relative, sizeof(relative));
    result = std::make_unique<Patch>(target, std::move(detour), allocation);
    return RESULT_OK;
}
}

namespace RuntimePatches {
Result Create(const std::string& owner, const RuntimePatchOptions* descriptor, RuntimePatch* patch) {
    if (owner.empty() || descriptor == nullptr || patch == nullptr ||
        descriptor->struct_size < sizeof(RuntimePatchOptions) ||
        (descriptor->relocation_count != 0 && descriptor->relocations == nullptr)) {
        return RESULT_INVALID_ARGUMENT;
    }
    *patch = 0;
    std::scoped_lock lock(mutex);
    std::unique_ptr<Patch> implementation;
    const auto result = BuildPatch(*descriptor, implementation);
    if (result != RESULT_OK) return result;

    for (const auto& [handle, entry] : patches) {
        if (implementation->Overlaps(*entry.patch)) return RESULT_CONFLICT;
    }
    const auto handle = nextHandle.fetch_add(1, std::memory_order_relaxed);
    patches.emplace(handle, Entry{owner, std::move(implementation)});
    *patch = handle;
    return RESULT_OK;
}

Result SetEnabled(const std::string& owner, RuntimePatch patch, bool enabled) {
    if (owner.empty() || patch == 0) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(mutex);
    const auto entry = patches.find(patch);
    if (entry == patches.end() || entry->second.owner != owner) return RESULT_NOT_FOUND;
    return entry->second.patch->Enable(enabled) ? RESULT_OK : RESULT_INTERNAL_ERROR;
}

Result GetState(const std::string& owner, RuntimePatch patch, RuntimePatchState* state) {
    if (owner.empty() || patch == 0 || state == nullptr ||
        state->struct_size < sizeof(RuntimePatchState)) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(mutex);
    const auto entry = patches.find(patch);
    if (entry == patches.end() || entry->second.owner != owner) return RESULT_NOT_FOUND;
    state->enabled = entry->second.patch->enabled() ? 1 : 0;
    std::fill(std::begin(state->reserved), std::end(state->reserved), std::uint8_t{0});
    return RESULT_OK;
}

Result Release(const std::string& owner, RuntimePatch patch) {
    if (owner.empty() || patch == 0) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(mutex);
    const auto entry = patches.find(patch);
    if (entry == patches.end() || entry->second.owner != owner) return RESULT_NOT_FOUND;
    if (!entry->second.patch->Enable(false)) return RESULT_INTERNAL_ERROR;
    patches.erase(entry);
    return RESULT_OK;
}

void ReleaseOwner(const std::string& owner) {
    std::scoped_lock lock(mutex);
    std::erase_if(patches, [&](const auto& pair) { return pair.second.owner == owner; });
}

void Shutdown() {
    std::scoped_lock lock(mutex);
    patches.clear();
}
}
