# Threads, Callbacks und Fehler

Lifecycle-`on_update` läuft auf Shroudtopias Worker-Thread, nicht auf dem Game-Thread. Keine ungeprüften Engine-Funktionen aufrufen. Event- und Command-Callbacks laufen synchron auf dem aufrufenden Thread. Callbacks begrenzen und niemals Exceptions über die ABI lassen.

Das spezifische `ST_Result` behandeln: `NOT_FOUND` und `UNSUPPORTED` sind oft erwartete Feature-Zustände; `PERMISSION_DENIED` benötigt Manifest-/Nutzeraktion; `CALLBACK_FAILED` kennzeichnet Extension-Code; `INTERNAL_ERROR` bleibt einem Hostfehler vorbehalten. Nicht jeden Fehler in `INTERNAL_ERROR` umwandeln.

Ein fehlgeschlagener Grid-Write darf partiell sein. Vor destruktiven Operationen Recovery-Daten sichern.
