# API-Referenz

Diese Referenz wird aus validierten Contracts erzeugt und spiegelt die öffentlichen C-Header. Jede Service-Seite beginnt mit Provider, Version, Status, Capability und Header; danach folgen feldvollständige Typen und Function-Contracts.

## Eine Funktion lesen

- **Richtung** unterscheidet Input, Output und initialisierten Input/Output-Speicher.
- **Pflicht** sagt, ob das Argument unter dokumentierten Bedingungen fehlen darf.
- **Ownership** und **Threading** gelten zusätzlich zu parameterspezifischen Regeln.
- **Ergebnisse** nennen relevante Ursachen; unbekannte zukünftige Fehler immer sicher behandeln.
- **Seit** bezeichnet die Minor-Version des Service, nicht das Repository-Release.

Die Header bleiben das normative Binärlayout. Wenn generierte Referenz und Verhalten widersprechen, einen Bug melden und keinem von beiden still vertrauen.
