# World API

The `shroudtopia.world 2.1` contract is experimental and currently has only a controlled ShroudEdit test provider. Production mods must handle `NOT_FOUND` and `NOT_AVAILABLE`.

Entity handles are bound to ID, generation, and session. Regions are half-open. Grid buffers use `x + width * (y + height * z)`. Coverage is explicit: `UNKNOWN` is not empty and writers leave unknown cells unchanged. A failed write may be partial, so retain recovery data.

Use `GridRegionFromPoints` for pure, bounded conversion of two included points into aligned half-open bounds. It uses `floor` for negative coordinates and enforces a caller-supplied cell budget.
