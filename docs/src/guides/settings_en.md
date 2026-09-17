# Mod settings, game-start settings, and actions

The API deliberately separates stored configuration from executable behavior.

## Mod settings

`mod_settings` belongs to one Mod. Use `register_mod_settings` to declare its schema and defaults, then read typed values with `get_mod_setting_bool` or `get_mod_setting_number`. A setting describes configuration; it is never used as a button or one-shot trigger.

## Game-start settings

`get_game_setting` reads the effective value. `stage_game_setting` validates JSON and stores a pending value for the next game start. `reset_game_setting` removes that pending override. Pending values never pretend to be effective in the running session. Game-start settings are applied before Mods are activated.

## In-game actions

Register executable behavior with `register_action`. Invoke it with `invoke_action` only while its required session and capability are available. `get_action_state` reports whether it is available, disabled, running, or failed. Commands, hotkeys, and UI buttons should call the same action rather than duplicate the implementation.
