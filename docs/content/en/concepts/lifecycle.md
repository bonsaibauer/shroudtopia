# Mod lifecycle

`CreateModFunction` describes a mod through `ModDescriptor`. The loader invokes `on_load`, `on_activate`, `on_update`, `on_deactivate`, and `on_unload` in lifecycle order.

Acquire long-lived registrations during load or activation, release them during deactivation, and keep unload idempotent. Never throw C++ exceptions across the C ABI boundary.
