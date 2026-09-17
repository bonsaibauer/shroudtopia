# Ownership, buffers, and lifetime

Assume every input pointer and `StringView` is borrowed only for the call unless the function says it copies data. A view is a byte span, not necessarily NUL-terminated. `data` may be null only when `size` is zero.

## Output buffers

Size-query APIs use two calls: first pass null/zero capacity to receive the required size, allocate caller-owned storage, then call again. Check the exact function contract: output commonly has no NUL terminator.

## Handles and cleanup

`Registration`, `RuntimePatch`, and `TextWindow` are opaque, owner-bound handles. Release individual handles when they are no longer needed. The loader automatically removes all remaining resources after a mod unloads. Never use a handle after release or in another owner context.
