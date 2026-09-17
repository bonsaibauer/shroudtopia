# Build and Test

## Requirements

| Tool | Purpose |
|---|---|
| Visual Studio 2022 C++ toolchain | Build x64 native DLLs and tests. |
| Rust toolchain | Build and test the internal asset engine. |
| mdBook | Build the documentation site. |

## Full build

```powershell
git clone --recurse-submodules <repository-url>
cd shroudtopia
.\build.ps1
```

The script builds `winmm.dll`, `shroudtopia.dll`, `shroudtopia-assets.dll`, all
bundled mods, native smoke tests, Rust tests, and one installable ZIP under `build/`.

## Asset engine test

```powershell
python tools/asset-engine-smoke.py src/engine/target/release/shroudtopia_assets.dll "<game-directory>" enshrouded
```

Use a controlled copy of game resources for write tests.

## Documentation

```powershell
mdbook build docs
mdbook test docs
```

| Check | Expected result |
|---|---|
| C++ build | 0 errors and 0 project warnings |
| Rust tests | All tests pass |
| Native smoke tests | All contracts and lifecycle checks pass |
| mdBook | All pages and links build successfully |
