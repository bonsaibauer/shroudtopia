# Commands

Erzeuge einen `CommandDescriptor`, initialisiere `struct_size`, setze eine stabile Command-ID und einen Callback und rufe anschließend `Api::register_command` auf. Bewahre die zurückgegebene `Registration` auf und gib sie beim Deaktivieren der Mod frei.

Alle Felder und Result-Codes stehen in der [Commands-Referenz](../reference/commands.md).
