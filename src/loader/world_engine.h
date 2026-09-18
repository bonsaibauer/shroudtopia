#pragma once
#include "shroudtopia.h"
#include "world_native_layout.h"
namespace WorldEngine {
Result Initialize(const Api* api);
void Shutdown(const Api* api);
bool DiagnosticSnapshot(WorldNative::Cursor& cursor,std::uint64_t& sequence,
                        std::uint32_t& thread,std::uintptr_t& input);
bool DiagnosticWorldSnapshot(std::uintptr_t& world,std::uint64_t& sequence,
                             std::uint32_t& thread,std::array<std::uint64_t,6>& arguments,
                             WorldNative::Cursor& descriptor);
}
