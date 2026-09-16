#pragma once
#include "shroudtopia/api/ui.h"
namespace UiText {
const ST_UiTextApiV1* Api();
void ReleaseOwner(ST_StringView owner);
void Shutdown();
}
