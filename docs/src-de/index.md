<div class="hero">

# Native Mods mit Sicherheit entwickeln

Eine zusammenhängende Referenz für Shroudtopias stabile C-ABI: exakte Felder, Ownership, Fehler, Berechtigungen und vollständige Abläufe.

**Hier beginnen:** [erste Mod erstellen](./guides/first-mod.md) oder die [API-Referenz](./reference/index.md) öffnen.

</div>

## Was du entwickeln kannst

| Bereich | Verfügbarkeit | Einstieg |
|---|:---:|---|
| Lifecycle, Registries, Settings, Logging | ✅ Stabil | [`ST_HostApiV1`](./reference/generated/host-api-v1.md) |
| Typisierte KFC3-Ressourcenbearbeitung | ✅ Stabil | [Asset API 1.1](./reference/generated/assets-v1.1.md) |
| Kontrollierte Prozess-Patches und Detours | ✅ Stabil | [Runtime Patch API 1.0](./reference/generated/runtime-patches-v1.0.md) |
| Native Textfenster mit Tabs | ✅ Stabil | [Text UI API 1.0](./reference/generated/text-ui-v1.0.md) |
| Entities und Welt-Grids | 🧪 Test-Provider | [World API 1.1](./reference/generated/world-v1.1.md) |

## So funktioniert diese Dokumentation

Guides lösen eine Aufgabe von Anfang bis Ende. Konzepte erklären Service-übergreifende Regeln. Die generierte Referenz nennt jedes dokumentierte C-Feld, jeden Parameter und Rückgabewert sowie Berechtigung, Ownership und Service-Version. CI gleicht die Contracts mit den öffentlichen Headern ab.
