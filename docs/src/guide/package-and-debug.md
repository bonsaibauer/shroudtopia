# 3. Package and Debug

Place `mod.json` next to the x64 DLL:

```json
{
  "id": "mod.author.hello",
  "name": "Hello Mod",
  "version": "1.0.0",
  "shroudtopia": {
    "abi": "1.0",
    "entrypoint": "ShroudtopiaCreateModV1",
    "binary": "my-first-mod.dll",
    "target": "both",
    "default": { "active": true }
  }
}
```

```text
mods/mod.author.hello/
├── mod.json
└── my-first-mod.dll
```

## Checklist

| Status | Check |
|:---:|---|
| ✅ | DLL is Windows x64 |
| ✅ | Factory export is present |
| ✅ | Manifest and descriptor IDs match |
| ✅ | Mod version exists only in `mod.json` |
| ✅ | No headers from `src/` are included |
| ✅ | Optional services handle `NOT_FOUND` and `UNSUPPORTED` |
| ✅ | Messages use `host->log` |
| ✅ | Current logs use `shroudtopia.logging.read@1.0` when needed |

A successful build verifies the binary contract. Game-specific behavior requires a
supported game profile and an in-game test.
