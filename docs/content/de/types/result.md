# Result

Jede fehlschlagbare API-Operation liefert `Result`. `RESULT_OK` kennzeichnet Erfolg; jeder andere Wert kennzeichnet einen Fehler.

| Wert | Bedeutung |
|---|---|
| `RESULT_OK` | Operation erfolgreich abgeschlossen. |
| `RESULT_INVALID_ARGUMENT` | Wert, Pointer, Strukturgröße oder Dokument ist ungültig. |
| `RESULT_CONFLICT` | Ownership oder Änderung kollidiert mit dem aktuellen Zustand. |
| `RESULT_NOT_FOUND` | Ressource oder Provider existiert nicht. |
| `RESULT_VERSION_MISMATCH` | Die angeforderte Version entspricht nicht API 1.1. |
| `RESULT_PERMISSION_DENIED` | Erforderliche Berechtigung fehlt. |
| `RESULT_NOT_AVAILABLE` | Feature ist im aktuellen Zustand nicht verfügbar. |
| `RESULT_CALLBACK_FAILED` | Consumer-Callback ist fehlgeschlagen. |
| `RESULT_INTERNAL_ERROR` | Loader konnte eine gültige Operation nicht abschließen. |
