#pragma once
#include "shroudtopia.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace BuildingNative {
// Reflection and producer stores at 3EBB70 / 3EBCB0. These are emitted ECS
// events, NOT callable world-write commands. Material is MaterialFeedbackId.
struct Event {
    std::uint64_t timestamp;
    float position[3], orientation[4], volume_min[3], volume_max[3];
    std::uint32_t material_feedback;
    std::uint32_t tracking_or_owner, owner_or_padding;
};
static_assert(sizeof(Event)==72 && offsetof(Event,position)==8 &&
    offsetof(Event,volume_min)==36 && offsetof(Event,material_feedback)==60);
// Client build AF2F5A... only. Recovered from A05180 (row copy), BBB660
// (item filtering) and B08808..B08D0F (row construction). Not a public ABI.
struct Item {
    std::uint64_t type;
    std::uint32_t item_id;
    std::array<std::byte, 0x3c> remaining;
};
struct Row {
    std::uint8_t kind;
    std::array<std::byte,3> padding0;
    std::uint32_t label;
    std::uint8_t style;
    std::array<std::byte,7> padding1;
    const Item* items;
    std::uint64_t item_count;
    std::array<std::byte,0x50> remaining;
};
struct Rows {
    Row* data;
    std::uint64_t size, capacity;
    void* resize_callback;
    void* allocator;
};
// BE5B00 returns this state. B08F75..B08FD4 initializes one zero slot per
// UI row, but ONLY when slot_count == 0 and the UI instance is new.
// Appending rows after initialization does not grow this array automatically.
// Ownership remains with the engine; never replace/free these pointers.
struct Selection {
    std::int64_t row;
    std::int64_t* slots;
    std::uint64_t slot_count, slot_capacity;
    void* resize_callback;
    void* allocator;
};
static_assert(sizeof(Selection)==0x30 && offsetof(Selection,slots)==8 &&
              offsetof(Selection,slot_count)==0x10 && offsetof(Selection,resize_callback)==0x20);
static_assert(sizeof(Item)==0x48 && offsetof(Item,item_id)==8);
static_assert(sizeof(Row)==0x70 && offsetof(Row,items)==0x10 && offsetof(Row,item_count)==0x18);
static_assert(sizeof(Rows)==0x28 && offsetof(Rows,capacity)==0x10);

// Prepare only in caller-owned storage. No engine allocations, hooks or writes.
// Borrowed fields and outputItems must outlive the consuming UI call. The native
// callback lifetime must be established before using this with the live menu.
inline Result PrepareRow(const Row& source, std::span<const Item> sourceItems,
                         std::uint32_t itemId, std::uint32_t label,
                         std::span<Item> outputItems, Row* output) {
    // B09308/B09327/B0932C interpret item counts/indices as signed bytes.
    if (!output || output==&source || !label || !itemId || sourceItems.size()>127 || outputItems.size()>127 ||
        source.items!=sourceItems.data() || source.item_count!=sourceItems.size()) return RESULT_INVALID_ARGUMENT;
    std::size_t count=0;
    for (const auto& item:sourceItems) if (item.item_id==itemId) ++count;
    if (!count) return RESULT_NOT_FOUND;
    if (count>outputItems.size()) return RESULT_INVALID_ARGUMENT;
    const auto inputAddress=reinterpret_cast<std::uintptr_t>(sourceItems.data());
    const auto outputAddress=reinterpret_cast<std::uintptr_t>(outputItems.data());
    if (outputAddress>=inputAddress ? outputAddress-inputAddress<sourceItems.size_bytes()
                                   : inputAddress-outputAddress<outputItems.size_bytes()) return RESULT_INVALID_ARGUMENT;
    std::size_t index=0;
    for (const auto& item:sourceItems) if (item.item_id==itemId) outputItems[index++]=item;
    *output=source;
    output->label=label;
    output->items=outputItems.data();
    output->item_count=count;
    return RESULT_OK;
}

// Explicitly refuses a full six-row buffer; never assumes room for a seventh.
inline Result AppendOwnedRow(std::span<Row> storage, std::size_t* count, const Row& row) {
    if (!count || *count>storage.size() || storage.size()>32) return RESULT_INVALID_ARGUMENT;
    if (*count==storage.size()) return RESULT_NOT_AVAILABLE;
    storage[(*count)++]=row;
    return RESULT_OK;
}
}
