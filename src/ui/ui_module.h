#pragma once

#include "shroudtopia.h"

namespace ShroudforgeUi {
constexpr const char* Version = "0.2.0";
Result Initialize(const Api* api);
void Tick();
void Shutdown();
const char* RendererStatus();
}
