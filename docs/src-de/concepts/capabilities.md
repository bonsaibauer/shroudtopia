# Capabilities und Berechtigungen

Eine Capability beantwortet, ob der aktuelle Build/Provider eine Operation ausführen kann. Eine Permission beantwortet, ob ein Owner diese geschützte Operation in `mod.json` angefordert hat. Vor dem Anzeigen eines Features beides prüfen und dennoch das Operationsergebnis behandeln, da sich Laufzeitzustand ändern kann.

```cpp
ST_CapabilityInfoV1 info{sizeof(info)};
uint8_t allowed = 0;
if (host->query_capability(View(ST_CAPABILITY_ASSETS_WRITE), &info) == ST_RESULT_OK && info.available)
    host->check_permission(owner, View(ST_CAPABILITY_ASSETS_WRITE), &allowed);
```

Native DLLs teilen einen Prozess. Owner-IDs und Permissions sind Policy- und deterministische Cleanup-Mechanismen, keine Sandbox oder Sicherheitsgrenze.
