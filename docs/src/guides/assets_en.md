# Edit assets safely

1. Declare `shroudtopia.assets.read` and/or `.write` in `mod.json`.
2. Use `api->assets` directly and verify that the domain is available.
3. Use `list` to obtain typed keys.
4. Use a zero-capacity call to size JSON storage, then read it.
5. Prefer `set` for one existing JSON Pointer; disjoint fields compose.
6. Call `save` to publish through staging and recovery, or `reset` to remove your overlay.

Full-document ownership and overlapping parent/child fields conflict with another owner and return `RESULT_CONFLICT`. `save` confirms file publication, not live reload of resources already held by the game. See the complete [Asset API](../reference/api.md).
