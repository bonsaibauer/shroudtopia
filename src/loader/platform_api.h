#pragma once

#include <string>
#include <vector>

namespace PlatformApi {
void Initialize();
void GrantCapabilities(const std::string& owner, const std::vector<std::string>& capabilities);
void Shutdown();
}
