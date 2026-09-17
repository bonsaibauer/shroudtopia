<div class="hero">

# Build native mods with confidence

A coherent reference for Shroudtopia's stable C ABI: exact fields, ownership, errors, permissions, and complete workflows.

**Start here:** [build your first mod](./guides/first-mod.md) or open the [API reference](./reference/index.md).

</div>

## What you can build

| Area | Availability | Entry point |
|---|:---:|---|
| Lifecycle, registries, settings, logging | ✅ Stable | [`ST_HostApiV1`](./reference/generated/host-api-v1.md) |
| Typed KFC3 resource editing | ✅ Stable | [Asset API 1.1](./reference/generated/assets-v1.1.md) |
| Controlled process patches and detours | ✅ Stable | [Runtime Patch API 1.0](./reference/generated/runtime-patches-v1.0.md) |
| Native tabbed text windows | ✅ Stable | [Text UI API 1.0](./reference/generated/text-ui-v1.0.md) |
| Entities and world grids | 🧪 Test provider | [World API 1.1](./reference/generated/world-v1.1.md) |

## How this documentation works

Guides solve an end-to-end task. Concepts explain rules that apply across services. The generated reference lists every documented C field, parameter, result, permission, ownership rule, and service version. Contract files are checked against public headers during CI.
