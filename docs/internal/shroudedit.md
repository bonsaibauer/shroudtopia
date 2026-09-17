# ShroudEdit World Editor Mod

## Status

| Status | Area |
|:---:|---|
| ✅ | Selection and volume geometry |
| ✅ | Blueprint schema, validation, and files |
| ✅ | Rotation and placement planning |
| 🧪 | Capture, paste, recovery, and undo against a test provider |
| ❌ | Live Enshrouded World provider |
| 🚧 | Wand action and target selection |
| 🚧 | Selection and placement overlay |

ShroudEdit is an external World Editor mod that depends only on published Shroudtopia
headers and services.

```text
Wand → Point A → Point B → Volume → Capture
     → Blueprint → Target → Transform → Preview → Paste
```

ShroudEdit 0.2.0 provides selection geometry, Blueprint schema 1.1, validation,
file storage, rotated placement plans, capture, paste, recovery, and undo logic.

Live integration requires target snapshots, semantic tool actions, entity operations,
voxel and terrain operations, a world overlay, game-thread jobs, persistence, and
server authorization. The production `shroudtopia.world` provider is not registered.
ShroudEdit therefore returns `RESULT_UNSUPPORTED` for live capture and paste.
Unknown Blueprint cells remain unknown and are never treated as empty space.
