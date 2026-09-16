# Game Binary Analysis

## Current profile

The current Enshrouded client and dedicated-server files are native PE x64 binaries.
Executable sections, imports, relocations, exception tables, and embedded reflection
data are available for static analysis.

| Process | SHA-256 |
|---|---|
| Client | `AF2F5A1227911D8AA06B3908D6BD0211838211CAE14EA91099CB57D0DF990781` |
| Dedicated server | `001C1B40ED091D8C1AEE583ADDE3800D7C858AE2C7F4DFF54FCA2938B2BE1637` |

## Verification workflow

```text
Hash executable and extract reflection
                 ↓
Locate complete function and caller context
                 ↓
Validate instruction boundaries and expected bytes
                 ↓
Capture read-only runtime data through an internal probe
                 ↓
Verify thread, lifetime, coordinates, and invariants
                 ↓
Perform one reversible write in a disposable world
                 ↓
Read back, undo, save, reload, and verify
                 ↓
Register provider and expose capability
```

## Current World candidates

| Status | Client RVA | Evidence |
|:---:|---:|---|
| 🚧 | `0x249CD8` | Cursor snapshot access aligned with `ClientCursorInput` reflection |
| 🚧 | `0x3E5610` | Pickaxe action context |
| 🚧 | `0x994810` | Bounded voxel buffer operation |
| 🚧 | `0x997EA0` | Hit-environment query candidate |
| 🚧 | `0x362510` | Terraforming execution candidate |
| 🚧 | `0x241716` | Preview mesh reference candidate |
| 🚧 | `0x23A067` | Prop inspection candidate |

A unique byte match confirms location only. It does not confirm arguments, ownership,
thread safety, persistence, or gameplay effect. Client addresses are not reused for
the dedicated server.

## Reflection extraction

```powershell
cargo run --release --manifest-path src/engine/Cargo.toml \
  -p shroudtopia-assets --example inspect_world_types -- \
  "<game-directory>\enshrouded.exe"
```

The tool writes reflection JSON to stdout and does not modify game files.

## Probe rules

- Bind every probe to a full executable fingerprint and expected bytes.
- Prefer a verified function boundary over an arbitrary mid-function hook.
- Copy fixed POD snapshots into a preallocated ring buffer.
- Do not log, allocate, open files, update UI, or call mods inside a hook.
- Drain and format telemetry on the loader worker.
- Disable a capability when any profile check fails.
