<!-- Generated from docs/contracts/assets-v1.1.yaml; do not edit by hand. -->
# Asset API

Typisierte KFC3-Ressourcensuche, JSON-Bearbeitung, Ownership und dauerhafte Veröffentlichung.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.1`
- **Provider:** Shroudtopia Asset Engine
- **Capability:** `shroudtopia.assets.read / shroudtopia.assets.write`
- **Header:** `api/include/shroudtopia/api/assets.h`

</div>

## Typen

### `ST_AssetResourceKeyV1`

Identität einer typisierten Ressource.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `guid` | `ST_StringView` | Ressourcen-GUID. | — |
| `type_name` | `ST_StringView` | Qualifizierter KFC3-Typ. | — |
| `part` | `uint32_t` | Index des Ressourcenteils. | — |

### `ST_AssetsApiV1`

Versionierte Service-Funktionstabelle.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `visit_resources` | `function pointer` | visit resources. | — |
| `read_resource_json` | `function pointer` | read resource json. | — |
| `replace_resource_json` | `function pointer` | replace resource json. | — |
| `create_resource_json` | `function pointer` | create resource json. | — |
| `discard_changes` | `function pointer` | discard changes. | — |
| `flush` | `function pointer` | flush. | — |
| `set_resource_field_json` | `function pointer` | set resource field json. | — |

## Funktionen

<section class="api-function" data-api-name="visit_resources" data-api-status="stable">

### `visit_resources`

Snapshot der Ressourcen eines qualifizierten Typs besuchen.

```c
ST_Result visit_resources(ST_StringView owner_id, ST_StringView type_name, ST_AssetResourceVisitorV1 visitor, void* user_data);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.assets.read`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `type_name` | `ST_StringView` | in | ja | Qualifizierter Ressourcentyp. |
| `visitor` | `ST_AssetResourceVisitorV1` | in | ja | Visitor-Callback. |
| `user_data` | `void*` | in | ja | Opaker Visitor-Kontext. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, type_name, visitor, user_data according to the parameter table.
const ST_Result status = api->visit_resources(owner_id, type_name, visitor, user_data);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="read_resource_json" data-api-status="stable">

### `read_resource_json`

Eine typisierte Ressource als JSON lesen.

```c
ST_Result read_resource_json(ST_StringView owner_id, const ST_AssetResourceKeyV1* resource, char* buffer, size_t capacity, size_t* required_size);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.assets.read`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `resource` | `const ST_AssetResourceKeyV1*` | in | ja | Ressourcenschlüssel. |
| `buffer` | `char*` | out | nein | Output-Buffer im Besitz des Aufrufers. |
| `capacity` | `size_t` | in | ja | Verfügbare Buffer-Bytes. |
| `required_size` | `size_t*` | out | ja | Empfängt exakte Bytezahl ohne Terminator. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, resource, buffer, capacity, required_size according to the parameter table.
const ST_Result status = api->read_resource_json(owner_id, resource, buffer, capacity, required_size);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="replace_resource_json" data-api-status="stable">

### `replace_resource_json`

Ein vollständiges Ressourcendokument ersetzen.

```c
ST_Result replace_resource_json(ST_StringView owner_id, const ST_AssetResourceKeyV1* resource, ST_StringView json);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.assets.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `resource` | `const ST_AssetResourceKeyV1*` | in | ja | Existierender Ressourcenschlüssel. |
| `json` | `ST_StringView` | in | ja | Typisiertes JSON-Dokument. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, resource, json according to the parameter table.
const ST_Result status = api->replace_resource_json(owner_id, resource, json);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="create_resource_json" data-api-status="stable">

### `create_resource_json`

Eine typisierte Ressource erstellen und ihren Schlüssel melden.

```c
ST_Result create_resource_json(ST_StringView owner_id, ST_StringView type_name, ST_StringView json, ST_AssetResourceVisitorV1 visitor, void* user_data);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.assets.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `type_name` | `ST_StringView` | in | ja | Qualifizierter Ressourcentyp. |
| `json` | `ST_StringView` | in | ja | Typisiertes JSON-Dokument. |
| `visitor` | `ST_AssetResourceVisitorV1` | in | ja | Empfängt den erzeugten Schlüssel. |
| `user_data` | `void*` | in | ja | Opaker Callback-Kontext. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, type_name, json, visitor, user_data according to the parameter table.
const ST_Result status = api->create_resource_json(owner_id, type_name, json, visitor, user_data);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="discard_changes" data-api-status="stable">

### `discard_changes`

Alle Asset-Änderungen einer Mod verwerfen.

```c
ST_Result discard_changes(ST_StringView owner_id);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.assets.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id according to the parameter table.
const ST_Result status = api->discard_changes(owner_id);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="flush" data-api-status="stable">

### `flush`

Das kombinierte Asset-Overlay mit Recovery publizieren.

```c
ST_Result flush(ST_StringView owner_id);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.assets.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id according to the parameter table.
const ST_Result status = api->flush(owner_id);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_resource_field_json" data-api-status="stable">

### `set_resource_field_json`

Ein existierendes JSON-Pointer-Feld setzen.

```c
ST_Result set_resource_field_json(ST_StringView owner_id, const ST_AssetResourceKeyV1* resource, ST_StringView path, ST_StringView json);
```

- **Status:** ✅ stable
- **Seit:** `1.1`
- **Berechtigung:** `shroudtopia.assets.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `resource` | `const ST_AssetResourceKeyV1*` | in | ja | Existierende Ressource. |
| `path` | `ST_StringView` | in | ja | Existierender JSON Pointer. |
| `json` | `ST_StringView` | in | ja | Ein typisierter JSON-Wert. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_ALREADY_EXISTS` | Ein anderer Owner kontrolliert ein überlappendes Feld. |

#### Beispiel

```cpp
// Initialize owner_id, resource, path, json according to the parameter table.
const ST_Result status = api->set_resource_field_json(owner_id, resource, path, json);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
