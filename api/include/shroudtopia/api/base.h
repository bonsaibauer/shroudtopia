#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(SHROUDTOPIA_EXPORTS)
#    define ST_API __declspec(dllexport)
#  else
#    define ST_API __declspec(dllimport)
#  endif
#  define ST_CALL __cdecl
#else
#  define ST_API
#  define ST_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ST_ABI_VERSION_1 UINT32_C(0x00010000)

typedef struct ST_StringView { const char* data; size_t size; } ST_StringView;
typedef uint64_t ST_Registration;

typedef enum ST_Result {
    ST_RESULT_OK = 0,
    ST_RESULT_INVALID_ARGUMENT = 1,
    ST_RESULT_ALREADY_EXISTS = 2,
    ST_RESULT_NOT_FOUND = 3,
    ST_RESULT_VERSION_MISMATCH = 4,
    ST_RESULT_PERMISSION_DENIED = 5,
    ST_RESULT_UNSUPPORTED = 6,
    ST_RESULT_CALLBACK_FAILED = 7,
    ST_RESULT_INTERNAL_ERROR = 8
} ST_Result;


#ifdef __cplusplus
}
#endif
