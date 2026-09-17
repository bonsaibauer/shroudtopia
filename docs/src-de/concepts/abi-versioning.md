# ABI und Versionierung

Shroudtopia verwendet an der DLL-Grenze eine C-ABI. Native Mods fordern `ST_ABI_VERSION_1` an; Exceptions und C++-Bibliotheksobjekte dürfen diese Grenze nie überschreiten.

## `struct_size`

Jede erweiterbare Struktur beginnt mit `struct_size`. Initialisiere das Feld mit `sizeof(value)`. Ein Empfänger weist zu kleine Pflichtpräfixe ab und darf einen unbekannten größeren Tail ignorieren. Vergleiche vor dem Lesen eines optionalen Function-Table-Tails die Tabellengröße mit `offsetof` plus Feldgröße.

```cpp
ST_ModDescriptorV1 descriptor{};
descriptor.struct_size = sizeof(descriptor);
```

## Versionen

- **Major** bei inkompatibler Layout- oder Semantikänderung erhöhen.
- **Minor** nur erhöhen, wenn das bestehende Präfix kompatibel bleibt.
- Consumer fordern eine Major- und minimale Minor-Version an.
- Immer Ergebnis, Pointer, Tabellengröße und gegebenenfalls ABI-Version prüfen.
