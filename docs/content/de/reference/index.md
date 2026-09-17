# API-Referenz

Die Referenz wird aus öffentlichen C-Headern und validierten Dokumentationsmetadaten erzeugt. Bezeichner und Signaturen stammen aus den Headern; Semantik, Ownership, Threading und Beispiele sind Vertragsmetadaten.

| Service | Aufgabe | Status | Verfügbar seit |
|---|---|---|---|
| [Actions](./actions.md) | Stellt auffindbare Operationen mit JSON-Schema-Eingaben und beobachtbarem Status bereit. | Stable | API 1.1 |
| [API](./api.md) | Handelt die einzige flache Shroudtopia-Schnittstelle aus, die jede native Mod verwendet. | Stable | API 1.1 |
| [Assets](./assets.md) | Listet, liest, erstellt, ändert, patcht, speichert und verwirft strukturierte Spiel-Assets. | Stable | API 1.1 |
| [Capability-Informationen](./capabilities.md) | Beschreibt ausgehandelte Version und Verfügbarkeit einer Capability. | Stable | API 1.1 |
| [Commands](./commands.md) | Registriert benannte Commands mit Beschreibungen und Callbacks. | Stable | API 1.1 |
| [Core API](./core.md) | Definiert ABI-Version, gemeinsame String- und Registrierungs-Handles sowie das Result-Fehlermodell. | Stable | API 1.1 |
| [Events](./events.md) | Veröffentlicht typisierte Event-Payloads und verwaltet Event-Abonnements. | Stable | API 1.1 |
| [Mod-Lebenszyklus](./lifecycle.md) | Deklariert eine Mod und ihre Callbacks für Laden, Aktivierung, Update, Deaktivierung und Entladen. | Stable | API 1.1 |
| [Logging](./logging.md) | Schreibt strukturierte Loader-Meldungen und liest begrenzte Ausschnitte aus Loader- oder Game-Logs. | Stable | API 1.1 |
| [Runtime Patches](./runtime-patches.md) | Erstellt und steuert besitzergebundene direkte oder Detour-Runtime-Patches. | Experimental | API 1.1 |
| [Service Discovery](./services.md) | Veröffentlicht Erweiterungsverträge und ermittelt Provider anhand der exakten Contract-Version. | Stable | API 1.1 |
| [Settings](./settings.md) | Liest typisierte Werte aus der Konfiguration der aufrufenden Mod. | Stable | API 1.1 |
| [Text UI](./ui.md) | Erstellt native Textfenster, aktualisiert Tab-Inhalte, beobachtet die Bereitschaft und zerstört Fenster. | Experimental | API 1.1 |
