# Edit assets safely

1. Declare `shroudtopia.assets.read` and/or `.write` in `mod.json`.
2. Use `api->assets` directly and verify that the domain is available.
3. Use `list` to obtain typed keys.
4. Use a zero-capacity call to size JSON storage, then read it.
5. Prefer `set` for one existing JSON Pointer; disjoint fields compose.
6. Call `save` to publish through staging and recovery, or `reset` to remove your overlay.

Full-document ownership and overlapping parent/child fields conflict with another owner and return `RESULT_CONFLICT`. `save` confirms file publication, not live reload of resources already held by the game. See the complete [Asset API](../reference/assets.md).

Verify the engine against locally installed game files:

```powershell
npm run sandbox:build
npm run sandbox:verify -- "C:\Program Files (x86)\Steam\steamapps\common\Enshrouded" enshrouded
```

The verifier opens the local KFC files through `shroudtopia.dll`, lists real recipe resources, reads their complete JSON, and tests ownership and field edits in memory. Add `--roundtrip` only with a copied fixture below this repository's `build` directory; it writes the fixture and verifies that it can be reopened.
