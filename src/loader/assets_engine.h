#pragma once

#include "shroudtopia.h"

namespace AssetsEngine {
bool Initialize();
bool Available();
void Shutdown();

Result List(StringView typeName, AssetVisitor visitor, void* userData);
Result Get(StringView owner, const AssetId* asset,
    char* buffer, size_t capacity, size_t* requiredSize);
Result Update(StringView owner, const AssetId* asset, StringView json);
Result Set(StringView owner, const AssetId* asset, StringView path, StringView json);
Result Create(StringView owner, StringView typeName, StringView json,
    AssetVisitor visitor, void* userData);
Result Reset(StringView owner);
Result Save();
}

