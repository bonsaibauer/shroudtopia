# Runtime Patch API

## Status

| Status | Feature |
|:---:|---|
| ✅ | Unique executable-section signature scan |
| ✅ | Direct byte replacement |
| ✅ | Near x64 detour with relative return relocation |
| ✅ | Owner cleanup and original-byte restoration |
| ❌ | Automatic instruction relocation |

`shroudtopia.runtime.patches@1.0` provides a controlled mechanism. Feature-specific
signatures and payloads remain in the mod.

```text
Manifest permission
       ↓
find_service(shroudtopia.runtime.patches@1.0)
       ↓
create → enable/disable → inspect → release
```

| Function | Purpose |
|---|---|
| `create` | Validate a descriptor and create an inactive owner-bound patch. |
| `set_enabled` | Apply replacement bytes or restore original bytes. |
| `get_state` | Return the current enabled state. |
| `release` | Restore and destroy one patch handle. |

The scanner rejects missing and ambiguous signatures. Overlapping active patch
ranges are rejected. A mod validates complete overwritten instructions, payload,
overwrite length, target process, and every supported game profile.
