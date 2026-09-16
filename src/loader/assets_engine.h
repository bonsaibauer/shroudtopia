#pragma once

#include "shroudtopia/api/assets.h"

namespace AssetsEngine {
bool Initialize();
bool Available();
void Shutdown();

ST_Result Visit(ST_StringView typeName, ST_AssetResourceVisitorV1 visitor, void* userData);
ST_Result ReadJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource,
    char* buffer, size_t capacity, size_t* requiredSize);
ST_Result ReplaceJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource, ST_StringView json);
ST_Result SetFieldJson(ST_StringView owner, const ST_AssetResourceKeyV1* resource, ST_StringView path, ST_StringView json);
ST_Result CreateJson(ST_StringView owner, ST_StringView typeName, ST_StringView json,
    ST_AssetResourceVisitorV1 visitor, void* userData);
ST_Result Discard(ST_StringView owner);
ST_Result Flush();
}

