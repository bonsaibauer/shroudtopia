#pragma once
#include "shroudtopia.h"
#include "world_native_layout.h"
namespace WorldEngine {
Result Initialize(const Api* api);
void Shutdown(const Api* api);
bool DiagnosticSnapshot(WorldNative::Cursor& cursor,std::uint64_t& sequence,
                        std::uint32_t& thread,std::uintptr_t& input);
}
