# API-Version

Shroudtopia 1.1.0 stellt API 1.1 bereit. Binde `shroudtopia.h` ein, exportiere `CreateMod` und fordere `API_VERSION` an. Der Loader verlangt eine exakte Übereinstimmung.

```c
if (requested_api_version != API_VERSION) {
    return RESULT_VERSION_MISMATCH;
}
```

Alle öffentlichen Funktionen verwenden `CALL`. `API_VERSION` ist `0x00010001`: Die oberen 16 Bits enthalten `1`, die unteren 16 Bits ebenfalls `1`.
