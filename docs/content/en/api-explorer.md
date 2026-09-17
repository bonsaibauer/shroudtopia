# API Explorer

Inspect every API function, edit JSON arguments, validate them, and execute supported Asset API calls against your own local Enshrouded files. The repository and public documentation contain no game executable, KFC container, resource data, or extracted game content.

<div id="api-explorer" class="api-explorer" data-mode="local"></div>

## Start with real local data

```powershell
npm run docs:check
npm run sandbox:build
npm run docs:serve -- --game-dir "C:\Program Files (x86)\Steam\steamapps\common\Enshrouded"
```

Open `http://127.0.0.1:43117/en/api-explorer.html`. The host binds only to `127.0.0.1`; it starts a persistent native inspector that reads the executable's type information and the matching KFC files. Responses are returned completely without example data, filtering, or truncation.

Reading is the default. To enable `update_asset`, `set_asset_field`, `create_asset`, `reset_assets`, and `save_assets`, add `--allow-write`. Edits remain staged until `save_assets` is called. Saving validates all edited resources, preserves `.shroudtopia.bak` copies, writes into a transaction stage, verifies the resulting KFC, and only then replaces the active pair. Close the game first and keep your own backup.

Functions that require the running in-game loader—such as runtime patches, UI, commands, and events—return `RESULT_NOT_AVAILABLE` in this offline explorer.
