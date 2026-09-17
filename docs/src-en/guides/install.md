# Install and configure

Shroudtopia targets Windows x64. Place the loader files as described by the release, then put each mod DLL and its `mod.json` in one mod directory. Configuration is read from `shroudtopia.json`; current logs are `shroudtopia.log` and `enshrouded.log`.

A mod manifest should use a globally unique ID and list every protected capability under `requires.capabilities`. Missing capabilities must disable only the dependent feature where possible. Inspect the current log first when discovery, loading, or a permission check fails.
