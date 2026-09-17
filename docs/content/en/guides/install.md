# Install and configure

Shroudtopia targets Windows x64. Extract the release, copy only the contents of `game/` next to `enshrouded.exe` or `enshrouded_server.exe`, and keep `licenses/` outside the game directory. The game folder receives `winmm.dll`, `shroudtopia.dll`, and `mods/`; it does not need license or notice files. Configuration is read from `shroudtopia.json`; current logs are `shroudtopia.log` and `enshrouded.log`.

A mod manifest should use a globally unique ID and list every protected capability under `requires.capabilities`. Missing capabilities must disable only the dependent feature where possible. Inspect the current log first when discovery, loading, or a permission check fails.
