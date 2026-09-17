#pragma once
#include "shroudtopia/api/ui.h"
namespace UiText {
const UiApi* Api();
void ReleaseOwner(StringView owner);
void Shutdown();
}
