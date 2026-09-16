# Architecture

| Status | Layer | Responsibility |
|:---:|---|---|
| ✅ | Shroudtopia bootstrap | Starts the loader in the game or server process. |
| ✅ | Shroudtopia core | Discovers mods and owns lifecycle, registries, permissions, logging, and cleanup. |
| ✅ | Public C API | Provides stable ABI types and versioned services. |
| ✅ | Mods | Implement features and may publish their own APIs. |
| ✅ | Asset engine | Owns KFC3 parsing, reflection, validation, and persistence. |
| 🚧 | Game and World providers | Convert verified engine behavior into public API operations. |

```text
ShroudEdit and community mods
              │
        Shroudtopia API
              │
    Lifecycle and registries
              │
    Internal game systems
              │
             GAME
```

Every native mod exports `ShroudtopiaCreateModV1` and uses the versioned C ABI. Mods
are loaded only from directories with a valid manifest. KFC processing, reflection,
hooks, runtime access, and asset codecs are internal Shroudtopia systems.

ShroudEdit is an external mod. It consumes public World, Game, Asset, and UI contracts.
the loader remains fully functional without it.
