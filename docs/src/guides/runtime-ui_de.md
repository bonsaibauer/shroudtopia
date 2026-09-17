# Runtime Patches und Text UI

Runtime Patches benötigen `shroudtopia.runtime.patches`. Signaturen gegen jedes unterstützte Game-Profil validieren und genau einen Treffer im ausführbaren Bereich fordern. `overwrite_size` muss vollständige Instruktionen abdecken; automatische Instruktions-Relocation ist nicht verfügbar. Inaktiv erstellen, bewusst aktivieren und zum Wiederherstellen der Originalbytes freigeben.

Der Text-UI-Service erstellt asynchron native Fenster mit einem bis acht Tabs. `get_status` pollen, Tab-Snapshots mit höchstens einem MiB UTF-8 ersetzen und das Handle beim Cleanup zerstören. Unterstützt werden Windowed-/Borderless-Windows-Clients, nicht exklusives Fullscreen. Der Service ruft niemals in die Mod zurück.
