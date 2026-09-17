# Repository Conventions

| Path | Purpose |
|---|---|
| `src/bootstrap/` | Bootstrap proxy |
| `src/loader/` | Loader core |
| `src/engine/` | Internal asset engine |
| `api/include/` | Public C API |
| `mods/native/` | Bundled API-only mods |
| `docs/` | mdBook source |
| `tools/` | Independent verification tools |
| `build.ps1` | Build, test, and package entry point |
| `VERSION` | Product-version source |

## Naming rules

| Item | Rule | Example |
|---|---|---|
| Directory | `lower-kebab-case` | `no-fall-damage` |
| Public ABI symbol | `ST_` prefix | `ST_Result` |
| Mod ID | lowercase dotted ID | `mod.no-fall-damage` |
| Service ID | lowercase dotted ID | `shroudtopia.logging.read` |
| Product version | `MAJOR.MINOR.PATCH` | `1.0.0` |
| Release tag | `vMAJOR.MINOR.PATCH` | `v1.0.0` |

Mods include headers from `api/include` only. Generated directories such as
`build/`, `x64/`, `site/`, and Rust `target/` are not source. User-facing text and
first-party documentation are written in English.
