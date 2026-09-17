# Events und Commands

Subscriptions und Commands in `on_load` registrieren, ihre `ST_Registration`-Handles behalten und in `on_unload` freigeben. Event-Payloads und Command-Argumentstrings sind nur für den Callback geliehen. Dispatch ist synchron: Pointer niemals behalten und blockierende Arbeit vermeiden.

Global namespacete IDs wie `author.mod.event.changed` und `author.mod.command` verwenden. Event-Publisher besitzen und versionieren ihre Payload-Strukturen. Die API bietet derzeit keine Schema-Registry, Queue, Command-Argumentschemas, Cheat-Policy oder Server-Autorisierung.
