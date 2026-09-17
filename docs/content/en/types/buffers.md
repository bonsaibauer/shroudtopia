# Buffers

For capacity-based reads, first call with a null output buffer and zero capacity when the operation permits a capacity query. Allocate the reported size, initialize output structures where required, and call again. A `StringView` and several log buffers are byte ranges and do not imply a trailing null byte.
