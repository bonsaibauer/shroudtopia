#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(SHROUDTOPIA_EXPORTS)
#    define API_EXPORT __declspec(dllexport)
#  else
#    define API_EXPORT __declspec(dllimport)
#  endif
#  define CALL __cdecl
#else
#  define API_EXPORT
#  define CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define API_VERSION UINT32_C(0x00020000)

typedef struct StringView { const char* data; size_t size; } StringView;
typedef uint64_t Registration;

typedef enum Result {
    RESULT_OK = 0,
    RESULT_INVALID_ARGUMENT = 1,
    RESULT_CONFLICT = 2,
    RESULT_NOT_FOUND = 3,
    RESULT_VERSION_MISMATCH = 4,
    RESULT_PERMISSION_DENIED = 5,
    RESULT_NOT_AVAILABLE = 6,
    RESULT_CALLBACK_FAILED = 7,
    RESULT_INTERNAL_ERROR = 8
} Result;


#ifdef __cplusplus
}
#endif
