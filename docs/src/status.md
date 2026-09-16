# Current Status

## Platform matrix

| Status | Area | Verification |
|:---:|---|---|
| ✅ | Bootstrap, discovery, and lifecycle | Release x64 build and lifecycle tests |
| ✅ | Services, events, commands, and settings | Platform API smoke tests |
| ✅ | Capabilities, permissions, and cleanup | Owner and manifest tests |
| ✅ | Central logging and session rotation | Log-reader and rotation tests |
| ✅ | Native text UI | Creation, ownership, update, and cleanup tests |
| ✅ | Runtime patches and detours | Executable-memory smoke tests |
| ✅ | Typed KFC3 Asset API 1.1 | Read, write, conflict, staging, and recovery tests |
| 🧪 | World API 1.1 | Public contract and ShroudEdit test provider |
| 🚧 | Targeting and live World integration | Static evidence and provider roadmap |
| 🚧 | World overlay and game-thread jobs | API design and engine discovery |
| 🚧 | Dedicated-server World writes | Server profile and authorization required |

## Bundled gameplay mods

| Status | Mod | Target |
|:---:|---|---|
| ✅ | Flight | Client |
| ✅ | No Stamina Loss | Client and server manifest target |
| ✅ | No Fall Damage | Client and server manifest target |
| ✅ | No Resource Cost | Client and server manifest target |
| ✅ | Infinite Item Use | Client and server manifest target |
| ✅ | Infinite Item Split | Client and server manifest target |
| ✅ | Unlock Blueprints | Client and server manifest target |

Runtime signatures are game-profile specific. Unsupported or ambiguous signatures
fail without modifying memory.

## Active development

- game-thread dispatcher and world-session lifetime.
- cursor target snapshots.
- entity enumeration and persistent entity writes.
- voxel and terrain region reads and writes.
- semantic tool actions and world overlays.
- dedicated-server authorization and multiplayer replication.
- archive enumeration through the Logging API.

## Validated executable profile

| Process | SHA-256 |
|---|---|
| Client | `AF2F5A1227911D8AA06B3908D6BD0211838211CAE14EA91099CB57D0DF990781` |
| Dedicated server | `001C1B40ED091D8C1AEE583ADDE3800D7C858AE2C7F4DFF54FCA2938B2BE1637` |

Unknown game builds do not receive unverified write capabilities.
