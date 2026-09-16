#pragma once

#include <windows.h>

namespace Runtime {
BOOL Start();
BOOL Stop(DWORD timeout_milliseconds);
}
