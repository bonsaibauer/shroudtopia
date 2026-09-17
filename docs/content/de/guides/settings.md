# Konfiguration

`shroudtopia.json` enthält Laufzeit- und Mod-Einstellungen:

```json
{
  "active": true,
  "updateDelay": 500,
  "enableLogging": true,
  "logLevel": "INFO",
  "mods": {
    "mod.example": {
      "enabled": true,
      "multiplier": 1.5
    }
  }
}
```

| Einstellung | Werte | Wirkung |
|---|---|---|
| `active` | `true`, `false` | Aktiviert oder deaktiviert konfigurierte Mods. |
| `updateDelay` | Millisekunden | Legt das Aktualisierungsintervall fest. |
| `enableLogging` | `true`, `false` | Aktiviert oder deaktiviert die Shroudtopia-Logausgabe. |
| `logLevel` | `ALL`, `TRACE`, `DEBUG`, `INFO`, `WARNING`, `ERROR` | `ALL` schreibt alle Stufen. Jeder andere Wert schreibt nur die gewählte Stufe. |

Die Debug Console verwendet dieselbe Einstellung. `INFO` zeigt im Level-Schalter `All` und `Info`. `ALL` zeigt alle Level.

Mods lesen ihre eigenen Werte mit `get_mod_setting_bool` und `get_mod_setting_number`. Actions verwenden `register_action`, `invoke_action` und `get_action_state`.
