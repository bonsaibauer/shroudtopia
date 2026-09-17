# Edit assets safely

1. Declare `shroudtopia.assets.read` and/or `.write` in `mod.json`.
2. Find `shroudtopia.assets@1.1` and validate the table.
3. Use `visit_resources` to obtain typed keys.
4. Use a zero-capacity call to size JSON storage, then read it.
5. Prefer `set_resource_field_json` for one existing JSON Pointer; disjoint fields compose.
6. Call `flush` to publish through staging and recovery, or `discard_changes` to remove your overlay.

Full-document ownership and overlapping parent/child fields conflict with another owner and return `ST_RESULT_ALREADY_EXISTS`. `flush` confirms file publication, not live reload of resources already held by the game. See the complete [Asset API](../reference/generated/assets-v1.1.md).
