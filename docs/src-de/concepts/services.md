# Services und Discovery

`ST_HostApiV1` wird jeder Mod bereitgestellt. Optionale Domain-APIs sind versionierte Services. Provider registrieren eine globale Contract-ID und Major-Version; Consumer verwenden `find_service` mit einer minimalen Minor-Version.

```cpp
ST_ServiceRequest request{sizeof(request), View(ST_ASSETS_SERVICE_ID), 1, 1};
const void* raw = nullptr;
if (host->find_service(&request, &raw) != ST_RESULT_OK) return;
const auto* assets = static_cast<const ST_AssetsApiV1*>(raw);
if (!assets || assets->struct_size < sizeof(ST_AssetsApiV1)) return;
```

Der Provider besitzt die zurückgegebene Tabelle bis zur Abmeldung. Nicht über das Entladen des Providers hinaus cachen. `ST_RESULT_NOT_FOUND` ist für einen optionalen Service normal.
