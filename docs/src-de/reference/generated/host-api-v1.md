<!-- Generated from docs/contracts/host-api-v1.yaml; do not edit by hand. -->
# Host API

Zentrale Registries, Berechtigungen, Settings und Logging für jede native Mod.

<div class="api-meta" data-api-status="stable">

- **Status:** ✅ stable
- **Version:** `1.0`
- **Provider:** Shroudtopia Loader
- **Capability:** Keine
- **Header:** `api/include/shroudtopia/api/host.h`

</div>

## Typen

### `ST_StringView`

Geliehener Byte-String; kein NUL-Abschluss erforderlich.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `data` | `const char*` | Erstes Byte des Strings. | Darf nur bei size == 0 null sein. |
| `size` | `size_t` | Stringlänge in Byte. | — |

### `ST_Result`

Ergebnis jeder fehlschlagbaren API-Operation.

| Werte | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `ST_RESULT_OK` | `0` | Operation abgeschlossen. | — |
| `ST_RESULT_INVALID_ARGUMENT` | `1` | Ungültige Eingabe. | — |
| `ST_RESULT_ALREADY_EXISTS` | `2` | Eindeutiger Wert oder Ownership-Bereich kollidiert. | — |
| `ST_RESULT_NOT_FOUND` | `3` | Angefordertes Element fehlt. | — |
| `ST_RESULT_VERSION_MISMATCH` | `4` | Versionen sind inkompatibel. | — |
| `ST_RESULT_PERMISSION_DENIED` | `5` | Erforderliche Berechtigung fehlt. | — |
| `ST_RESULT_UNSUPPORTED` | `6` | Bekannte Operation ist nicht verfügbar. | — |
| `ST_RESULT_CALLBACK_FAILED` | `7` | Ein Callback schlug fehl oder warf eine Ausnahme. | — |
| `ST_RESULT_INTERNAL_ERROR` | `8` | Hostfehler. | — |

### `ST_Registration`

Opakes, von null verschiedenes, Owner-gebundenes Registry-Handle.

### `ST_ServiceDescriptor`

Identität und Interface eines veröffentlichten Service.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `contract_id` | `ST_StringView` | Global eindeutige Contract-ID. | — |
| `version_major` | `uint32_t` | Version für inkompatible Änderungen. | — |
| `version_minor` | `uint32_t` | Rückwärtskompatible Version. | — |
| `interface_pointer` | `const void*` | Interface-Tabelle im Besitz des Providers. | — |

### `ST_ServiceRequest`

Kompatible Service-Suchanfrage.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `contract_id` | `ST_StringView` | Angeforderte Contract-ID. | — |
| `version_major` | `uint32_t` | Erforderliche Major-Version. | — |
| `minimum_minor` | `uint32_t` | Kleinste kompatible Minor-Version. | — |

### `ST_Event`

Publiziertes synchrones Event.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `event_id` | `ST_StringView` | Globale Event-ID. | — |
| `payload` | `const void*` | Opaqe Payload-Bytes. | Während Dispatch geliehen; Eigentum des Publishers. |
| `payload_size` | `size_t` | Payload-Länge in Byte. | — |

### `ST_CommandDescriptor`

Globale Command-Registrierung.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `command_id` | `ST_StringView` | Globale Command-ID. | — |
| `description` | `ST_StringView` | Menschenlesbarer Hilfetext. | — |
| `callback` | `ST_CommandCallback` | Ausführungs-Callback. | — |
| `user_data` | `void*` | Opaker Callback-Kontext. | — |

### `ST_SettingsDescriptor`

Settings-Schema und Standardwerte.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `settings_id` | `ST_StringView` | Eindeutige Settings-ID. | — |
| `schema_json` | `ST_StringView` | JSON-Schema-Dokument. | — |
| `defaults_json` | `ST_StringView` | JSON-Objekt mit Standardwerten. | — |

### `ST_CapabilityInfoV1`

Laufzeit-Verfügbarkeit eines Features.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße in Byte. | Auf sizeof(der Struktur) setzen. |
| `version_major` | `uint32_t` | Major-Version der Capability. | — |
| `version_minor` | `uint32_t` | Minor-Version der Capability. | — |
| `flags` | `uint64_t` | Capability-spezifische Flags; null, wenn keine. | — |
| `available` | `uint8_t` | Eins bei Verfügbarkeit, sonst null. | — |
| `reserved` | `uint8_t[7]` | Reserviert und mit null initialisiert. | — |

### `ST_EventSubscription`

Event-Subscription mit Callback und Kontext.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Initialisierte Strukturgröße. | — |
| `event_id` | `ST_StringView` | Abonnierte Event-ID. | — |
| `callback` | `ST_EventCallback` | Synchroner Callback. | — |
| `user_data` | `void*` | Opaker Callback-Kontext. | — |

### `ST_HostApiV1`

Versionierte Host-Funktionstabelle für Lifecycle-Callbacks.

| Feld | C-Typ / Wert | Bedeutung | Regeln |
|---|---|---|---|
| `struct_size` | `size_t` | Verfügbare Tabellenbytes. | — |
| `abi_version` | `uint32_t` | Host-ABI-Version. | — |
| `register_service` | `function pointer` | register service-Operation. | — |
| `find_service` | `function pointer` | find service-Operation. | — |
| `subscribe_event` | `function pointer` | subscribe event-Operation. | — |
| `publish_event` | `function pointer` | publish event-Operation. | — |
| `register_command` | `function pointer` | register command-Operation. | — |
| `execute_command` | `function pointer` | execute command-Operation. | — |
| `register_settings` | `function pointer` | register settings-Operation. | — |
| `release_registration` | `function pointer` | release registration-Operation. | — |
| `release_owner` | `function pointer` | release owner-Operation. | — |
| `query_capability` | `function pointer` | query capability-Operation. | — |
| `check_permission` | `function pointer` | check permission-Operation. | — |
| `log` | `function pointer` | log-Operation. | — |
| `get_setting_bool` | `function pointer` | get setting bool-Operation. | — |
| `get_setting_number` | `function pointer` | get setting number-Operation. | — |

## Funktionen

<section class="api-function" data-api-name="register_service" data-api-status="stable">

### `register_service`

Einen versionierten Service-Provider veröffentlichen.

```c
ST_Result register_service(ST_StringView owner_id, const ST_ServiceDescriptor* descriptor, ST_Registration* registration);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** Keine
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Ändert die beschriebene Ressource.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Stabile Mod-Owner-ID. |
| `descriptor` | `const ST_ServiceDescriptor*` | in | ja | Initialisierte Service-Beschreibung. |
| `registration` | `ST_Registration*` | out | ja | Empfängt das Release-Handle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_ALREADY_EXISTS` | Contract und Major-Version haben bereits einen Provider. |

#### Beispiel

```cpp
// Initialize owner_id, descriptor, registration according to the parameter table.
const ST_Result status = api->register_service(owner_id, descriptor, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="find_service" data-api-status="stable">

### `find_service`

Den höchsten kompatiblen Service-Provider finden.

```c
ST_Result find_service(const ST_ServiceRequest* request, const void** interface_pointer);
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
| `request` | `const ST_ServiceRequest*` | in | ja | Angeforderter Contract und Version. |
| `interface_pointer` | `const void**` | out | ja | Empfängt die Provider-eigene Tabelle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_NOT_FOUND` | Kein kompatibler Provider ist registriert. |
| `ST_RESULT_VERSION_MISMATCH` | Der Contract existiert in inkompatibler Version. |

#### Beispiel

```cpp
// Initialize request, interface_pointer according to the parameter table.
const ST_Result status = api->find_service(request, interface_pointer);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="subscribe_event" data-api-status="stable">

### `subscribe_event`

Einen Callback für eine Event-ID abonnieren.

```c
ST_Result subscribe_event(ST_StringView owner_id, const ST_EventSubscription* subscription, ST_Registration* registration);
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
| `owner_id` | `ST_StringView` | in | ja | Owner-ID des Subscribers. |
| `subscription` | `const ST_EventSubscription*` | in | ja | Event-ID, Callback und Kontext. |
| `registration` | `ST_Registration*` | out | ja | Empfängt das Subscription-Handle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, subscription, registration according to the parameter table.
const ST_Result status = api->subscribe_event(owner_id, subscription, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="publish_event" data-api-status="stable">

### `publish_event`

Ein Event synchron an einen Subscriber-Snapshot ausliefern.

```c
ST_Result publish_event(ST_StringView owner_id, const ST_Event* event_data);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** Keine
- **Threading:** Ruft Callbacks synchron auf dem publizierenden Thread auf.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Ändert die beschriebene Ressource.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Owner-ID des Publishers. |
| `event_data` | `const ST_Event*` | in | ja | Event und geliehener Payload. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_CALLBACK_FAILED` | Ein Subscriber schlug fehl oder warf eine Ausnahme. |

#### Beispiel

```cpp
// Initialize owner_id, event_data according to the parameter table.
const ST_Result status = api->publish_event(owner_id, event_data);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="register_command" data-api-status="stable">

### `register_command`

Ein global eindeutiges Command registrieren.

```c
ST_Result register_command(ST_StringView owner_id, const ST_CommandDescriptor* descriptor, ST_Registration* registration);
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
| `owner_id` | `ST_StringView` | in | ja | Command-Owner. |
| `descriptor` | `const ST_CommandDescriptor*` | in | ja | Command-Callback und Metadaten. |
| `registration` | `ST_Registration*` | out | ja | Empfängt die Registrierung. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_ALREADY_EXISTS` | Die Command-ID ist bereits registriert. |

#### Beispiel

```cpp
// Initialize owner_id, descriptor, registration according to the parameter table.
const ST_Result status = api->register_command(owner_id, descriptor, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="execute_command" data-api-status="stable">

### `execute_command`

Ein registriertes Command mit Rohargumenten ausführen.

```c
ST_Result execute_command(ST_StringView owner_id, ST_StringView command_id, ST_StringView arguments);
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
| `owner_id` | `ST_StringView` | in | ja | Aufrufender Owner. |
| `command_id` | `ST_StringView` | in | ja | Auszuführendes Command. |
| `arguments` | `ST_StringView` | in | ja | Geliehener Argument-Rohtext. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_NOT_FOUND` | Das Command ist nicht registriert. |
| `ST_RESULT_CALLBACK_FAILED` | Der Command-Callback schlug fehl. |

#### Beispiel

```cpp
// Initialize owner_id, command_id, arguments according to the parameter table.
const ST_Result status = api->execute_command(owner_id, command_id, arguments);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="register_settings" data-api-status="stable">

### `register_settings`

Settings-Schema und Standardwerte registrieren.

```c
ST_Result register_settings(ST_StringView owner_id, const ST_SettingsDescriptor* descriptor, ST_Registration* registration);
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
| `owner_id` | `ST_StringView` | in | ja | Settings-Owner. |
| `descriptor` | `const ST_SettingsDescriptor*` | in | ja | Schema- und Defaults-JSON. |
| `registration` | `ST_Registration*` | out | ja | Empfängt die Registrierung. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, descriptor, registration according to the parameter table.
const ST_Result status = api->register_settings(owner_id, descriptor, registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="release_registration" data-api-status="stable">

### `release_registration`

Eine Registry-Ressource freigeben.

```c
ST_Result release_registration(ST_Registration registration);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** Keine
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Ändert die beschriebene Ressource.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `registration` | `ST_Registration` | in | ja | Gültiges Owner-gebundenes Handle. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_NOT_FOUND` | Das Handle ist unbekannt oder bereits freigegeben. |

#### Beispiel

```cpp
// Initialize registration according to the parameter table.
const ST_Result status = api->release_registration(registration);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="release_owner" data-api-status="stable">

### `release_owner`

Alle einem Owner zugeordneten Ressourcen freigeben.

```c
ST_Result release_owner(ST_StringView owner_id);
```

- **Status:** ✅ stable
- **Seit:** `1.0`
- **Berechtigung:** Keine
- **Threading:** Synchron auf dem aufrufenden Thread; nicht gleichzeitig aufrufen, sofern nicht anders angegeben.
- **Ownership:** Eingaben sind für die Aufrufdauer geliehen; Output-Ownership steht am jeweiligen Parameter.
- **Seiteneffekte:** Ändert die beschriebene Ressource.

#### Parameter

| Feld | C-Typ / Wert | Richtung | Pflicht | Bedeutung |
|---|---|:---:|:---:|---|
| `owner_id` | `ST_StringView` | in | ja | Zu bereinigender Owner. |

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
const ST_Result status = api->release_owner(owner_id);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="query_capability" data-api-status="stable">

### `query_capability`

Verfügbarkeit einer bekannten Capability abfragen.

```c
ST_Result query_capability(ST_StringView capability_id, ST_CapabilityInfoV1* information);
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
| `capability_id` | `ST_StringView` | in | ja | Capability-ID. |
| `information` | `ST_CapabilityInfoV1*` | inout | ja | Initialisierte Output-Information. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |
| `ST_RESULT_NOT_FOUND` | Die Capability-ID ist unbekannt. |

#### Beispiel

```cpp
// Initialize capability_id, information according to the parameter table.
const ST_Result status = api->query_capability(capability_id, information);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="check_permission" data-api-status="stable">

### `check_permission`

Eine Manifest-gebundene Owner-Berechtigung prüfen.

```c
ST_Result check_permission(ST_StringView owner_id, ST_StringView permission_id, uint8_t* allowed);
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
| `owner_id` | `ST_StringView` | in | ja | Zu prüfender Owner. |
| `permission_id` | `ST_StringView` | in | ja | Permission-/Capability-ID. |
| `allowed` | `uint8_t*` | out | ja | Empfängt eins oder null. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, permission_id, allowed according to the parameter table.
const ST_Result status = api->check_permission(owner_id, permission_id, allowed);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="log" data-api-status="stable">

### `log`

Eine Owner-markierte Logmeldung schreiben.

```c
ST_Result log(ST_StringView owner_id, ST_LogLevel level, ST_StringView message);
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
| `owner_id` | `ST_StringView` | in | ja | Owner der Meldung. |
| `level` | `ST_LogLevel` | in | ja | Log-Schweregrad. |
| `message` | `ST_StringView` | in | ja | UTF-8-Meldung; wird im Aufruf kopiert. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, level, message according to the parameter table.
const ST_Result status = api->log(owner_id, level, message);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_setting_bool" data-api-status="stable">

### `get_setting_bool`

Ein boolesches Setting oder seinen Fallback lesen.

```c
ST_Result get_setting_bool(ST_StringView owner_id, ST_StringView key, uint8_t fallback, uint8_t* value);
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
| `owner_id` | `ST_StringView` | in | ja | Settings-Owner. |
| `key` | `ST_StringView` | in | ja | Setting-Key. |
| `fallback` | `uint8_t` | in | ja | Fallback null oder eins. |
| `value` | `uint8_t*` | out | ja | Empfängt null oder eins. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, key, fallback, value according to the parameter table.
const ST_Result status = api->get_setting_bool(owner_id, key, fallback, value);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>

<section class="api-function" data-api-name="get_setting_number" data-api-status="stable">

### `get_setting_number`

Ein endliches numerisches Setting oder seinen Fallback lesen.

```c
ST_Result get_setting_number(ST_StringView owner_id, ST_StringView key, double fallback, double* value);
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
| `owner_id` | `ST_StringView` | in | ja | Settings-Owner. |
| `key` | `ST_StringView` | in | ja | Setting-Key. |
| `fallback` | `double` | in | ja | Endlicher Fallback. |
| `value` | `double*` | out | ja | Empfängt eine endliche Zahl. |

#### Ergebnisse

| Ergebnis | Bedeutung |
|---|---|
| `ST_RESULT_OK` | Die Operation wurde abgeschlossen. |
| `ST_RESULT_INVALID_ARGUMENT` | Ein Pointer, eine Größe, ID, Beschreibung oder ein Wert ist ungültig. |
| `ST_RESULT_PERMISSION_DENIED` | Dem Owner fehlt die erforderliche Berechtigung. |
| `ST_RESULT_INTERNAL_ERROR` | Der Host konnte die Operation nicht abschließen. |

#### Beispiel

```cpp
// Initialize owner_id, key, fallback, value according to the parameter table.
const ST_Result status = api->get_setting_number(owner_id, key, fallback, value);
if (status != ST_RESULT_OK) {
    // Handle the specific documented result.
    return status;
}
```

</section>
