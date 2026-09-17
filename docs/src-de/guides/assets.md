# Assets sicher bearbeiten

1. `shroudtopia.assets.read` und/oder `.write` in `mod.json` deklarieren.
2. `shroudtopia.assets@1.1` finden und die Tabelle validieren.
3. Mit `visit_resources` typisierte Schlüssel erhalten.
4. JSON-Speicher zuerst mit Zero-Capacity dimensionieren, dann lesen.
5. Für einen existierenden JSON Pointer bevorzugt `set_resource_field_json` nutzen; disjunkte Felder kombinieren sich.
6. Mit `flush` über Staging und Recovery publizieren oder mit `discard_changes` das eigene Overlay entfernen.

Ownership vollständiger Dokumente und überlappende Parent-/Child-Felder kollidieren mit anderem Owner und liefern `ST_RESULT_ALREADY_EXISTS`. `flush` bestätigt die Dateipublikation, nicht das Live-Reload bereits vom Spiel gehaltener Ressourcen. Siehe vollständige [Asset API](../reference/generated/assets-v1.1.md).
