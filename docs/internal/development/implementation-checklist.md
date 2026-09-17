# Implementation Matrix

A row is complete when contract, core implementation, automated verification, and
documentation are available.

| Status | Area | Contract | Core | Test |
|:---:|---|:---:|:---:|:---:|
| ✅ | Mod lifecycle and discovery | ✅ | ✅ | ✅ |
| ✅ | Services, events, commands, and settings | ✅ | ✅ | ✅ |
| ✅ | Capabilities and permissions | ✅ | ✅ | ✅ |
| ✅ | Runtime patches and detours | ✅ | ✅ | ✅ |
| ✅ | Logging and session rotation | ✅ | ✅ | ✅ |
| ✅ | Current-log reader | ✅ | ✅ | ✅ |
| ✅ | Native text UI | ✅ | ✅ | ✅ |
| ✅ | KFC3 reflection and resource blocks | ✅ | ✅ | ✅ |
| ✅ | Typed Asset JSON operations | ✅ | ✅ | ✅ |
| ✅ | Asset ownership, staging, and recovery | ✅ | ✅ | ✅ |
| 🧪 | World entities and grids | ✅ | ❌ | Test provider |
| 🚧 | Targeting and semantic game actions | Design | ❌ | ❌ |
| 🚧 | Game-thread jobs | Design | ❌ | ❌ |
| 🚧 | World overlay | Capability | ❌ | ❌ |
| 🚧 | Content, blob, image, and audio API | Design | Partial engine | ❌ |
| 🚧 | Archive log API | Design | Session files | ❌ |

The parser source under `third-party/kfc-parser` is an internal build dependency.
Community mods use headers from `api/include/shroudtopia` only.
