<!-- Generated from docs/contracts/logging-v1.0.yaml; do not edit by hand. -->
# Logging API

Owner-markiertes Schreiben über die Host API und begrenztes Lesen aktueller Logs.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia Loader
- **Capability:** Keine
- **Header:** `api/include/shroudtopia/api/logging.h`

</div>

## Typen

### `ST_LogLevel`

Schweregrad der Meldung.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_LOG_TRACE` | `0` | Fokussierte hochfrequente Diagnose. | — |
| `ST_LOG_DEBUG` | `1` | Technische Entscheidungen und Zustand. | — |
| `ST_LOG_INFO` | `2` | Erfolgreiche nutzerrelevante Änderung. | — |
| `ST_LOG_WARNING` | `3` | Eingeschränkte oder nicht unterstützte Operation. | — |
| `ST_LOG_ERROR` | `4` | Angeforderte Operation schlug fehl. | — |

### `ST_LogSourceV1`

Lesbare aktuelle Logquelle.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_LOG_SOURCE_LOADER` | `0` | Shroudtopia- und Mod-Log. | — |
| `ST_LOG_SOURCE_GAME` | `1` | Enshrouded-Spiel-Log. | — |

### `ST_LogReadApiV1`

Tabelle zum Lesen aktueller Logs.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `abi_version` | `uint32_t` | Muss ST_ABI_VERSION_1 entsprechen. | — |
| `read_tail` | `function pointer` | Begrenzter Tail-Reader. | — |

## Funktionen

<section class="api-function" data-api-name="read_tail" data-api-status="stable">

### `read_tail`

Höchstens ein MiB vom Ende des aktuellen Logs lesen.

```c
ST_Result read_tail(ST_LogSourceV1 source, char* buffer, size_t capacity, size_t* written);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** Keine
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `source` | `ST_LogSourceV1` | in | ja | Loader- oder Spiel-Log. |
| `buffer` | `char*` | out | ja | Byte-Buffer im Besitz des Aufrufers. |
| `capacity` | `size_t` | in | ja | Zwischen 1 und 1.048.576 Byte. |
| `written` | `size_t*` | out | ja | Empfängt geschriebene Bytes; kein Terminator. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_NOT_FOUND` | Die aktuelle Logdatei existiert nicht. |

#### Beispiel

```cpp
// Initialize source, buffer, capacity, written according to the parameter table.
const ST_Result status = api->read_tail(source, buffer, capacity, written);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
