# Runtime patches and text UI

Runtime patches require `shroudtopia.runtime.patches`. Validate signatures against every supported game profile and require one unique executable-section match. `overwrite_size` must cover complete instructions. Create the patch inactive, enable it when needed, and release it to restore the original bytes.

The Text UI service creates one to eight tabbed native windows asynchronously. Poll `get_status`, replace tab snapshots with at most one MiB UTF-8, and destroy the handle during cleanup. It supports windowed/borderless Windows clients, not exclusive fullscreen. The service never calls back into the mod.
