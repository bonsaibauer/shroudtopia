# World API

Der Contract `shroudtopia.world 2.1` ist experimentell und besitzt derzeit nur einen kontrollierten ShroudEdit-Test-Provider. Production-Mods müssen `NOT_FOUND` und `NOT_AVAILABLE` behandeln.

Entity-Handles sind an ID, Generation und Session gebunden. Regionen sind halboffen. Grid-Buffer verwenden `x + width * (y + height * z)`. Coverage ist explizit: `UNKNOWN` ist nicht leer und Writer lassen unbekannte Zellen unverändert. Ein fehlgeschlagener Write darf partiell sein; Recovery-Daten behalten.

`GridRegionFromPoints` konvertiert zwei inkludierte Punkte rein und begrenzt in ausgerichtete halboffene Grenzen. Die Funktion nutzt `floor` für negative Koordinaten und erzwingt ein vom Aufrufer angegebenes Zellbudget.
