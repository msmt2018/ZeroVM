#ifndef ZERODROID_ZERODROID_H
#define ZERODROID_ZERODROID_H
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32)
# if defined(ZERODROID_BUILDING_LIBRARY)
#  define ZDROID_API __declspec(dllexport)
# else
#  define ZDROID_API __declspec(dllimport)
# endif
#elif defined(__GNUC__) || defined(__clang__)
# define ZDROID_API __attribute__((visibility("default")))
#else
# define ZDROID_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define ZDROID_ABI_VERSION 1u
#define ZDROID_ERROR_MESSAGE_CAPACITY 256u
typedef uint64_t zdroid_handle;
typedef uint32_t zdroid_status;
enum zdroid_status_code {
 ZDROID_STATUS_OK=0u, ZDROID_STATUS_INVALID_ARGUMENT=1u, ZDROID_STATUS_ABI_MISMATCH=2u,
 ZDROID_STATUS_OUT_OF_MEMORY=3u, ZDROID_STATUS_INVALID_HANDLE=4u, ZDROID_STATUS_STALE_HANDLE=5u,
 ZDROID_STATUS_UNSUPPORTED_CAPABILITY=6u, ZDROID_STATUS_BACKEND_UNAVAILABLE=7u,
 ZDROID_STATUS_INVALID_STATE=8u, ZDROID_STATUS_RESOURCE_LIMIT=9u, ZDROID_STATUS_INTERNAL=255u
};
typedef struct zdroid_runtime_options { uint32_t abi_version; uint32_t struct_size; uint64_t flags; } zdroid_runtime_options;
typedef struct zdroid_error_info {
 uint32_t struct_size; uint32_t code; uint32_t domain; uint32_t reserved;
 uint64_t operation_id; char message[ZDROID_ERROR_MESSAGE_CAPACITY];
} zdroid_error_info;
ZDROID_API uint32_t zdroid_get_abi_version(void);
ZDROID_API const char* zdroid_status_name(zdroid_status status);
ZDROID_API zdroid_status zdroid_runtime_create(const zdroid_runtime_options* options, zdroid_handle* out_runtime, zdroid_error_info* out_error);
ZDROID_API zdroid_status zdroid_runtime_destroy(zdroid_handle runtime, zdroid_error_info* out_error);
#ifdef __cplusplus
}
#endif
#endif
