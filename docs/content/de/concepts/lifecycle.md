# Mod-Lebenszyklus

`CreateModFunction` beschreibt eine Mod über `ModDescriptor`. Der Loader ruft `on_load`, `on_activate`, `on_update`, `on_deactivate` und `on_unload` in dieser Lebenszyklusreihenfolge auf.

Langlebige Registrierungen werden beim Laden oder Aktivieren angelegt und beim Deaktivieren freigegeben. `on_unload` muss wiederholbar sicher sein. C++-Exceptions dürfen die C-ABI-Grenze niemals überschreiten.
