# Commands

Create a `CommandDescriptor`, initialize `struct_size`, provide a stable command ID and callback, then call `Api::register_command`. Retain the returned `Registration` and release it when the mod deactivates.

See the [Commands reference](../reference/commands.md) for every field and result code.
