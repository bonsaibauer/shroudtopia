<!-- Generated from docs/contracts/world-v1.1.yaml; do not edit by hand. -->
# World API

Entity-Snapshots, Transforms und Welt-Grids mit expliziter Abdeckung.

<div class="api-meta" data-api-status="experimental">

- **Status:** 🧪 experimental
- **Version:** `1.1`
- **Provider:** Nur kontrollierter ShroudEdit-Test-Provider
- **Capability:** `shroudtopia.world.entities.* / shroudtopia.world.voxels.*`
- **Header:** `api/include/shroudtopia/api/world.h`

</div>

## Typen

### `ST_Vec3d`

Dreidimensionaler Double-Vektor.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `x` | `double` | X-Komponente. | — |
| `y` | `double` | Y-Komponente. | — |
| `z` | `double` | Z-Komponente. | — |

### `ST_Quaterniond`

Quaternion-Rotation.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `x` | `double` | X-Komponente. | — |
| `y` | `double` | Y-Komponente. | — |
| `z` | `double` | Z-Komponente. | — |
| `w` | `double` | W-Komponente. | — |

### `ST_TransformV1`

Position, Rotation und Skalierung.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `position` | `ST_Vec3d` | Weltposition. | — |
| `rotation` | `ST_Quaterniond` | Quaternion-Rotation. | — |
| `scale` | `ST_Vec3d` | Skalierung je Achse. | — |

### `ST_AabbV1`

Halboffene Grenzen im Weltraum.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `minimum` | `ST_Vec3d` | Inklusives Minimum. | — |
| `maximum` | `ST_Vec3d` | Exklusives Maximum. | — |

### `ST_EntityHandleV1`

Session- und generationsgebundene Entity-Identität.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `id` | `uint64_t` | Entity-ID. | — |
| `generation` | `uint32_t` | Wiederverwendungs-Generation. | — |
| `session` | `uint32_t` | Weltsession. | — |

### `ST_EntityKindV1`

Unterstützte Entity-Klassifikation.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_ENTITY_UNKNOWN` | `0` | Nicht sicher kopierbar. | — |
| `ST_ENTITY_PROP` | `1` | Unterstützte statische Template-Instanz. | — |
| `ST_ENTITY_OTHER` | `2` | Spieler, NPC oder nicht unterstütztes dynamisches Objekt. | — |

### `ST_EntitySnapshotV1`

Vollständiger Query-Snapshot.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `handle` | `ST_EntityHandleV1` | Entity-Handle. | — |
| `template_id` | `ST_StringView` | Template-Identität. | Bis zum nächsten World-Aufruf auf diesem Thread gültig. |
| `transform` | `ST_TransformV1` | Snapshot-Transform. | — |
| `kind` | `ST_EntityKindV1` | Entity-Klassifikation. | — |

### `ST_CoverageV1`

Bedeutung einer Grid-Zelle.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_COVERAGE_UNKNOWN` | `0` | Nicht geladen oder unbekannt; niemals Luft. | — |
| `ST_COVERAGE_EMPTY` | `1` | Bekannte leere Zelle. | — |
| `ST_COVERAGE_OCCUPIED` | `2` | Bekannte belegte Zelle. | — |

### `ST_GridSpecV1`

Geometrie eines Welt-Grids.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `grid_id` | `ST_StringView` | Grid-ID. | — |
| `origin` | `ST_Vec3d` | Grid-Ursprung. | — |
| `cell_size` | `ST_Vec3d` | Positive Zellabmessungen. | — |
| `chunk_size_x` | `uint32_t` | Chunk-Breite. | — |
| `chunk_size_y` | `uint32_t` | Chunk-Höhe. | — |
| `chunk_size_z` | `uint32_t` | Chunk-Tiefe. | — |

### `ST_WorldApiV1`

Versionierte Service-Funktionstabelle.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `api_version` | `uint32_t` | api version. | — |
| `query_entities` | `function pointer` | query entities. | — |
| `get_entity_transform` | `function pointer` | get entity transform. | — |
| `spawn_entity` | `function pointer` | spawn entity. | — |
| `destroy_entity` | `function pointer` | destroy entity. | — |
| `set_entity_transform` | `function pointer` | set entity transform. | — |
| `get_grid_spec` | `function pointer` | get grid spec. | — |
| `read_grid_region` | `function pointer` | read grid region. | — |
| `write_grid_region` | `function pointer` | write grid region. | — |

### `ST_GridRegionV1`

Begrenzte ausgerichtete Grid-Region.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße. | — |
| `bounds` | `ST_AabbV1` | Halboffene Weltgrenzen. | — |
| `minimum` | `int32_t[3]` | Minimale Zellindizes. | — |
| `dimensions` | `uint32_t[3]` | Zellanzahl je Achse. | — |
| `cell_count` | `size_t` | Gesamte Zellanzahl. | — |

## Funktionen

<section class="api-function" data-api-name="query_entities" data-api-status="experimental">

### `query_entities`

Vollständige Entity-Snapshots in einer halboffenen Region liefern.

```c
ST_Result query_entities(ST_StringView owner_id, const ST_AabbV1* region, ST_EntitySnapshotV1* entities, size_t capacity, size_t* required_count);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.entities.read`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `region` | `const ST_AabbV1*` | in | ja | Halboffene Weltgrenzen. |
| `entities` | `ST_EntitySnapshotV1*` | out | nein | Initialisiertes Array des Aufrufers. |
| `capacity` | `size_t` | in | ja | Elementkapazität des Arrays. |
| `required_count` | `size_t*` | out | ja | Empfängt benötigte Elemente. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, region, entities, capacity, required_count according to the parameter table.
const ST_Result status = api->query_entities(owner_id, region, entities, capacity, required_count);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_entity_transform" data-api-status="experimental">

### `get_entity_transform`

Entity-Transform lesen.

```c
ST_Result get_entity_transform(ST_StringView owner_id, ST_EntityHandleV1 entity, ST_TransformV1* transform);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.entities.read`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `entity` | `ST_EntityHandleV1` | in | ja | Session-gebundene Entity. |
| `transform` | `ST_TransformV1*` | inout | ja | Initialisierter Output. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, entity, transform according to the parameter table.
const ST_Result status = api->get_entity_transform(owner_id, entity, transform);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="spawn_entity" data-api-status="experimental">

### `spawn_entity`

Ein unterstütztes Template spawnen.

```c
ST_Result spawn_entity(ST_StringView owner_id, ST_StringView template_id, const ST_TransformV1* transform, ST_EntityHandleV1* entity);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.entities.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `template_id` | `ST_StringView` | in | ja | Unterstütztes Template. |
| `transform` | `const ST_TransformV1*` | in | ja | Initialer Transform. |
| `entity` | `ST_EntityHandleV1*` | out | ja | Empfängt Entity-Handle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, template_id, transform, entity according to the parameter table.
const ST_Result status = api->spawn_entity(owner_id, template_id, transform, entity);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="destroy_entity" data-api-status="experimental">

### `destroy_entity`

Eine unterstützte Entity zerstören.

```c
ST_Result destroy_entity(ST_StringView owner_id, ST_EntityHandleV1 entity);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.entities.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `entity` | `ST_EntityHandleV1` | in | ja | Zu zerstörende Entity. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, entity according to the parameter table.
const ST_Result status = api->destroy_entity(owner_id, entity);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_entity_transform" data-api-status="experimental">

### `set_entity_transform`

Entity-Transform ändern.

```c
ST_Result set_entity_transform(ST_StringView owner_id, ST_EntityHandleV1 entity, const ST_TransformV1* transform);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.entities.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `entity` | `ST_EntityHandleV1` | in | ja | Zu ändernde Entity. |
| `transform` | `const ST_TransformV1*` | in | ja | Neuer Transform. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, entity, transform according to the parameter table.
const ST_Result status = api->set_entity_transform(owner_id, entity, transform);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_grid_spec" data-api-status="experimental">

### `get_grid_spec`

Grid-Geometrie lesen.

```c
ST_Result get_grid_spec(ST_StringView owner_id, ST_StringView grid_id, ST_GridSpecV1* grid);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.voxels.read`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `grid_id` | `ST_StringView` | in | ja | Grid-Identität. |
| `grid` | `ST_GridSpecV1*` | inout | ja | Initialisierter Output. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, grid_id, grid according to the parameter table.
const ST_Result status = api->get_grid_spec(owner_id, grid_id, grid);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="read_grid_region" data-api-status="experimental">

### `read_grid_region`

Ausgerichtete Zellwerte und explizite Abdeckung lesen.

```c
ST_Result read_grid_region(ST_StringView owner_id, ST_StringView grid_id, const ST_AabbV1* region, uint32_t* values, ST_CoverageV1* coverage, size_t capacity, size_t* required_count);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.voxels.read`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `grid_id` | `ST_StringView` | in | ja | Grid-Identität. |
| `region` | `const ST_AabbV1*` | in | ja | Am Grid ausgerichtete Grenzen. |
| `values` | `uint32_t*` | out | ja | X-schnellster Buffer des Aufrufers. |
| `coverage` | `ST_CoverageV1*` | out | ja | Paralleler Coverage-Buffer. |
| `capacity` | `size_t` | in | ja | Verfügbare Zellen. |
| `required_count` | `size_t*` | out | ja | Benötigte Zellen. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, grid_id, region, values, coverage, capacity, required_count according to the parameter table.
const ST_Result status = api->read_grid_region(owner_id, grid_id, region, values, coverage, capacity, required_count);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="write_grid_region" data-api-status="experimental">

### `write_grid_region`

Bekannte Zellen in eine ausgerichtete Region schreiben.

```c
ST_Result write_grid_region(ST_StringView owner_id, ST_StringView grid_id, const ST_AabbV1* region, const uint32_t* values, const ST_CoverageV1* coverage, size_t count);
```

- **Status:** 🧪 experimental
- **Seit:** `1.0`
- **Berechtigung:** `shroudtopia.world.voxels.write`
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Keine Seiteneffekte außerhalb der beschriebenen Operation.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `grid_id` | `ST_StringView` | in | ja | Grid-Identität. |
| `region` | `const ST_AabbV1*` | in | ja | Am Grid ausgerichtete Grenzen. |
| `values` | `const uint32_t*` | in | ja | X-schnellste Werte. |
| `coverage` | `const ST_CoverageV1*` | in | ja | UNKNOWN-Zellen bleiben unverändert. |
| `count` | `size_t` | in | ja | Exakte Zellanzahl. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, grid_id, region, values, coverage, count according to the parameter table.
const ST_Result status = api->write_grid_region(owner_id, grid_id, region, values, coverage, count);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="st_gridregionfrompoints" data-api-status="stable">

### `ST_GridRegionFromPoints`

Zwei inkludierte Punkte in begrenzte ausgerichtete halboffene Grid-Grenzen umwandeln.

```c
ST_Result ST_GridRegionFromPoints(const ST_GridSpecV1* grid, ST_Vec3d a, ST_Vec3d b, size_t maximum_cells, ST_GridRegionV1* output);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** Keine
- **Threading:** Rein, reentrant und auf jedem Thread aufrufbar.
- **Ownership:** Keine Allokation; der Aufrufer besitzt Input und Output.
- **Seiteneffekte:** Keine.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `grid` | `const ST_GridSpecV1*` | in | ja | Initialisierte Grid-Geometrie. |
| `a` | `ST_Vec3d` | in | ja | Erster inkludierter Punkt. |
| `b` | `ST_Vec3d` | in | ja | Zweiter inkludierter Punkt. |
| `maximum_cells` | `size_t` | in | ja | Von null verschiedenes Allokationsbudget. |
| `output` | `ST_GridRegionV1*` | inout | ja | Initialisiertes Ergebnis. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Eine begrenzte Region wurde geschrieben. |
| `ST_RESULT_INVALID_ARGUMENT` | Input ist ungültig, nicht endlich, überlaufend oder über Budget. |

#### Beispiel

```cpp
ST_GridRegionV1 region{sizeof(region)};
ST_Result status = ST_GridRegionFromPoints(&grid, a, b, 1u << 20, &region);
```

</section>
