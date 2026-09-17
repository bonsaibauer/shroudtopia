# API domains and extension services

`Api` is the single entry point provided to every Mod. Built-in domains are direct members: `api->assets`, `api->patches`, `api->ui`, and `api->logs`. No discovery call or second public function table is needed to reach them.

```cpp
if (!api->assets) return RESULT_NOT_AVAILABLE;
return api->assets->list(owner, type, visitor, context);
```

`register_service` and `find_service` handle optional contracts published by third-party Mods. Lookup requires the exact major and minor contract version. A provider owns an extension table until it unregisters; consumers must not retain it across provider unload.
