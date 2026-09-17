# API-Domains und Extension Services

`Api` ist der einzige Einstiegspunkt jeder Mod. Eingebaute Domains sind direkte Member: `api->assets`, `api->patches`, `api->ui` und `api->logs`. Für sie sind weder Discovery-Aufruf noch eine zweite öffentliche Funktionstabelle erforderlich.

```cpp
if (!api->assets) return RESULT_NOT_AVAILABLE;
return api->assets->list(owner, type, visitor, context);
```

`register_service` und `find_service` verwalten optionale Contracts von Drittanbieter-Mods. Die Suche verlangt die exakte Haupt- und Nebenversion des Contracts. Ein Provider besitzt seine Extension-Tabelle bis zur Abmeldung; Consumer dürfen sie nicht über das Entladen des Providers hinaus behalten.
