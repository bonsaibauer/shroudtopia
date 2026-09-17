# Ownership, Buffer und Lifetime

Gehe davon aus, dass jeder Input-Pointer und `ST_StringView` nur für den Aufruf geliehen ist, sofern die Funktion kein Kopieren zusichert. Ein View ist eine Byte-Spanne und nicht zwingend NUL-terminiert. `data` darf nur bei `size == 0` null sein.

## Output-Buffer

APIs mit Größenabfrage werden zweimal aufgerufen: zuerst mit null/Zero-Capacity die benötigte Größe lesen, Speicher im Besitz des Aufrufers anlegen und erneut aufrufen. Den exakten Function-Contract beachten: Output besitzt häufig keinen NUL-Terminator.

## Handles und Cleanup

`ST_Registration`, `ST_RuntimePatch` und `ST_TextWindow` sind opake, Owner-gebundene Handles. Einzelne Handles möglichst direkt freigeben und beim Unload `release_owner` als Sicherheitsnetz aufrufen. Ein Handle nie nach Release oder mit anderem Owner verwenden.
