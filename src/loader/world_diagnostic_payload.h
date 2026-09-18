#pragma once
#include "world_native_layout.h"
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace WorldDiagnostic {
enum class Kind { Cursor, TerrainEntry, BuildingEvent, VoxelWriteEntry, WorldContext };
// Fixed latest-sample mailbox. No allocation, file access, calls or world writes
// in the hook. Reader and writers acquire the same lock; contention drops a sample.
struct alignas(8) Record {
    volatile long lock{};
    volatile long enabled{};
    std::uint64_t sequence{};
    std::uint64_t arguments[6]{};
    WorldNative::Cursor cursor{};
    std::uint32_t thread{};
};
static_assert(sizeof(long)==4 && offsetof(Record,cursor)==64 && offsetof(Record,thread)==224);
inline std::vector<std::uint8_t> Payload(Record* record, Kind kind) {
    std::vector<std::uint8_t> code;
    auto bytes=[&](std::initializer_list<std::uint8_t> values) { code.insert(code.end(),values); };
    auto qword=[&](std::uint64_t value) { for (int i=0;i<8;++i) code.push_back(static_cast<std::uint8_t>(value>>(i*8))); };
    // Save every modified register and RFLAGS (including the direction flag).
    bytes({0x9c,0x50,0x51,0x52,0x56,0x57,0x48,0xbf}); qword(reinterpret_cast<std::uintptr_t>(record));
    bytes({0x83,0x7f,0x04,0x00,0x0f,0x84}); const auto disabled=code.size(); bytes({0,0,0,0});
    bytes({0x31,0xc0,0xb9,1,0,0,0,0xf0,0x0f,0xb1,0x0f,0x0f,0x85});
    const auto busy=code.size(); bytes({0,0,0,0});
    if (kind==Kind::Cursor) {
        bytes({0x4c,0x89,0x77,0x10}); // arguments[0] = R14, diagnostic identity only
        bytes({0x49,0x8d,0xb6,0x70,0x02,0,0,0x48,0x8b,0xd7}); // source; preserve mailbox base
        bytes({0x48,0x8d,0x7f,0x40,0xb9,20,0,0,0,0xfc,0xf3,0x48,0xa5,0x48,0x8b,0xfa});
    } else if (kind==Kind::BuildingEvent) {
        // Completed 72-byte ECS event in RBX; context in R14. The caller replays
        // the final owner-ID store BEFORE this payload. No game pointers escape.
        bytes({0x4c,0x89,0x77,0x10,0x48,0x89,0x5f,0x18});
        bytes({0x48,0x8b,0xf3,0x48,0x8b,0xd7,0x48,0x8d,0x7f,0x40});
        bytes({0xb9,9,0,0,0,0xfc,0xf3,0x48,0xa5,0x48,0x8b,0xfa});
    } else {
        // Entry arguments + return address. Do not dereference any engine argument.
        bytes({0x48,0x8b,0x44,0x24,0x18,0x48,0x89,0x47,0x10});
        bytes({0x48,0x8b,0x44,0x24,0x10,0x48,0x89,0x47,0x18});
        bytes({0x4c,0x89,0x47,0x20,0x4c,0x89,0x4f,0x28});
        bytes({0x48,0x8b,0x44,0x24,0x30,0x48,0x89,0x47,0x30});
        bytes({0x48,0x8b,0x44,0x24,0x58,0x48,0x89,0x47,0x38});
        if (kind==Kind::VoxelWriteEntry) {
            // Sixth argument: descriptor read by the original function through
            // byte 0x47. Copy that bounded prefix while it is alive, not later.
            bytes({0x48,0x8b,0x74,0x24,0x60,0x48,0x8b,0xd7,0x48,0x8d,0x7f,0x40});
            bytes({0xb9,9,0,0,0,0xfc,0xf3,0x48,0xa5,0x48,0x8b,0xfa});
            // Low bytes of arguments 7..9; upper bytes are not interpreted.
            bytes({0x48,0x8b,0x44,0x24,0x68,0x48,0x89,0x87,0x88,0,0,0});
            bytes({0x48,0x8b,0x44,0x24,0x70,0x48,0x89,0x87,0x90,0,0,0});
            bytes({0x48,0x8b,0x44,0x24,0x78,0x48,0x89,0x87,0x98,0,0,0});
        }
    }
    // Windows x64 TEB.ClientId.UniqueThread (GS:0x48); no API call.
    bytes({0x65,0x8b,0x04,0x25,0x48,0,0,0,0x89,0x87,0xe0,0,0,0});
    bytes({0x48,0xff,0x47,0x08,0xc7,0x07,0,0,0,0}); // sequence++; release mailbox
    const auto restore=code.size();
    for (const auto fixup:{disabled,busy}) {
        const auto relative=static_cast<std::int32_t>(restore-(fixup+4));
        std::memcpy(code.data()+fixup,&relative,4);
    }
    bytes({0x5f,0x5e,0x5a,0x59,0x58,0x9d});
    // Caller appends complete overwritten instructions and the return relocation.
    return code;
}
}
