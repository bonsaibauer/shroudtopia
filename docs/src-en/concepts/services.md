# Services and discovery

`ST_HostApiV1` is provided to every mod. Optional domain APIs are versioned services. Providers register one global contract ID and major version; consumers use `find_service` with a minimum minor version.

```cpp
ST_ServiceRequest request{sizeof(request), View(ST_ASSETS_SERVICE_ID), 1, 1};
const void* raw = nullptr;
if (host->find_service(&request, &raw) != ST_RESULT_OK) return;
const auto* assets = static_cast<const ST_AssetsApiV1*>(raw);
if (!assets || assets->struct_size < sizeof(ST_AssetsApiV1)) return;
```

The provider owns the returned table until it unregisters. Do not cache it across provider unload. `ST_RESULT_NOT_FOUND` is normal for an optional service.
