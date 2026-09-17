# Installieren und konfigurieren

Shroudtopia zielt auf Windows x64. Loader-Dateien wie im Release beschrieben ablegen, dann jede Mod-DLL mit ihrer `mod.json` in ein eigenes Mod-Verzeichnis legen. Konfiguration wird aus `shroudtopia.json` gelesen; aktuelle Logs sind `shroudtopia.log` und `enshrouded.log`.

Ein Mod-Manifest sollte eine global eindeutige ID verwenden und jede geschützte Capability unter `requires.capabilities` nennen. Fehlende Capabilities sollten möglichst nur das abhängige Feature deaktivieren. Bei Fehlern in Discovery, Laden oder Permission-Prüfung zuerst das aktuelle Log lesen.
