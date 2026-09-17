# API version

Shroudtopia 1.1.0 provides API 1.1. Include `shroudtopia.h`, export `CreateMod`, and request `API_VERSION`. The loader requires an exact version match.

```c
if (requested_api_version != API_VERSION) {
    return RESULT_VERSION_MISMATCH;
}
```

All public functions use `CALL`. `API_VERSION` is `0x00010001`: the upper 16 bits contain `1` and the lower 16 bits contain `1`.
