# Buffer

Rufe größenbasierte Leseoperationen zuerst mit Null-Puffer und Kapazität null auf, wenn der Vertrag eine Größenabfrage erlaubt. Reserviere die gemeldete Größe, initialisiere erforderliche Ausgabestrukturen und rufe erneut auf. `StringView` und mehrere Log-Puffer sind Bytebereiche ohne garantiertes abschließendes Nullbyte.
