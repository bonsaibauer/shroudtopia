# Vorschlag: eine einfache und einheitliche Mod API v2

## Ausgangslage

Die aktuelle API ist technisch ABI-stabil, aber ihre Bedienoberfläche erklärt sich
nicht selbst. Ein Mod-Autor muss heute gleichzeitig Service Discovery,
`struct_size`, Owner-IDs, Buffer-Größen, Callback-Besitz und einzelne
Namenskonventionen verstehen, bevor eine einfache fachliche Abfrage möglich ist.

Beispiele für die derzeitige Reibung:

- `visit_resources` liefert nicht aus dem Namen heraus, ob gesucht, gefiltert oder
  verändert wird;
- `read_resource_json` klingt nach Datei-I/O, obwohl eine typisierte Ressource
  abgefragt wird;
- `query_entities`, `get_entity_transform` und `read_grid_region` verwenden drei
  verschiedene Verben für fachlich ähnliche Leseoperationen;
- `replace_resource_json`, `set_resource_field_json`, `write_grid_region` und
  `set_entity_transform` bilden Änderungen unterschiedlich ab;
- fast jeder Aufruf verlangt technische Parameter, die nichts mit der eigentlichen
  Mod-Funktion zu tun haben.

Das Problem lässt sich nicht allein durch schönere Dokumentation lösen. Die
öffentliche Mod-Oberfläche selbst benötigt ein kleines, konsistentes Fachmodell.

## Zielbild

Eine neue Mod soll mit wenigen, lesbaren Schritten beginnen können:

```cpp
auto api = shroudtopia::connect(mod);

auto assets = api.assets();
auto swords = assets.list({.type = "game.Item", .name = "*Sword*"});

auto item = assets.get(swords.front().id);
item.set("maxStackSize", 20);
assets.save();
```

Für Weltobjekte soll dasselbe Vokabular gelten:

```cpp
auto entities = api.world().entities().list({.inside = area});
auto entity = api.world().entities().get(entities.front().id);

entity.transform.position.z += 2.0;
api.world().entities().update(entity);
```

Die API soll sowohl einen stabilen C-Vertrag als auch eine komfortable C++-Schicht
anbieten. Mod-Autoren verwenden standardmäßig die C++-Schicht. Die C-Schicht bleibt
die binär stabile Grundlage für andere Sprachen und Loader-Kompatibilität.

## 1. Zwei klar getrennte Ebenen

### Ebene A: stabile C ABI

Die C ABI bleibt klein, versioniert und frei von STL- oder Compiler-spezifischen
Typen. Sie enthält Handles, POD-Strukturen und Function Tables. Sie ist nicht länger
die primäre Lernoberfläche.

### Ebene B: Mod SDK für C++

Ein header-only oder statisch gelinktes SDK übernimmt die technische Arbeit:

- Service Discovery und Versionsprüfung;
- `struct_size`-Initialisierung;
- Owner-ID und Permission-Kontext;
- Zwei-Aufruf-Buffer-Muster;
- Umwandlung von `ST_StringView` in `std::string_view` oder `std::string`;
- RAII für Registrierungen, Fenster, Patches und Änderungen;
- typisierte Fehler über `Result<T>`;
- Collections als `std::vector<T>`;
- sichere Callback-Adapter ohne Exceptions über der C ABI.

Damit bleibt die ABI robust, während der normale Mod-Code kurz und verständlich
wird.

## 2. Ein verbindliches Namensschema

Jedes Verb hat genau eine Bedeutung. Synonyme werden nicht vermischt.

| Verb | Bedeutung | Beispiel |
|---|---|---|
| `get` | Ein bekanntes Objekt anhand seiner ID laden; fehlt es, kommt `not_found`. | `assets.get(id)` |
| `find` | Optional genau ein Objekt nach Kriterien suchen. | `services.find("weather")` |
| `list` | Null bis viele Objekte anhand eines Filters liefern. | `entities.list(query)` |
| `create` | Ein neues Objekt erzeugen und das fertige Objekt beziehungsweise seine ID liefern. | `entities.create(draft)` |
| `update` | Ein bestehendes vollständiges Objekt ändern. | `entities.update(entity)` |
| `set` | Genau einen Wert oder ein Feld setzen. | `item.set("maxStackSize", 20)` |
| `remove` | Ein fachliches Objekt entfernen. | `entities.remove(id)` |
| `release` | Eine technische Registrierung oder Ressource freigeben. | `subscription.release()` |
| `save` | Vorgemerkte Änderungen dauerhaft veröffentlichen. | `assets.save()` |
| `reset` | Eigene noch nicht gespeicherte Änderungen verwerfen. | `assets.reset()` |
| `subscribe` | Einen Handler registrieren und eine Subscription liefern. | `events.subscribe<T>(handler)` |
| `publish` | Ein Event synchron oder explizit asynchron veröffentlichen. | `events.publish(event)` |

`query`, `visit`, `read`, `write`, `replace`, `execute` und `destroy` werden in der
komfortablen Mod API nicht als austauschbare Standardverben verwendet. Sie bleiben
nur dort erlaubt, wo sie fachlich präziser sind, beispielsweise `commands.execute`
oder ein echter Byte-Stream `stream.read`.

## 3. Eine vorhersehbare API-Struktur

Die Mod erhält genau einen gebundenen Einstiegspunkt:

```text
api
├── log
│   ├── trace(message)
│   ├── debug(message)
│   ├── info(message)
│   ├── warn(message)
│   └── error(message)
├── settings
│   ├── get_bool(key, fallback)
│   ├── get_number(key, fallback)
│   ├── get_string(key, fallback)
│   └── subscribe(handler)
├── events
│   ├── subscribe<T>(handler)
│   └── publish<T>(event)
├── commands
│   ├── register(command)
│   └── execute(id, arguments)
├── assets
│   ├── get(id)
│   ├── find(query)
│   ├── list(query)
│   ├── create(draft)
│   ├── update(asset)
│   ├── set(id, field, value)
│   ├── save()
│   └── reset()
├── world
│   ├── entities()
│   │   ├── get(id)
│   │   ├── find(query)
│   │   ├── list(query)
│   │   ├── create(draft)
│   │   ├── update(entity)
│   │   └── remove(id)
│   └── grids()
│       ├── get(id)
│       ├── get_region(id, bounds)
│       └── update_region(id, region)
├── ui
│   └── create_text_window(options)
└── patches
    └── create(options)
```

Der aktuelle `owner_id` wird beim Verbindungsaufbau einmal gebunden und danach nicht
bei jedem Aufruf wiederholt. Permissions werden beim Zugriff auf eine Domain geprüft.
Fehlt eine Domain, liefert beispielsweise `api.assets()` einen erklärbaren Fehler
mit Capability, benötigter Version und Lösungshinweis.

## 4. Klare Abfragemodelle statt unklarer Parameterlisten

Jede Collection erhält ein benanntes Query-Objekt. Leere Felder bedeuten „kein
Filter“. Alle Filter werden in der Referenz mit Typ, Operator, Einheit, Default und
Beispiel dokumentiert.

```cpp
AssetQuery query{
    .type = "game.Item",
    .name = "*Sword*",
    .changed_by = std::nullopt,
    .limit = 100
};

EntityQuery query{
    .inside = Aabb::from_center(player.position, {50, 50, 20}),
    .kind = EntityKind::prop,
    .template_id = std::nullopt,
    .limit = 500
};
```

Für jede Query gelten dieselben Regeln:

- `list(query)` liefert immer eine Collection, auch wenn sie leer ist;
- `find(query)` liefert höchstens ein Objekt und meldet Mehrdeutigkeit;
- `get(id)` nimmt ausschließlich eine stabile ID;
- Pagination beziehungsweise `limit` ist explizit und besitzt ein dokumentiertes
  Maximum;
- Sortierung ist stabil und wird benannt;
- ein Filter, den der Provider nicht unterstützt, liefert `unsupported_filter` und
  wird niemals still ignoriert;
- Partial Results sind nur mit expliziter Kennzeichnung erlaubt.

## 5. Verständliche Datenmodelle

### Assets

Ein Mod-Autor arbeitet mit `AssetId`, `AssetSummary`, `Asset`, `AssetQuery` und
`AssetDraft`. Interne Begriffe wie KFC3, Part-Index oder JSON Pointer erscheinen nur
in einem Abschnitt für fortgeschrittene Nutzung.

```cpp
struct AssetSummary {
    AssetId id;
    std::string type;
    std::string name;
};

struct Asset {
    AssetId id;
    std::string type;
    Value fields;
    Revision revision;
};
```

`assets.get(id)` beantwortet klar: „Gib mir eine Ressource.“ `asset.get<T>(field)`
und `asset.set(field, value)` beantworten: „Lies oder ändere dieses Feld.“ JSON bleibt
ein mögliches Transportformat der C ABI, aber nicht das primäre Mod-Programmiermodell.

### Entities

`Entity` enthält ID, Typ, Template und Transform. `EntitySummary` ist die kleine Form
für Listen. Ein Handle mit Generation und Session wird im SDK in `EntityId` gekapselt,
statt in jedem Guide neu erklärt zu werden.

### Grids

Grid-Abfragen verwenden `GridRegion` mit `bounds`, `dimensions`, `values` und
`coverage`. Unbekannte Zellen bleiben ein eigener Zustand. Die Speicherreihenfolge
wird innerhalb des SDK gekapselt und nicht jeder Mod erneut auferlegt.

## 6. Ein einheitliches Ergebnis- und Fehlermodell

Jeder fallible SDK-Aufruf liefert `Result<T>`:

```cpp
auto result = api.assets().get(id);
if (!result) {
    api.log.error(result.error().message);
    return;
}

Asset asset = std::move(result.value());
```

Ein Fehler besitzt mindestens:

```cpp
struct Error {
    ErrorCode code;
    std::string message;
    std::string operation;
    std::optional<std::string> capability;
    std::optional<std::string> field;
};
```

Die Codes werden fachlich erweitert und nicht auf ein unspezifisches
`invalid_argument` reduziert:

| Code | Bedeutung |
|---|---|
| `not_found` | ID oder gesuchtes Objekt existiert nicht. |
| `not_available` | Domain besitzt in diesem Prozess keinen Provider. |
| `permission_denied` | Manifest oder Benutzer erlaubt die Operation nicht. |
| `unsupported_filter` | Provider versteht ein Query-Feld nicht. |
| `ambiguous` | `find` hat mehr als einen Treffer. |
| `conflict` | Revision oder Feld-Ownership kollidiert. |
| `invalid_value` | Ein Feldwert verletzt Typ, Bereich oder Format. |
| `limit_exceeded` | Ergebnis, Region oder Payload überschreitet ein Limit. |
| `callback_failed` | Mod-Callback ist fehlgeschlagen. |
| `internal_error` | Der Host konnte die Operation nicht abschließen. |

Die C ABI kann diese Codes als stabile Enumeration und optionale strukturierte
Fehlerdetails transportieren. Das SDK erzeugt daraus verständliche Meldungen.

## 7. Schemas werden zu einer echten Entdeckungsfunktion

Die bisherigen Dokumentations-Contracts reichen nicht aus. Zusätzlich benötigt jede
Domain ein maschinenlesbares fachliches Schema, das eine Mod auch zur Laufzeit
abfragen kann:

```cpp
auto schema = api.assets().schema("game.Item");

for (const Field& field : schema.fields) {
    api.log.info("{}: {}", field.name, field.type);
}
```

Ein Feldschema enthält mindestens:

| Eigenschaft | Zweck |
|---|---|
| `name` | Stabiler programmatischer Feldname |
| `display_name` | Menschenlesbare Bezeichnung |
| `description` | Was das Feld im Spiel bewirkt |
| `type` | Boolean, Integer, Number, String, Enum, Object, Array oder Reference |
| `readable` / `writable` | Erlaubte Operationen |
| `required` / `nullable` | Vorhandensein und Null-Semantik |
| `default` | Standardwert, falls definiert |
| `minimum` / `maximum` | Numerische Grenzen inklusive Einheit |
| `values` | Erlaubte Enum-Werte mit Beschreibung |
| `reference_type` | Zieltyp einer Referenz |
| `since` | Erste API-/Schema-Version |
| `deprecated` | Ablösung und Migration |

Damit kann die Dokumentation konkrete Attribute zeigen, das SDK Werte früh prüfen
und ShroudEdit Formulare generieren. Nicht bekannte oder nicht beschriebene Felder
werden nicht als stabile Mod API versprochen.

## 8. Konkretes Vorher/Nachher-Mapping

| API v1 | Mod API v2 | Begründung |
|---|---|---|
| `visit_resources(type, callback)` | `assets.list(AssetQuery{.type = type})` | Collection und Filter sind sofort erkennbar. |
| `read_resource_json(key, buffer, ...)` | `assets.get(id)` | Technisches Buffer-/JSON-Detail verschwindet. |
| `replace_resource_json(key, json)` | `assets.update(asset)` | Vollständiges Objekt wird explizit aktualisiert. |
| `set_resource_field_json(key, path, json)` | `asset.set(field, value)` | Typisierter Feldzugriff statt Transportdetails. |
| `flush(owner)` | `assets.save()` | Fachliche Absicht statt Speicheroperation. |
| `discard_changes(owner)` | `assets.reset()` | Eigene vorgemerkte Änderungen verwerfen. |
| `query_entities(region, buffer, ...)` | `entities.list(EntityQuery{.inside = region})` | Gleiches Collection-Muster wie Assets. |
| `get_entity_transform(handle)` | `entities.get(id).transform` | Ein Objekt wird vollständig gelesen. |
| `spawn_entity(template, transform)` | `entities.create(EntityDraft{...})` | Einheitliches Create-Modell. |
| `set_entity_transform(handle, transform)` | `entities.update(entity)` | Einheitliches Update-Modell. |
| `destroy_entity(handle)` | `entities.remove(id)` | Einheitliches fachliches Entfernen. |
| `read_grid_region(...)` | `grids.get_region(id, bounds)` | Region und Ergebnis sind typisiert. |
| `write_grid_region(...)` | `grids.update_region(id, region)` | Symmetrisches Lesen/Ändern. |
| `get_setting_bool/number` | `settings.get<T>(key, fallback)` | Ein Einstieg mit typisierten Overloads. |
| `query_capability` + `check_permission` | `api.domain().availability()` | Eine verständliche kombinierte Antwort. |

Die Namen der C ABI müssen nicht zwangsläufig alle sofort geändert werden. Das SDK
kann dieses konsistente Modell zuerst auf v1 abbilden. Ein späteres ABI v2 übernimmt
nur Änderungen, die sich nicht sicher adaptieren lassen.

## 9. Dokumentation nach Aufgaben statt Function Tables

Die erste Seite jeder Domain beantwortet in dieser Reihenfolge:

1. **Was kann ich damit machen?** Konkrete Use Cases und Grenzen.
2. **Welche Daten kann ich abfragen?** Objekt- und Feldübersicht.
3. **Wie suche ich?** Alle Query-Felder mit Beispielen.
4. **Wie lese ich ein Objekt?** `get`-Beispiel.
5. **Wie ändere ich ein Objekt?** `create`, `update`, `set`, `remove`, `save`.
6. **Was brauche ich?** Capability, Permission, Provider und Version.
7. **Was kann fehlschlagen?** Fehlercodes mit Lösung.

Erst danach folgt die vollständige C-ABI-Referenz. Auf der Einstiegsseite dürfen
keine Buffer-Kapazitäten, Function-Pointer oder `struct_size`-Prüfungen nötig sein.

## 10. Umsetzung in kontrollierten Phasen

### Phase 1 – tatsächliche Use Cases festlegen

- für Assets, Entities, Grids, Settings, Events, Commands, UI und Patches jeweils die
  fünf wichtigsten Mod-Aufgaben sammeln;
- für jede Aufgabe Input, Output, Filter, Limits, Permissions und Fehler festlegen;
- Funktionen ohne belegten Use Case nicht in die einfache API übernehmen.

**Ergebnis:** eine freigegebene Capability-/Use-Case-Matrix.

### Phase 2 – Naming und Domain-Modell einfrieren

- die Verben aus diesem Dokument als verbindliche Konvention beschließen;
- `Asset`, `Entity`, `GridRegion`, Query-Objekte, IDs, Drafts und Summaries definieren;
- vor der Implementierung zehn typische Codebeispiele schreiben und im Review nur
  die Verständlichkeit der Aufruferseite bewerten.

**Ergebnis:** eine SDK-Spezifikation mit Beispielcode, noch ohne Loader-Umbau.

### Phase 3 – C++ SDK über API v1 bauen

- `Api`, Domain-Clients, `Result<T>`, RAII-Handles und Buffer-Adapter implementieren;
- Owner, Capability und Service Discovery im `Api`-Kontext kapseln;
- SDK-Tests gegen die vorhandenen In-Memory-/Test-Provider schreiben;
- bestehende Mods schrittweise auf das SDK umstellen.

**Ergebnis:** einfacher Mod-Code ohne sofortigen ABI-Bruch.

### Phase 4 – Schema Discovery implementieren

- Field-, Enum-, Query- und Limit-Schemas definieren;
- Asset- und World-Provider lassen unterstützte Typen und Felder abfragen;
- Dokumentationsgenerator verwendet dieselben Schemas;
- ShroudEdit nutzt sie für Inspektoren und Validierung.

**Ergebnis:** API und Dokumentation zeigen nachweisbar dieselben Attribute.

### Phase 5 – notwendige ABI v2 definieren

- messen, welche SDK-Operationen über v1 ineffizient oder unsicher bleiben;
- nur dafür neue C-Strukturen und Function Tables hinzufügen;
- lange Parametersignaturen durch Request-/Result-Strukturen ersetzen;
- strukturierten Fehlerkontext und standardisierte Listen/Pagination ergänzen.

**Ergebnis:** eine kleine ABI v2, die aus realen SDK-Anforderungen entsteht.

### Phase 6 – Migration und Stabilisierung

- v1 bleibt für einen dokumentierten Zeitraum unterstützt;
- ein Migrationsleitfaden zeigt jedes v1/v2-Mapping;
- Deprecations werden im Compiler und in der Dokumentation sichtbar;
- alle Bundled Mods dienen als ausführbare Beispiele für die neue API;
- erst nach erfolgreicher Migration wird v1 eingefroren oder später entfernt.

## 11. Abnahmekriterien

Die neue Mod API ist erst fertig, wenn alle folgenden Aussagen stimmen:

- ein neues Projekt kann loggen, Settings lesen und einen Service nutzen, ohne
  manuell `ST_StringView`, `struct_size` oder Buffer-Kapazitäten zu verwalten;
- `get`, `find`, `list`, `create`, `update`, `set`, `remove`, `save` und `reset`
  besitzen überall dieselbe Bedeutung;
- jede Liste besitzt ein dokumentiertes Query-Objekt, Limit und stabile Sortierung;
- jede öffentlich lesbare oder schreibbare Eigenschaft steht in einem Schema;
- jedes Beispiel zeigt reale Typen und kompiliert als Test;
- jeder Fehler besitzt Code, Operation und eine für Mod-Autoren brauchbare Meldung;
- Permissions und fehlende Provider ergeben einen erklärbaren Fehler statt nur
  `NOT_FOUND` oder `UNSUPPORTED` ohne Kontext;
- die Bundled Mods verwenden dieselbe SDK-Oberfläche wie externe Mods;
- die C ABI bleibt durch ABI-Tests stabil;
- Deutsch und Englisch werden aus denselben API-/Field-Schemas erzeugt.

## Empfehlung

Nicht sofort alle C-Funktionen umbenennen und damit bestehende Mods brechen. Zuerst
wird die neue, konsistente C++ Mod API als SDK über v1 gebaut. Dabei werden echte
Use Cases, Queries und Datenmodelle stabilisiert. Erst danach wird eine ABI v2 für
die Punkte entworfen, die sich durch den Adapter nicht sauber lösen lassen.

Der erste Implementierungs-Slice sollte **Assets lesen** vollständig abdecken:

```text
api.assets().list(query)
api.assets().get(id)
asset.get<T>(field)
api.assets().schema(type)
```

Dieser Slice löst die aktuell größte Unklarheit – „Was kann ich überhaupt abfragen?“ –
und liefert ein wiederverwendbares Muster für Entities, Grids und Settings.
