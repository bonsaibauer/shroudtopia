# Callbacks

Callbacks use `CALL`, return `Result`, and receive opaque `user_data` where applicable. They must not throw exceptions across the ABI boundary, retain borrowed data beyond its lifetime, or recursively invoke operations that are documented as non-reentrant.
