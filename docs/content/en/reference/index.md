# API reference

The reference is generated from public C headers and validated documentation metadata. Identifiers and signatures come from the headers; semantics, ownership, threading, and examples are contract metadata.

| Service | Purpose | Status | Available since |
|---|---|---|---|
| [Actions](./actions.md) | Exposes discoverable operations with JSON Schema inputs and observable state. | Stable | API 1.1 |
| [API](./api.md) | Negotiates and exposes the single flat Shroudtopia interface used by every native mod. | Stable | API 1.1 |
| [Assets](./assets.md) | Lists, reads, creates, updates, patches, saves, and resets structured game assets. | Stable | API 1.1 |
| [Capability information](./capabilities.md) | Describes the negotiated version and availability of a capability. | Stable | API 1.1 |
| [Commands](./commands.md) | Registers named commands with descriptions and callbacks. | Stable | API 1.1 |
| [Core API](./core.md) | Defines the ABI version, common string and registration handles, and the Result error model. | Stable | API 1.1 |
| [Events](./events.md) | Publishes typed event payloads and manages event subscriptions. | Stable | API 1.1 |
| [Mod lifecycle](./lifecycle.md) | Declares a mod and its load, activation, update, deactivation, and unload callbacks. | Stable | API 1.1 |
| [Logging](./logging.md) | Writes structured loader messages and reads bounded tails of loader or game logs. | Stable | API 1.1 |
| [Runtime patches](./runtime-patches.md) | Creates and controls ownership-scoped direct or detour runtime patches. | Experimental | API 1.1 |
| [Service discovery](./services.md) | Publishes extension contracts and resolves providers by exact contract version. | Stable | API 1.1 |
| [Settings](./settings.md) | Reads typed values from the calling mod's configuration. | Stable | API 1.1 |
| [Text UI](./ui.md) | Creates native text windows, updates tab snapshots, observes readiness, and destroys windows. | Experimental | API 1.1 |
