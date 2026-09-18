#pragma once
#include "shroudtopia.h"
#include "world_native_layout.h"
#include "world_diagnostic_payload.h"
namespace WorldEngine {
Result Initialize(const Api* api);
void Tick();
void Shutdown(const Api* api);
bool DiagnosticSnapshot(WorldNative::Cursor& cursor,std::uint64_t& sequence,
                        std::uint32_t& thread,std::uintptr_t& input);
bool DiagnosticWorldSnapshot(std::uintptr_t& world,std::uint64_t& sequence,
                             std::uint32_t& thread,std::array<std::uint64_t,6>& arguments,
                             WorldNative::Cursor& descriptor);
bool DiagnosticObjectSnapshot(WorldDiagnostic::Record& output);
bool DiagnosticBuildingSnapshot(bool remove,WorldDiagnostic::Record& output);
}
