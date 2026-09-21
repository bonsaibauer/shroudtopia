#pragma once

#include "shroudtopia.h"

namespace UiPages {
Result Register(StringView owner, const UiPageDescriptor* descriptor, Registration registration);
Result Visit(StringView owner, UiPageVisitor visitor, void* user_data);
Result Release(Registration registration);
void ReleaseOwner(StringView owner);
void Shutdown();
}
