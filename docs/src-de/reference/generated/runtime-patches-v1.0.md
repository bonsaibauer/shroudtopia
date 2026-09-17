<!-- Generated from docs/contracts/runtime-patches-v1.0.yaml; do not edit by hand. -->
# Runtime Patch API

Kontrollierte signaturbasierte direkte Patches und x64-Detours.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia Loader
- **Capability:** `shroudtopia.runtime.patches`
- **Header:** `api/include/shroudtopia/api/runtime.h`

</div>

## Typen

### `ST_RuntimePatch`

Opakes Owner-gebundenes Patch-Handle.

### `ST_RuntimePatchKindV1`

Strategie zur Patch-Anwendung.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_RUNTIME_PATCH_DIRECT` | `0` | Direkte Ersatzbytes. | — |
| `ST_RUNTIME_PATCH_DETOUR` | `1` | Naher x64-Detour. | — |

### `ST_RuntimeRelocationKindV1`

Unterstützte Payload-Relocation.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_RUNTIME_RELOCATION_REL32_RETURN` | `1` | Relative 32-Bit-Return-Distanz schreiben. | — |

### `ST_RuntimeRelocationV1`

Payload-Relocation-Anweisung.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `payload_offset` | `size_t` | Offset innerhalb des Payloads. | — |
| `kind` | `ST_RuntimeRelocationKindV1` | Unterstützte Relocation-Art. | — |

### `ST_RuntimePatchDescriptorV1`

Vollständige Definition eines inaktiven Patches.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `signature` | `ST_StringView` | Byte-Signatur im ausführbaren Bereich. | — |
| `match_offset` | `int64_t` | Vorzeichenbehafteter Offset vom eindeutigen Treffer. | — |
| `kind` | `ST_RuntimePatchKindV1` | Direkt oder Detour. | — |
| `overwrite_size` | `size_t` | Vollständige Zahl überschriebener Instruktionsbytes. | — |
| `payload` | `const uint8_t*` | Ersatz-Payload. | — |
| `payload_size` | `size_t` | Payload-Bytes. | — |
| `relocations` | `const ST_RuntimeRelocationV1*` | Relocation-Array. | — |
| `relocation_count` | `size_t` | Anzahl Relocations. | — |

### `ST_RuntimePatchStateV1`

Aktueller Patch-Zustand.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `enabled` | `uint8_t` | Eins, wenn angewandt. | — |
| `reserved` | `uint8_t[7]` | Reserviert, null. | — |

### `ST_RuntimePatchesApiV1`

Versionierte Service-Funktionstabelle.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `create` | `function pointer` | create. | — |
| `set_enabled` | `function pointer` | set enabled. | — |
| `get_state` | `function pointer` | get state. | — |
| `release` | `function pointer` | release. | — |

## Funktionen

<section class="api-function" data-api-name="create" data-api-status="stable">

### `create`

Einen inaktiven Patch validieren und erstellen.

```c
ST_Result create(ST_StringView owner_id, const ST_RuntimePatchDescriptorV1* descriptor, ST_RuntimePatch* patch);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.runtime.patches`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Patch-Owner. |
| `descriptor` | `const ST_RuntimePatchDescriptorV1*` | in | ja | Vollständige Patch-Definition. |
| `patch` | `ST_RuntimePatch*` | out | ja | Empfängt das Patch-Handle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, descriptor, patch according to the parameter table.
const ST_Result status = api->create(owner_id, descriptor, patch);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_enabled" data-api-status="stable">

### `set_enabled`

Patch-Bytes anwenden oder wiederherstellen.

```c
ST_Result set_enabled(ST_StringView owner_id, ST_RuntimePatch patch, uint8_t enabled);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.runtime.patches`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Patch-Owner. |
| `patch` | `ST_RuntimePatch` | in | ja | Eigener Patch. |
| `enabled` | `uint8_t` | in | ja | Eins zum Anwenden, null zum Wiederherstellen. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, patch, enabled according to the parameter table.
const ST_Result status = api->set_enabled(owner_id, patch, enabled);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_state" data-api-status="stable">

### `get_state`

Lesen, ob ein Patch aktiv ist.

```c
ST_Result get_state(ST_StringView owner_id, ST_RuntimePatch patch, ST_RuntimePatchStateV1* state);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.runtime.patches`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Patch-Owner. |
| `patch` | `ST_RuntimePatch` | in | ja | Eigener Patch. |
| `state` | `ST_RuntimePatchStateV1*` | inout | ja | Initialisierter Output-Zustand. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, patch, state according to the parameter table.
const ST_Result status = api->get_state(owner_id, patch, state);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="release" data-api-status="stable">

### `release`

Einen Patch wiederherstellen und zerstören.

```c
ST_Result release(ST_StringView owner_id, ST_RuntimePatch patch);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.runtime.patches`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Patch-Owner. |
| `patch` | `ST_RuntimePatch` | in | ja | Eigener Patch. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, patch according to the parameter table.
const ST_Result status = api->release(owner_id, patch);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
