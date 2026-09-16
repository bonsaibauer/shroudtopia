#pragma once

#include "shroudtopia/api/runtime.h"

#include <string>

namespace RuntimePatches {
ST_Result Create(const std::string& owner, const ST_RuntimePatchDescriptorV1* descriptor, ST_RuntimePatch* patch);
ST_Result SetEnabled(const std::string& owner, ST_RuntimePatch patch, bool enabled);
ST_Result GetState(const std::string& owner, ST_RuntimePatch patch, ST_RuntimePatchStateV1* state);
ST_Result Release(const std::string& owner, ST_RuntimePatch patch);
void ReleaseOwner(const std::string& owner);
void Shutdown();
}
