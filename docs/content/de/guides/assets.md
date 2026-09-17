# Assets sicher bearbeiten

1. `shroudtopia.assets.read` und/oder `.write` in `mod.json` deklarieren.
2. `api->assets` direkt verwenden und die Verfügbarkeit der Domain prüfen.
3. Mit `list` typisierte Schlüssel erhalten.
4. JSON-Speicher zuerst mit Zero-Capacity dimensionieren, dann lesen.
5. Für einen existierenden JSON Pointer bevorzugt `set` nutzen; disjunkte Felder kombinieren sich.
6. Mit `save` über Staging und Recovery publizieren oder mit `reset` das eigene Overlay entfernen.

Ownership vollständiger Dokumente und überlappende Parent-/Child-Felder kollidieren mit anderem Owner und liefern `RESULT_CONFLICT`. `save` bestätigt die Dateipublikation, nicht das Live-Reload bereits vom Spiel gehaltener Ressourcen. Siehe vollständige [Asset API](../reference/assets.md).

Die Engine lässt sich mit lokal installierten Spieldateien prüfen:

```powershell
npm run sandbox:build
npm run sandbox:verify -- "C:\Program Files (x86)\Steam\steamapps\common\Enshrouded" enshrouded
```

Der Prüflauf öffnet die lokalen KFC-Dateien über `shroudtopia.dll`, listet echte Rezeptressourcen, liest deren vollständiges JSON und prüft Ownership sowie Feldänderungen im Speicher. `--roundtrip` darf nur mit einer kopierten Testdatei unterhalb des `build`-Ordners dieses Repositories verwendet werden; dann wird die Testdatei geschrieben und erneut geöffnet.
