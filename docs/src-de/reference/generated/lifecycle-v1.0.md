<!-- Generated from docs/contracts/lifecycle-v1.0.yaml; do not edit by hand. -->
# Lifecycle API

Discovery, Erstellung, Aktivierung, Update und deterministischer Abbau nativer Mods.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia Loader
- **Capability:** `shroudtopia.lifecycle.native`
- **Header:** `api/include/shroudtopia/api/lifecycle.h`

</div>

## Typen

### `ST_ModDescriptorV1`

Identität, Zustand und Lifecycle-Callbacks einer nativen Mod.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße. | — |
| `mod_id` | `ST_StringView` | Global eindeutige stabile Mod-ID. | — |
| `user_data` | `void*` | Callback-Kontext im Besitz der Mod. | — |
| `on_load` | `ST_ModLifecycleCallback` | Optionaler Registrierungs-Callback. | — |
| `on_activate` | `ST_ModLifecycleCallback` | Optionaler Aktivierungs-Callback. | — |
| `on_update` | `ST_ModUpdateCallback` | Optionales begrenztes Worker-Thread-Update. | — |
| `on_deactivate` | `ST_ModLifecycleCallback` | Optionaler Deaktivierungs-Callback. | — |
| `on_unload` | `ST_ModLifecycleCallback` | Optionaler finaler Cleanup-Callback. | — |

## Funktionen

<section class="api-function" data-api-name="shroudtopiagetapi" data-api-status="stable">

### `ShroudtopiaGetApi`

Die Host-Tabelle des Loaders anfordern.

```c
ST_Result ShroudtopiaGetApi(uint32_t requested_abi, const ST_HostApiV1** api);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** Keine
- **Threading:** Beim nativen Bootstrap auf dem Lade-Thread aufrufen.
- **Ownership:** Der Loader besitzt die zurückgegebene Tabelle für die Lebensdauer der geladenen Mod.
- **Seiteneffekte:** Kein persistenter Seiteneffekt.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `requested_abi` | `uint32_t` | in | ja | Erforderliche ABI-Konstante. |
| `api` | `const ST_HostApiV1**` | out | ja | Empfängt die Loader-eigene Tabelle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die kompatible Tabelle wurde geliefert. |
| `ST_RESULT_INVALID_ARGUMENT` | Der Output-Pointer ist null. |
| `ST_RESULT_VERSION_MISMATCH` | Die ABI wird nicht unterstützt. |

#### Beispiel

```cpp
const ST_HostApiV1* host = nullptr;
ST_Result status = ShroudtopiaGetApi(ST_ABI_VERSION_1, &host);
```

</section>
