# Capabilities and permissions

A capability answers whether the current build/provider can perform an operation. A permission answers whether one owner requested that protected operation in `mod.json`. Check both before offering the feature, and still handle the operation result because runtime state can change.

```cpp
ST_CapabilityInfoV1 info{sizeof(info)};
uint8_t allowed = 0;
if (host->query_capability(View(ST_CAPABILITY_ASSETS_WRITE), &info) == ST_RESULT_OK && info.available)
    host->check_permission(owner, View(ST_CAPABILITY_ASSETS_WRITE), &allowed);
```

Native DLLs share a process. Owner IDs and permissions are policy and deterministic cleanup mechanisms, not a sandbox or security boundary.
