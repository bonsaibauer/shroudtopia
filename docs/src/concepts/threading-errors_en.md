# Threads, callbacks, and errors

Lifecycle `on_update` runs on Shroudtopia's worker thread, not the game thread. Do not call unverified engine functions. Events and command callbacks are synchronous on the invoking thread. Keep callbacks bounded and never let an exception cross the API boundary.

Handle the specific `Result`: `NOT_FOUND` and `NOT_AVAILABLE` are often expected feature states; `PERMISSION_DENIED` needs manifest/user action; `CALLBACK_FAILED` identifies extension code; `INTERNAL_ERROR` is reserved for a host failure. Do not collapse every failure into `INTERNAL_ERROR`.

A failed grid write may be partial. Preserve recovery data before performing destructive operations.
