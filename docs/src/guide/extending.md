# Publish Mod APIs

Community mods extend the ecosystem through the Service Registry.

```text
Provider mod                         Consumer mod
register_service()                       │
        │                                │
        └──── Service Registry ── find_service()
                         │
              versioned interface pointer
```

## Contract rules

| Rule | Requirement |
|---|---|
| Contract ID | Globally unique dotted name such as `author.navigation` |
| ABI shape | C-compatible structure beginning with `struct_size` |
| Breaking change | New major version |
| Compatible addition | Append an optional field and increase minor version |
| Lifetime | Provider keeps the interface valid until release |
| Cleanup | Provider releases its owner registrations during unload |

Use services for direct queries and actions, events for notifications, and commands
for user-triggered operations. ShroudEdit publishes `shroudedit.blueprints` without
creating a loader dependency on its Blueprint types.
