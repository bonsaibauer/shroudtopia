# Shroudtopia Commands

| Property | Value |
|---|---|
| Mod ID | `mod.commands` |
| Target | `both` |
| Public API only | ✅ |
| Enshrouded chat input | ❌ |

The mod registers:

- `mod.commands.execute` as the shared command-registry entry point.
- `mod.commands.status` for status checks.
- `mod.commands.settings` as its settings contract.

```text
mod.commands.execute <command-id> [arguments]
```

Any active mod may register a globally unique command. Deactivating Commands removes
its entry points while commands owned by other mods remain registered. Calls currently
enter through the public Command API. A game chat provider is not available.
