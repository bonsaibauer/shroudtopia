# World API and ShroudEdit Roadmap

ShroudEdit is an external mod. Cursor, entity, voxel, game-thread, and rendering
access are implemented inside Shroudtopia and published as general services.

## Current matrix

| Status | Area | Current result |
|:---:|---|---|
| ✅ | ShroudEdit selection | Two points, negative coordinates, and bounded volume |
| ✅ | Blueprint logic | Schema 1.1, validation, files, transforms, and coverage |
| 🧪 | Capture, paste, and undo | Pass against a controlled World test provider |
| 🚧 | Targeting | Reflection and a client hook candidate are available |
| 🚧 | Entities | Runtime ID, region query, template resolution, and writes are required |
| 🚧 | Voxel and terrain | Buffer candidates exist, cell semantics and persistence are required |
| 🚧 | Tool action | Asset definition and runtime action event are required |
| 🚧 | Overlay | Selection box and placement ghosts require a renderer provider |
| 🚧 | Multiplayer | Server profile, authorization, and replication are required |

## Dependency map

```text
Game profile and reflection
          │
          v
Game-thread dispatcher ─────> World and session lifetime
          │
          ├────> Targeting ───> Tool action ───> Point A, B, and target
          ├────> Entity read ─> Entity write ──> Prop copy and paste
          ├────> Grid read ───> Grid write ────> Voxel and terrain copy and paste
          └────> Overlay ──────────────────────> Selection and preview
```

## Public contracts

| Status | Contract | Purpose |
|:---:|---|---|
| 🚧 | `shroudtopia.game.targeting@1` | Return a copied target snapshot. |
| 🚧 | `shroudtopia.game.actions@1` | Publish semantic tool and item actions. |
| 🧪 | `shroudtopia.world@1.1` | Entity and grid operations. |
| 🚧 | `shroudtopia.ui.overlay@1` | Draw temporary lines, boxes, and ghosts. |
| 🚧 | `shroudtopia.world.jobs@1` | Run large cancellable edits with progress. |

Item definitions use the Asset API when possible. Runtime actions use a Game service.
Blueprint files, selection state, rotation, library management, and undo planning stay
inside ShroudEdit.

## Discovery gate

| Step | Required evidence |
|---:|---|
| 1 | Reproducible game action and negative control |
| 2 | Full executable fingerprint and matching reflection |
| 3 | Function boundary, callers, control flow, and expected bytes |
| 4 | Read-only POD snapshots collected without hook-side work |
| 5 | Confirmed thread, lifetime, flags, coordinates, and world transitions |
| 6 | One reversible write in a disposable world |
| 7 | Read-back, undo, save, reload, and client/server verification |
| 8 | Internal provider with stable handles and documented errors |
| 9 | Capability enabled only for the verified profile |

## Implementation order

| Order | Status | Deliverable | Acceptance |
|---:|:---:|---|---|
| 1 | 🚧 | Game profile and telemetry ring | Wrong builds install no hooks. |
| 2 | 🚧 | Game-thread dispatcher and session ID | Jobs run on the verified thread. |
| 3 | 🚧 | Target snapshot | Terrain, prop, and no-hit states are distinct. |
| 4 | 🚧 | Entity resolver | Runtime ID maps to transform and template. |
| 5 | 🚧 | Entity region query | Complete result or explicit error. |
| 6 | 🚧 | Entity writes | Spawn, move, delete, save, and reload one prop. |
| 7 | 🚧 | Grid read | Material, building cell, empty, and unknown remain distinct. |
| 8 | 🚧 | Grid write | Change, read back, undo, save, and reload one cell. |
| 9 | 🚧 | Tool action | ShroudEdit sets A and B through a real item action. |
| 10 | 🚧 | Overlay | Selection and preview disappear on cancel and world change. |
| 11 | 🚧 | World jobs | Mixed volume supports progress, cancellation, and recovery. |
| 12 | 🚧 | Multiplayer | Server authorizes and all clients observe one result. |

## Reference scene

The disposable verification world contains one rotated and scaled prop, one building
block, wall, roof, foundation, two terrain materials, an enclosed empty space, one
chunk boundary, and one dynamic object that must not appear as a copyable prop.

Each operation records pre-state, action, API snapshot, post-state, undo, and
save-reload result. API availability grows from measured behavior only.
