#pragma once

#include "shroudtopia/api/patches.h"

#include <string>

namespace RuntimePatches {
Result Create(const std::string& owner, const RuntimePatchOptions* descriptor, RuntimePatch* patch);
Result SetEnabled(const std::string& owner, RuntimePatch patch, bool enabled);
Result GetState(const std::string& owner, RuntimePatch patch, RuntimePatchState* state);
Result Release(const std::string& owner, RuntimePatch patch);
void ReleaseOwner(const std::string& owner);
void Shutdown();
}
