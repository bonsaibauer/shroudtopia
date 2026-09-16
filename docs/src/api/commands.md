# Commands

## Status

| Status | Feature |
|:---:|---|
| ✅ | Global command registration |
| ✅ | Command execution through the API |
| ✅ | Owner cleanup |
| ❌ | Enshrouded chat input |

```cpp
ST_CommandDescriptor command{
    sizeof(command),
    View("author.mod.echo"),
    View("Write text to the log"),
    ExecuteEcho,
    state
};
```

Command IDs are globally unique. The callback receives a borrowed argument string.
The registry is independent of its input source. The bundled Commands mod routes
calls through the same API.

Argument schemas, automatic help, caller identity, cheat policy, and server
authorization are not available.
