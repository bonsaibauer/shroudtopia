# StringView

`StringView` ist ein geliehener UTF-8-Bytebereich aus `data` und `size`. Eine Nullterminierung ist nicht garantiert. Lies niemals über `size` hinaus und kopiere die Bytes, wenn sie länger als der dokumentierte Aufruf oder Callback gültig bleiben müssen.
