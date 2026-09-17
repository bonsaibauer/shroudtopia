# Project structure

The complete public ABI lives in `api/shroudtopia.h`, native examples in `mods`, the loader implementation in `src/loader`, and documentation sources in `docs/content/en`.

Treat headers as the ABI contract. Keep implementation details outside public headers and release every registration or owned resource during deactivation or unload.
