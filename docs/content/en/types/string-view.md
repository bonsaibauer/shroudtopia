# StringView

`StringView` is a borrowed UTF-8 byte range consisting of `data` and `size`. It is not guaranteed to be null-terminated. Never read beyond `size`, and copy the bytes when they must outlive the documented call or callback.
