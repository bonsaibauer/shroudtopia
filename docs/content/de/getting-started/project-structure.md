# Projektstruktur

Die vollständige öffentliche ABI liegt in `api/shroudtopia.h`, native Beispiele unter `mods`, die Loader-Implementierung unter `src/loader` und englische Dokumentationsquellen unter `docs/content/en`.

Die Header sind der ABI-Vertrag. Implementierungsdetails gehören nicht in öffentliche Header. Registrierungen und besitzgebundene Ressourcen müssen beim Deaktivieren oder Entladen freigegeben werden.
