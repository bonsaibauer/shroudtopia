# API Explorer

Untersuche jede API-Funktion, bearbeite JSON-Argumente, validiere sie und führe unterstützte Asset-API-Aufrufe gegen deine eigenen lokalen Enshrouded-Dateien aus. Repository und öffentliche Dokumentation enthalten keine Spiel-EXE, KFC-Container, Ressourcendaten oder extrahierten Spielinhalte.

<div id="api-explorer" class="api-explorer" data-mode="local"></div>

## Mit echten lokalen Daten starten

```powershell
npm run docs:check
npm run sandbox:build
npm run docs:serve -- --game-dir "C:\Program Files (x86)\Steam\steamapps\common\Enshrouded"
```

Öffne `http://127.0.0.1:43117/de/api-explorer.html`. Der Host bindet ausschließlich an `127.0.0.1` und startet einen persistenten nativen Inspector, der Typinformationen aus der EXE und die passenden KFC-Dateien liest. Antworten werden vollständig und ohne Beispieldaten, Filterung oder Kürzung zurückgegeben.

Standardmäßig sind nur Lesezugriffe aktiv. Ergänze `--allow-write`, um `update_asset`, `set_asset_field`, `create_asset`, `reset_assets` und `save_assets` freizuschalten. Änderungen bleiben bis zum Aufruf von `save_assets` vorgemerkt. Beim Speichern werden alle geänderten Ressourcen validiert, `.shroudtopia.bak`-Kopien bewahrt, ein Transaktionsverzeichnis beschrieben, das Ergebnis geprüft und erst danach das aktive Dateipaar ersetzt. Beende vorher das Spiel und behalte eine eigene Sicherung.

Funktionen, die den laufenden In-Game-Loader benötigen – etwa Runtime Patches, UI, Commands und Events – liefern im Offline-Explorer `RESULT_NOT_AVAILABLE`.
