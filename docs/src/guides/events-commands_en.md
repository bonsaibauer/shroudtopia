# Events and commands

Register subscriptions and commands in `on_load`, retain their `Registration` handles, and release them during `on_unload`. Event payloads and command argument strings are borrowed only for the callback. Dispatch is synchronous, so never retain pointers and avoid blocking work.

Use globally namespaced IDs such as `author.mod.event.changed` and `author.mod.command`. Event publishers own and version their payload structs. The API currently provides no schema registry, queue, command argument schema, cheat policy, or server authorization.
