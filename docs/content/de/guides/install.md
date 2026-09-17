# Installieren und konfigurieren

Shroudtopia zielt auf Windows x64. Entpacke das Release, kopiere nur den Inhalt von `game/` neben `enshrouded.exe` oder `enshrouded_server.exe` und bewahre `licenses/` außerhalb des Spielordners auf. In den Spielordner kommen `winmm.dll`, `shroudtopia.dll` und `mods/`; Lizenz- oder Notice-Dateien werden dort nicht benötigt. Konfiguration wird aus `shroudtopia.json` gelesen; aktuelle Logs sind `shroudtopia.log` und `enshrouded.log`.

Ein Mod-Manifest sollte eine global eindeutige ID verwenden und jede geschützte Capability unter `requires.capabilities` nennen. Fehlende Capabilities sollten möglichst nur das abhängige Feature deaktivieren. Bei Fehlern in Discovery, Laden oder Permission-Prüfung zuerst das aktuelle Log lesen.
