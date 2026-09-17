# Architektur: zweisprachige, schema-basierte API-Dokumentation

> **Umgesetzt:** Diese Architektur wird durch die Contract-Dateien, den Generator,
> die parallelen englischen/deutschen Quellen, das TypeScript-Theme und die
> Dokumentations-Checks in diesem Repository umgesetzt. Dieses Dokument hält die
> Designentscheidungen und Abnahmekriterien fest.

## 1. Zielbild

Die Dokumentation wird von einer Sammlung kurzer, teilweise überlappender Seiten zu
einem **zusammenhängenden Entwicklerportal** umgebaut. Ein Mod-Entwickler soll dort
ohne Lesen der Header beantworten können:

1. Welche Funktionen sind heute tatsächlich verfügbar?
2. Welchen Service und welche Berechtigung benötigt meine Mod?
3. Welche Felder besitzt ein Typ und welche Werte sind erlaubt?
4. Wem gehören Pointer, Strings, Buffer und Handles, und wie lange sind sie gültig?
5. Welche Fehler kann jeder Aufruf zurückgeben?
6. Auf welchem Thread darf der Aufruf erfolgen?
7. Wie sieht ein vollständiges, kompilierbares Beispiel aus?

Die englische und deutsche Ausgabe haben dieselbe Struktur und denselben fachlichen
Inhalt. Oben rechts wird ausschließlich ein Sprachschalter mit den Flaggen `🇬🇧` und
`🇩🇪` angezeigt. Die Auswahl öffnet nach Möglichkeit dieselbe Seite in der anderen
Sprache und wird im Browser gespeichert.

## 2. Übernommenes Schema-Prinzip

Das gewünschte Schema-Prinzip aus dem
[Stellantis API Style Guide](https://github.com/Stellantis/api-standards/blob/master/api-style-guide.md)
wird auf die Shroudtopia-C-ABI übertragen: **Der Vertrag wird einmal
maschinenlesbar beschrieben; Navigation, Tabellen und Konsistenzprüfungen werden
daraus abgeleitet.** OpenAPI wird nicht erzwungen, weil Shroudtopia keine HTTP-API
beschreibt. Stattdessen wird ein kleines, repository-eigenes Vertragsschema auf Basis
von JSON Schema Draft 2020-12 verwendet.

### Eine fachliche Quelle

Vorgeschlagene Struktur:

```text
docs/
├── book.de.toml
├── book.en.toml
├── contracts/
│   ├── host-api-v1.yaml
│   ├── assets-v1.1.yaml
│   ├── logging-v1.0.yaml
│   ├── runtime-patches-v1.0.yaml
│   ├── text-ui-v1.0.yaml
│   └── world-v1.1.yaml
├── schema/
│   └── api-contract.schema.json
├── src-de/
│   ├── SUMMARY.md
│   ├── index.md
│   ├── guides/
│   ├── concepts/
│   ├── reference/       # generierte Referenz plus kuratierte Einleitungen
│   └── status.md
├── src-en/
│   └── ...              # exakt dieselben Seitenpfade wie src-de
├── theme/
│   ├── shroudtopia.css
│   ├── shroudtopia.ts
│   └── generated/
│       └── shroudtopia.js
└── tools/
    ├── build-docs.mjs
    ├── generate-reference.mjs
    └── validate-contracts.mjs
```

Die Header in `api/include/shroudtopia/api/` bleiben die normative ABI-Quelle. Die
Contract-Dateien sind die normative Dokumentationsquelle. CI prüft beide gegeneinander,
damit dokumentierte Funktions- und Feldnamen nicht unbemerkt auseinanderlaufen.

### Minimales Contract-Modell

Jeder Service enthält mindestens:

- ID, Major-/Minor-Version, Stabilitätsstatus und benötigte Capability;
- zugehörige Header und C-Strukturen;
- Funktionen, Typen, Enumerationen und Konstanten;
- Verfügbarkeit im Loader beziehungsweise nur im Test-Provider;
- Threading-, Reentrancy-, Ownership- und Lifetime-Regeln.

Jede Funktion enthält mindestens:

| Eigenschaft | Inhalt |
|---|---|
| `name` / `signature` | Exakter C-Name und vollständige Signatur |
| `summary` | Ein Satz: was der Aufruf bewirkt |
| `since` / `status` | Service-Version und `stable`, `experimental`, `planned` oder `unavailable` |
| `permission` | Benötigte Capability oder ausdrücklich `none` |
| `threading` | Erlaubter Thread, Synchronität, Reentrancy |
| `parameters` | Name, C-Typ, Richtung, Pflichtfeld, Nullability, Wertebereich und Semantik |
| `ownership` | Wer Speicher/Handle besitzt, ob kopiert oder geliehen, Gültigkeitsdauer |
| `returns` | Erfolgseffekt und alle möglichen `ST_Result`-Werte mit konkreter Ursache |
| `sideEffects` | Registrierung, Persistenz, Callback, Datei- oder Weltänderung |
| `example` | Vollständiger positiver Ablauf und mindestens ein Fehlerpfad |

Jede Struktur dokumentiert **jedes Feld** mit C-Typ, Bedeutung, Einheit, erlaubten
Werten, Initialisierung, Ownership, Lifetime und der Version, in der das Feld
hinzukam. Enumerationen erhalten Wert, Zahlenwert, Bedeutung und zulässige
Verwendung. Undokumentierte Felder oder Funktionen lassen die CI fehlschlagen.

## 3. Neue Informationsarchitektur

Die Seiten werden nicht mehr nach zufälliger Entstehungsgeschichte verteilt. Die
Navigation folgt der Aufgabe des Lesers:

```text
Start
├── In 10 Minuten zur ersten Mod
├── Konzepte
│   ├── ABI, Versionierung und struct_size
│   ├── Services und Service Discovery
│   ├── Ownership, Buffer und Lifetime
│   ├── Capabilities und Permissions
│   ├── Threads, Callbacks und Fehlerbehandlung
│   └── Stabilitätsstatus
├── Anleitungen
│   ├── Projekt erstellen
│   ├── Events publizieren und abonnieren
│   ├── Einstellungen definieren
│   ├── Assets lesen und ändern
│   ├── Runtime Patch erstellen
│   ├── Textfenster anzeigen
│   └── World API verwenden
├── API-Referenz
│   ├── Gemeinsame Typen und ST_Result
│   ├── Host API
│   └── ein Kapitel pro Service
├── Beispiele
│   ├── vollständige Minimal-Mod
│   ├── Asset-Mod
│   └── Flight-Mod
└── Status und Roadmap
```

Pro Thema gibt es genau eine Hauptseite. Andere Seiten verlinken darauf, statt
dieselben Statuslisten oder Regeln erneut und möglicherweise widersprüchlich zu
erklären. Architektur- und Loader-Interna werden klar von der öffentlichen Mod-API
getrennt.

## 4. Aufbau einer Referenzseite

Jede Service-Seite verwendet dieselbe Leserführung:

1. **Wofür ist der Service da?** Zwei bis vier Sätze ohne Implementierungsdetails.
2. **Verfügbarkeit.** Version, Status, Provider und Capability auf einen Blick.
3. **Schnellbeispiel.** Ein vollständiger, kopierbarer Ablauf.
4. **Datentypen.** Vollständige Feldtabellen, nicht nur Strukturnamen.
5. **Funktionen.** Eine einheitliche Detailkarte pro Funktion.
6. **Fehler und Grenzfälle.** Konkrete Ursachen pro `ST_Result`.
7. **Ownership und Threading.** Explizite Regeln, auch wenn die Antwort „keine“ ist.
8. **Siehe auch.** Nur relevante Konzepte und Guides.

Beispiel für die notwendige Detailtiefe:

```text
read_resource_json(owner_id, resource, buffer, capacity, required_size)

owner_id       input, required, borrowed for call, non-empty mod ID
resource       input, required, borrowed for call, struct_size initialized
buffer         output, nullable only when capacity == 0, caller-owned
capacity       input, bytes available in buffer
required_size  output, required, receives exact byte count, no NUL terminator

OK                 JSON bytes were written.
INVALID_ARGUMENT   Pointer/size/key combination is invalid.
NOT_FOUND          The resource is unknown.
PERMISSION_DENIED  Owner lacks shroudtopia.assets.read.
...
```

Die endgültigen Fehlerlisten werden aus Implementierung und Tests verifiziert; das
Beispiel ist keine Vorwegnahme eines noch ungeprüften Vertrags.

## 5. Zweisprachigkeit

### Empfohlene technische Lösung

- Zwei mdBook-Builds erzeugen `/de/` und `/en/`; `/` leitet auf die gespeicherte
  Sprache oder standardmäßig Englisch weiter.
- Beide `SUMMARY.md` besitzen identische Zielpfade. Eine CI-Prüfung meldet fehlende
  oder zusätzliche Übersetzungen.
- Code, API-Bezeichner und Contract-Dateien werden nicht übersetzt. Nur erklärender
  Text, Überschriften und Tabellenbeschreibungen sind lokalisiert.
- `lang="de"` beziehungsweise `lang="en"`, Seitentitel, Suchindex und Metadaten
  werden pro Build korrekt gesetzt.
- Der Schalter oben rechts enthält sichtbar **nur** `🇬🇧` und `🇩🇪`. Unsichtbare
  `aria-label`- und `title`-Attribute erhalten die Bedienbarkeit für Screenreader.
- Der Wechsel behält Pfad und Fragment bei. Fehlt ausnahmsweise die Zielseite, wird
  auf die Startseite der gewählten Sprache gewechselt.

Maschinelle Rohübersetzung wird nicht veröffentlicht. Bei einer fachlichen Änderung
wird zunächst der Contract aktualisiert, danach werden beide Sprachtexte im selben
Pull Request angepasst.

## 6. TypeScript und visuelles Konzept

TypeScript verbessert die Bedienung, bleibt aber progressive enhancement: Die
Referenz muss auch ohne JavaScript vollständig lesbar und verlinkbar sein.

### TypeScript-Funktionen

1. Sprachpfad, Fragment und gespeicherte Auswahl sicher umschalten.
2. API-Funktionskarten nach Status oder Namen filtern.
3. Permalinks und „Code kopieren“-Feedback ergänzen.
4. Aktive Navigation und Inhaltsverzeichnis bei langen Referenzseiten synchronisieren.
5. Optional ein kompaktes „Quick info“-Panel aus bereits generierten HTML-Daten
   steuern; keine Vertragsdaten erst im Browser nachladen.

### Gestaltung

- ruhiges, technisch klares Layout statt vieler gleichwertiger Tabellen;
- breiterer Referenzbereich und gut lesbare maximale Textbreite;
- Service-Kopf mit Version, Status und Capability;
- konsistente Callouts für Gefahr, experimentell, Berechtigung und Ownership;
- Funktionssignatur als dominanter Einstieg, danach Parameter- und Fehlerbereiche;
- responsives Layout für Desktop und Mobilgeräte sowie vollständige Tastaturbedienung;
- vorhandene mdBook-Hell-/Dunkel-Themes respektieren, keine eigenständige
  inkompatible Design-App bauen.

TypeScript wird im Build zu einer versionierten JavaScript-Datei kompiliert und über
`additional-js` eingebunden. Generierte Dateien werden deterministisch erzeugt; CI
prüft mit einem sauberen Neuaufbau, dass kein Diff entsteht.

## 7. Umsetzung in sechs lieferbaren Phasen

### Phase 1 – Inventur und verbindliches Contract-Schema

**Arbeiten**

- alle öffentlichen Header, Loader-Implementierungen, Tests und bestehenden Seiten
  inventarisieren;
- JSON Schema und ein vollständig ausgefülltes Pilot-Contract für
  `shroudtopia.assets@1.1` erstellen;
- Begriffe für Status, Ownership, Lifetime, Nullability und Threading festlegen;
- offene oder nicht aus dem Code beweisbare Semantik als Issues markieren, nicht
  erraten.

**Abnahme**

- das Pilot-Contract validiert;
- jede Asset-Struktur, jedes Feld und jede Funktion ist erfasst;
- dokumentierte Fehler sind durch Code oder Test belegbar.

### Phase 2 – Generator und Qualitäts-Gates

**Arbeiten**

- TypeScript/Node-Generator für Feld-, Parameter-, Enum- und Fehlertabellen bauen;
- Header-Symbolabgleich für öffentliche Namen und Funktionsfelder ergänzen;
- Linkprüfung, Schema-Validierung, Übersetzungs-Parität und reproduzierbaren Build in
  CI aufnehmen;
- für generierte Abschnitte klare „nicht manuell bearbeiten“-Grenzen verwenden.

**Abnahme**

- ein falscher Feldname, fehlendes Attribut, kaputter Link oder fehlende
  Sprachseite stoppt CI;
- lokaler Ein-Befehl-Build erzeugt dieselbe Website wie GitHub Pages.

### Phase 3 – Referenz vollständig machen

**Arbeiten**

- Host, Lifecycle, Services, Events, Commands, Settings, Logging, UI, Runtime,
  Assets und World in Contracts erfassen;
- pro Funktion Parameter, Rückgaben, Fehler, Threading, Ownership und Beispiel
  ausarbeiten;
- Status nicht mehr an mehreren Stellen manuell duplizieren.

**Abnahme**

- 100 % der öffentlichen Header-Symbole erscheinen in der Referenz oder stehen auf
  einer expliziten Ausschlussliste mit Begründung;
- keine Referenzseite enthält nur eine Funktionsliste ohne Attributdetails.

### Phase 4 – Inhalt ordnen und zweisprachig ausarbeiten

**Arbeiten**

- neue Informationsarchitektur in Deutsch und Englisch anlegen;
- bestehende Inhalte zu einer kanonischen Seite je Thema zusammenführen;
- Quickstart und Guides auf echte End-to-End-Abläufe mit Build-, Fehler- und
  Cleanup-Pfad umstellen;
- interne Entwicklungsdokumentation aus dem Nutzerfluss herausnehmen, aber weiterhin
  erreichbar halten.

**Abnahme**

- identische Seitenstruktur in beiden Sprachen;
- vom Start bis zu einer kompilierenden Minimal-Mod höchstens drei
  Navigationsentscheidungen;
- jede Guide-Seite verlinkt exakt auf die verwendeten Typen und Funktionen.

### Phase 5 – Theme, Flaggen-Schalter und TypeScript

**Arbeiten**

- responsives Theme und API-Komponenten implementieren;
- ausschließlich `🇬🇧` und `🇩🇪` als sichtbare Sprachauswahl oben rechts integrieren;
- Filter, Copy-Feedback und lange Seitennavigation progressiv ergänzen;
- Fokuszustände, Kontrast, Screenreader-Texte und reduzierte Bewegung testen.

**Abnahme**

- Sprachwechsel landet auf derselben Seite und am selben Anker;
- Schalter zeigt keinen sichtbaren Zusatztext;
- Kerninhalt funktioniert ohne JavaScript;
- Desktop- und Mobile-Screenshots beider Themes bestehen den Review.

### Phase 6 – Migration, Veröffentlichung und Pflege

**Arbeiten**

- alte URLs über Weiterleitungsseiten oder stabile Pfade erhalten;
- GitHub-Pages-Workflow auf beide Builds und Root-Redirect umstellen;
- Contributor-Anleitung und Definition of Done ergänzen;
- tote, doppelte Seiten erst nach Linkprüfung entfernen.

**Abnahme**

- keine bekannten internen 404-Links;
- alte wichtige API-Links führen zum neuen Inhalt;
- PR-Template verlangt Contract- und Übersetzungsänderungen bei API-Änderungen;
- veröffentlichte Website entspricht dem lokal geprüften Artefakt.

## 8. Vorgeschlagene Prüfbefehle

Der endgültige Paketmanager wird erst nach Prüfung der vorhandenen Toolchain
festgelegt. Ziel ist eine kleine, reproduzierbare Oberfläche wie:

```bash
npm run docs:validate    # Schema, Symbole, Links und Sprachparität
npm run docs:generate    # deterministische Referenz erzeugen
npm run docs:build       # TypeScript sowie beide mdBook-Ausgaben bauen
npm run docs:check       # validate + generate-diff + build
```

Zusätzlich bleiben die bestehenden C/C++-Tests maßgeblich für das tatsächliche
API-Verhalten. Dokumentationsbeispiele sollten, soweit möglich, als kompilierbare
Snippets oder Smoke Tests ausgeführt werden.

## 9. Reihenfolge und Pull-Request-Zuschnitt

Die Arbeit sollte nicht als ein schwer prüfbarer Komplettumbau erfolgen:

1. **PR 1:** Contract-Schema, Asset-Pilot und Validierung.
2. **PR 2:** Generator, Symbolabgleich und CI-Gates.
3. **PR 3:** vollständige englische Referenz aus allen Contracts.
4. **PR 4:** deutsche Referenz und neue, parallele Navigation.
5. **PR 5:** Theme, TypeScript und reiner Flaggen-Sprachschalter.
6. **PR 6:** Guide-Migration, Redirects, Bereinigung und Veröffentlichung.

Jeder PR ist separat baubar und lässt die vorhandene Dokumentation bis zur
kontrollierten Umschaltung erreichbar. So werden Design, Vertragstreue und
Übersetzungsqualität jeweils überschaubar reviewbar.

## 10. Definition of Done für jede zukünftige API-Änderung

Eine API-Änderung gilt erst als fertig, wenn:

- Header und Implementierung samt Tests aktualisiert sind;
- das Contract-Schema alle neuen oder geänderten Attribute enthält;
- deutsche und englische Erklärung vorhanden sind;
- Ownership, Lifetime, Threading, Fehlerfälle und Versionswirkung dokumentiert sind;
- mindestens ein realistisches Beispiel aktualisiert wurde;
- Generator, Schema-Prüfung, Symbolabgleich, Linkcheck und beide mdBook-Builds grün
  sind.

Damit wird die Dokumentation nicht erneut „zerpflückt“: Der maschinenlesbare Vertrag
liefert die vollständige Referenz, während Konzepte und Guides eine klare,
aufgabenorientierte Lernstrecke bilden.
