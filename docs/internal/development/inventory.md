# Repository Inventory

| Status | Path | Content |
|:---:|---|---|
| ✅ | `src/bootstrap/` | WinMM proxy and loader start |
| ✅ | `src/loader/` | Runtime, discovery, lifecycle, API, configuration, logging, UI, and patches |
| ✅ | `src/engine/` | Internal KFC3 reflection and resource engine |
| ✅ | `api/include/` | Published C API headers |
| ✅ | `mods/native/` | Bundled mods that use the public API |
| ✅ | `tools/` | API, mod, logging, patch, and asset verification tools |
| ✅ | `docs/` | mdBook source |
| ✅ | `build.ps1` | Build, test, and package entry point |
| ✅ | `VERSION` | Single product-version source |
| ✅ | `third-party/nlohmann-json/` | JSON dependency |
| ✅ | `third-party/kfc-parser/` | Internal KFC3 parser source |

`build/`, `x64/`, `site/`, and Rust `target/` are generated outputs. The build keeps
one current release archive. Logging has one implementation in `src/loader/utils.cpp`.
Public headers are grouped by API domain.
