# ABI and versioning

Shroudtopia uses a C ABI at the DLL boundary. Native mods request `ST_ABI_VERSION_1`; exceptions and C++ library objects must never cross that boundary.

## `struct_size`

Every extensible structure starts with `struct_size`. Initialize it with `sizeof(value)`. A receiver rejects undersized required prefixes and may ignore a larger tail it does not know. Before reading an optional function-table tail, compare the table size with `offsetof` plus the field size.

```cpp
ST_ModDescriptorV1 descriptor{};
descriptor.struct_size = sizeof(descriptor);
```

## Versions

- Increase **major** for a breaking layout or semantic change.
- Increase **minor** only when the existing prefix remains compatible.
- Consumers request a major and minimum minor version.
- Always check the result, returned pointer, table size, and ABI version where present.
