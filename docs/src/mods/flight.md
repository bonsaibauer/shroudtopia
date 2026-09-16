# Shroudtopia Flight

| Property | Value |
|---|---|
| Mod ID | `mod.flight` |
| Target | `client` |
| Default | Off |
| Capability | `shroudtopia.runtime.patches` |
| Local client effect | ✅ |

Enable the mod in `shroudtopia.json`:

```json
"mod.flight": {
  "active": true
}
```

Use the glider and steer upward. The mod enables its owner-bound detour only when the
signature matches uniquely. Deactivation restores the original bytes. A missing or
ambiguous signature is logged and leaves memory unchanged.
