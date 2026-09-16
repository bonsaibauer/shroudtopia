# Services

## Status

| Status | Feature |
|:---:|---|
| ✅ | Publish and discover versioned providers |
| ✅ | Owner cleanup |
| ✅ | One provider per contract ID and major version |
| ❌ | Safe leases across parallel provider unload |

```cpp
ST_ServiceDescriptor service{
    sizeof(service),
    View("author.weather.api"),
    1, 0,
    &weather_interface
};
ST_Registration registration = 0;
const ST_Result result = host->register_service(owner, &service, &registration);
```

A consumer requests the same major version and a minimum minor version. Shroudtopia
returns the highest compatible minor provider.

| Version change | Rule |
|---|---|
| Breaking ABI change | Increase major. |
| Appended optional field | Increase minor and preserve the structure prefix. |

The provider owns the interface pointer and keeps it valid until release. Native mods
share a process. The registry is an API policy and ownership mechanism, not a native
code sandbox.
