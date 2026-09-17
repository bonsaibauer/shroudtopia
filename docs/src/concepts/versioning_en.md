# API versioning

Include `shroudtopia/api.h`, export `CreateMod`, and request `API_VERSION`. The loader accepts the current major API version and returns `RESULT_VERSION_MISMATCH` for any other major version.

```c
if (requested_api_version != API_VERSION) {
    return RESULT_VERSION_MISMATCH;
}
```

All public functions use `CALL`. A release that changes a public type, function signature, constant, or lifecycle rule increments the major version. Additive features increment the minor version; implementation fixes increment the patch version.
