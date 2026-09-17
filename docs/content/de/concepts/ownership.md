# Ownership, Buffer und Lifetime

Gehe davon aus, dass jeder Input-Pointer und `StringView` nur für den Aufruf geliehen ist, sofern die Funktion kein Kopieren zusichert. Ein View ist eine Byte-Spanne und nicht zwingend NUL-terminiert. `data` darf nur bei `size == 0` null sein.

## Output-Buffer

APIs mit Größenabfrage werden zweimal aufgerufen: zuerst mit null/Zero-Capacity die benötigte Größe lesen, Speicher im Besitz des Aufrufers anlegen und erneut aufrufen. Den exakten Function-Contract beachten: Output besitzt häufig keinen NUL-Terminator.

## Handles und Cleanup

`Registration`, `RuntimePatch` und `TextWindow` sind opake, Owner-gebundene Handles. Einzelne Handles werden freigegeben, sobald sie nicht mehr benötigt werden. Der Loader entfernt nach dem Entladen einer Mod automatisch alle verbliebenen Ressourcen. Ein Handle nie nach Release oder mit anderem Owner verwenden.
