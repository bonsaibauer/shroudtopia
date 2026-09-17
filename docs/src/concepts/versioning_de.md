# API-Versionierung

Binde `shroudtopia/api.h` ein, exportiere `CreateMod` und fordere `API_VERSION` an. Der Loader akzeptiert die aktuelle Hauptversion und gibt bei jeder anderen Hauptversion `RESULT_VERSION_MISMATCH` zurück.

```c
if (requested_api_version != API_VERSION) {
    return RESULT_VERSION_MISMATCH;
}
```

Alle öffentlichen Funktionen verwenden `CALL`. Eine Änderung an einem öffentlichen Typ, einer Funktionssignatur, einer Konstante oder einer Lifecycle-Regel erhöht die Hauptversion. Neue Funktionen erhöhen die Nebenversion; Implementierungsfehler werden über die Patch-Version korrigiert.
