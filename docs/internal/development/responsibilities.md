# Loader, API, and Mod Responsibilities

```text
Mod feature intent
       ↓
Shroudtopia public API
       ↓
Internal asset, runtime, game, and world systems
       ↓
Enshrouded
```

| Component | Owns | Does not own |
|---|---|---|
| Loader | Discovery, lifecycle, policy, registries, cleanup, and internal providers | Feature-specific mod behavior |
| Public API | Stable types, function tables, service versions, ownership, and errors | EXE addresses and parser internals |
| Mod | Feature rules, settings, user workflow, and its own published contracts | Loading other mods and core hook infrastructure |

Reusable technical access belongs in Shroudtopia. Feature behavior belongs in a mod.
For example, reading a typed resource is an Asset API operation. Unlocking every
recipe is the responsibility of `mod.unlock-blueprints`.

Mods may publish services, commands, events, and settings. This extends the ecosystem
without making the loader depend on a specific mod.
