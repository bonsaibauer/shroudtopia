#pragma once
#include "shroudtopia.h"
namespace UiText {
Result Create(StringView owner, const TextWindowOptions* options, TextWindow* window);
Result SetText(StringView owner, TextWindow window, size_t tab, StringView text);
Result Status(StringView owner, TextWindow window, TextWindowStatus* status);
Result Destroy(StringView owner, TextWindow window);
void ReleaseOwner(StringView owner);
void Shutdown();
}
