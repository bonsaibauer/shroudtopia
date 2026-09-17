#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../src/loader/world_native_layout.h"
#include <cassert>
#include <iostream>
#include <limits>
int main() {
    WorldNative::Cursor cursor{};
    cursor.primary.position[0] = -INT64_C(4294967296);
    cursor.primary.position[1] = INT64_C(6442450944);
    cursor.primary.rotation[3] = cursor.secondary.rotation[3] = 1;
    cursor.primary.scale[0] = cursor.primary.scale[1] = cursor.primary.scale[2] = 1;
    cursor.material = 93; cursor.selected.value = 12345; cursor.selected.type = 2;
    CursorSnapshot output{sizeof(output)};
    auto bytes=std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(&cursor),sizeof(cursor));
    assert(WorldNative::DecodeCursor(bytes,&output) == RESULT_OK);
    assert(output.primary.position.x == -1 && output.primary.position.y == 1.5);
    assert(output.material == 93 && output.selected_object.value == 12345 && output.selected_object.type == 2);
    assert(WorldNative::DecodeCursor(bytes.first(159),&output) == RESULT_INVALID_ARGUMENT);
    cursor.primary.rotation[0] = std::numeric_limits<float>::quiet_NaN();
    assert(WorldNative::DecodeCursor(bytes,&output) == RESULT_NOT_AVAILABLE);
    assert(WorldNative::EncodeCell({93,128}) == 0x805d);
    std::size_t index=0;
    assert(WorldNative::CellIndex({3,4,5},{2,1,3},60,index) && index == 41);
    assert(!WorldNative::CellIndex({3,4,5},{3,1,3},60,index));
    assert(!WorldNative::CellIndex({SIZE_MAX,4,5},{0,0,0},60,index));
    std::cout << "Native cursor layout, fixed-point decoding and terrain indexing passed.\n";
}
