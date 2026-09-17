<!-- Generated from docs/contracts/text-ui-v1.0.yaml; do not edit by hand. -->
# Native Text UI API

Native Textfenster mit Tabs für Windowed- und Borderless-Clients.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia Loader auf Windows-Clients
- **Capability:** Keine
- **Header:** `api/include/shroudtopia/api/ui.h`

</div>

## Typen

### `ST_TextWindow`

Opakes Owner-gebundenes natives Fenster.

### `ST_TextWindowStatusV1`

Asynchroner Erstellungszustand.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_TEXT_PENDING` | `0` | Erstellung ausstehend. | — |
| `ST_TEXT_READY` | `1` | Fenster bereit. | — |
| `ST_TEXT_FAILED` | `2` | Erstellung fehlgeschlagen. | — |

### `ST_TextWindowDescriptorV1`

Definition eines nativen Fensters mit Tabs.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `title` | `ST_StringView` | Fenstertitel, höchstens 128 Byte. | — |
| `tabs` | `const ST_StringView*` | Ein bis acht Tab-Beschriftungen. | — |
| `tab_count` | `size_t` | Anzahl Tabs, 1..8. | — |
| `toggle_key` | `uint32_t` | Windows Virtual Key; null deaktiviert. | — |

### `ST_UiTextApiV1`

Versionierte Service-Funktionstabelle.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | struct size. | — |
| `abi_version` | `uint32_t` | abi version. | — |
| `create` | `function pointer` | create. | — |
| `set_text` | `function pointer` | set text. | — |
| `get_status` | `function pointer` | get status. | — |
| `destroy` | `function pointer` | destroy. | — |

## Funktionen

<section class="api-function" data-api-name="create" data-api-status="stable">

### `create`

Asynchrone Erstellung eines nativen Fensters beginnen.

```c
ST_Result create(ST_StringView owner, const ST_TextWindowDescriptorV1* descriptor, ST_TextWindow* window);
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
| `owner` | `ST_StringView` | in | ja | Fenster-Owner. |
| `descriptor` | `const ST_TextWindowDescriptorV1*` | in | ja | Fenster und Tabs. |
| `window` | `ST_TextWindow*` | out | ja | Empfängt das Fenster-Handle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner, descriptor, window according to the parameter table.
const ST_Result status = api->create(owner, descriptor, window);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="set_text" data-api-status="stable">

### `set_text`

Snapshot eines Tabs ersetzen.

```c
ST_Result set_text(ST_StringView owner, ST_TextWindow window, size_t tab, ST_StringView text);
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
| `owner` | `ST_StringView` | in | ja | Fenster-Owner. |
| `window` | `ST_TextWindow` | in | ja | Eigenes Fenster. |
| `tab` | `size_t` | in | ja | Nullbasierter Tab-Index. |
| `text` | `ST_StringView` | in | ja | Kopiertes UTF-8, höchstens ein MiB. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner, window, tab, text according to the parameter table.
const ST_Result status = api->set_text(owner, window, tab, text);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_status" data-api-status="stable">

### `get_status`

Asynchronen Erstellungsstatus lesen.

```c
ST_Result get_status(ST_StringView owner, ST_TextWindow window, ST_TextWindowStatusV1* status);
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
| `owner` | `ST_StringView` | in | ja | Fenster-Owner. |
| `window` | `ST_TextWindow` | in | ja | Eigenes Fenster. |
| `status` | `ST_TextWindowStatusV1*` | out | ja | Empfängt den aktuellen Status. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner, window, status according to the parameter table.
const ST_Result status = api->get_status(owner, window, status);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="destroy" data-api-status="stable">

### `destroy`

Fenster zerstören und seinen UI-Thread joinen.

```c
ST_Result destroy(ST_StringView owner, ST_TextWindow window);
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
| `owner` | `ST_StringView` | in | ja | Fenster-Owner. |
| `window` | `ST_TextWindow` | in | ja | Eigenes Fenster. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner, window according to the parameter table.
const ST_Result status = api->destroy(owner, window);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
