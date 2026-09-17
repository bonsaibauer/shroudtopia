# Mod Settings, Spielstart-Einstellungen und Actions

Die API trennt gespeicherte Konfiguration bewusst von ausführbarem Verhalten.

## Mod Settings

`mod_settings` gehört genau einer Mod. Mit `register_mod_settings` werden Schema und Standardwerte registriert; typisierte Werte liest die Mod über `get_mod_setting_bool` oder `get_mod_setting_number`. Ein Setting beschreibt Konfiguration und wird niemals als Button oder einmaliger Trigger verwendet.

## Spielstart-Einstellungen

`get_game_setting` liest den effektiven Wert. `stage_game_setting` validiert JSON und speichert einen ausstehenden Wert für den nächsten Spielstart. `reset_game_setting` entfernt diesen ausstehenden Override. Vorgemerkte Werte werden in der laufenden Session niemals als bereits aktiv dargestellt. Spielstart-Einstellungen werden angewandt, bevor Mods aktiviert werden.

## Ingame-Actions

Ausführbares Verhalten wird mit `register_action` registriert. `invoke_action` führt es nur aus, wenn Session und Capability passen. `get_action_state` meldet `available`, `disabled`, `running` oder `failed`. Commands, Hotkeys und UI-Buttons sollen dieselbe Action aufrufen, statt die Funktion mehrfach zu implementieren.
