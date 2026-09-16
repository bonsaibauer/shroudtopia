# Install Mods

Each mod uses one directory:

```text
mods/
└── mod.author.mod-name/
    ├── mod.json
    └── mod-name.dll
```

Shroudtopia loads directories with a valid manifest. The manifest ID must match the
descriptor ID exported by the DLL. DLL files placed directly in `mods/` are ignored.

## Process targets

| Manifest value | Loader behavior |
|---|---|
| `"target": "client"` | Load only in `enshrouded.exe`. |
| `"target": "server"` | Load only in `enshrouded_server.exe`. |
| `"target": "both"` | Load in either process. |

The `target` field controls where the DLL may load. It does not grant server
authority or guarantee that installing a mod on one side changes the other side.

## Activation

```json
{
  "mods": {
    "mod.author.mod-name": {
      "active": true
    }
  }
}
```

Disabling a mod invokes its deactivate callback. Unloading releases all resources
registered under its owner ID.
