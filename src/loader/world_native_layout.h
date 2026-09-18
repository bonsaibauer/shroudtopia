#pragma once

#include "shroudtopia_world.h"
#include <array>
#include <cmath>
#include <cstring>
#include <span>

namespace WorldNative {
// Native placement reads float3 endpoints at offsets 0 and 16, not 0 and 12.
struct alignas(16) PlacementBounds {
    float minimum[4]{};
    float maximum[4]{};
    explicit PlacementBounds(const float* packed) {
        std::memcpy(minimum,packed,3*sizeof(float));
        std::memcpy(maximum,packed+3,3*sizeof(float));
    }
};
static_assert(sizeof(PlacementBounds)==32 && offsetof(PlacementBounds,maximum)==16);
// Client AF2F5A12...; sizes/offsets from its reflection, confirmed in instructions.
// These are engine layouts, never part of the public mod contract.
struct Transform {
    std::int64_t position[3];
    float rotation[4];
    float scale[3];
    std::uint32_t padding;
};
struct Cursor {
    Transform primary, secondary;
    std::uint8_t primaryFlags, secondaryFlags, material, padding[5];
    std::uint64_t selectionVersion;
    GameObjectId selected;
    std::uint64_t growthAction;
    std::uint8_t attached, tail[7];
};
static_assert(sizeof(Transform) == 0x38);
static_assert(offsetof(Transform, rotation) == 0x18);
static_assert(offsetof(Transform, scale) == 0x28);
static_assert(sizeof(Cursor) == 0xa0);
static_assert(offsetof(Cursor, primaryFlags) == 0x70);
static_assert(offsetof(Cursor, material) == 0x72);
static_assert(offsetof(Cursor, selected) == 0x80);
static_assert(offsetof(Cursor, attached) == 0x98);

inline bool DecodeTransform(const Transform& input, WorldTransform& output) {
    for (float value : input.rotation) if (!std::isfinite(value)) return false;
    for (float value : input.scale) if (!std::isfinite(value)) return false;
    output = {sizeof(output),
        {std::ldexp(static_cast<double>(input.position[0]),-32),
         std::ldexp(static_cast<double>(input.position[1]),-32),
         std::ldexp(static_cast<double>(input.position[2]),-32)},
        {input.rotation[0],input.rotation[1],input.rotation[2],input.rotation[3]},
        {input.scale[0],input.scale[1],input.scale[2]}};
    return true;
}
inline Result DecodeCursor(std::span<const std::uint8_t> bytes, CursorSnapshot* output) {
    if (bytes.size() != sizeof(Cursor) || !output || output->struct_size < sizeof(*output)) return RESULT_INVALID_ARGUMENT;
    Cursor raw{};
    std::memcpy(&raw,bytes.data(),sizeof(raw));
    CursorSnapshot decoded{sizeof(decoded)};
    if (!DecodeTransform(raw.primary,decoded.primary) || !DecodeTransform(raw.secondary,decoded.secondary) || raw.attached > 1)
        return RESULT_NOT_AVAILABLE;
    decoded.selected_object = raw.selected;
    decoded.primary_flags = raw.primaryFlags;
    decoded.secondary_flags = raw.secondaryFlags;
    decoded.material = raw.material;
    decoded.attached_to_prop = raw.attached;
    *output = decoded;
    return RESULT_OK;
}

// The native terrain cell at 0x994df5/0x994e05: material byte then density byte.
// Preserve both bytes. Density zero is not sufficient to discard the raw value.
struct TerrainCell { std::uint8_t material, density; };
static_assert(sizeof(TerrainCell) == 2);
struct TerrainBuffer {
    TerrainCell* data;
    std::uint64_t capacity;
    std::uint32_t dimensions[3];
};
struct alignas(16) TerrainOperation {
    float radius;
    std::int32_t gridOrigin[3];
    float cellSize;
    float center[3];
    std::uint8_t shape;
    std::uint8_t padding[15];
    float axes[3][4];
    float plane[3];
    std::uint8_t usePlane;
    std::uint8_t planePadding[3];
    const float* materialCosts;
    std::uint64_t materialCount;
    void* payout;
    std::uint8_t suppressCommit;
    std::uint8_t tail[7];
};
static_assert(offsetof(TerrainBuffer,dimensions) == 0x10);
static_assert(offsetof(TerrainOperation,cellSize) == 0x10);
static_assert(offsetof(TerrainOperation,axes) == 0x30);
static_assert(offsetof(TerrainOperation,materialCosts) == 0x70);
static_assert(offsetof(TerrainOperation,suppressCommit) == 0x88);
static_assert(sizeof(TerrainOperation) == 0x90);
// This operation edits a supplied buffer. It does NOT itself acquire/commit world chunks.
using ApplyTerrainBuffer = bool (CALL*)(const TerrainOperation*,TerrainBuffer*,void*,const std::uint32_t*,const std::uint32_t*);
inline std::uint32_t EncodeCell(TerrainCell cell) {
    return cell.material | (static_cast<std::uint32_t>(cell.density) << 8);
}
// X fastest, then Y, then Z, from 0x994de1..0x994df0.
inline bool CellIndex(std::array<std::size_t,3> dimensions,
    std::array<std::size_t,3> cell, std::size_t capacity, std::size_t& index) {
    std::size_t product = 1;
    for (int i=0; i<3; ++i) {
        if (!dimensions[i] || cell[i] >= dimensions[i] || dimensions[i] > capacity/product) return false;
        product *= dimensions[i];
    }
    index = (cell[2]*dimensions[1]+cell[1])*dimensions[0]+cell[0];
    return true;
}
}
