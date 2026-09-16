# Ownership and Cleanup

## Status

| Status | Feature |
|:---:|---|
| ✅ | Owner-bound registrations |
| ✅ | Release one registration |
| ✅ | Release all owner resources |
| ❌ | Unforgeable caller identity |

Every successful registry operation returns an opaque `ST_Registration` handle.
`release_registration` removes one handle. `release_owner` removes all services,
event subscriptions, commands, settings, UI windows, patches, and supported
owner-bound changes managed by the corresponding subsystem.

Mods pass their owner ID in ABI v1. This provides deterministic cleanup but is not a
security boundary between native DLLs.
