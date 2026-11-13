#ifndef ZEROVM_ZEROVM_H
#define ZEROVM_ZEROVM_H
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32)
# if defined(ZEROVM_BUILDING_LIBRARY)
#  define ZVM_API __declspec(dllexport)
# else
#  define ZVM_API __declspec(dllimport)
# endif
#elif defined(__GNUC__) || defined(__clang__)
# define ZVM_API __attribute__((visibility("default")))
#else
# define ZVM_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define ZVM_ABI_VERSION 1u
#define ZVM_ERROR_MESSAGE_CAPACITY 256u
typedef uint64_t zvm_handle;
typedef uint32_t zvm_status;
enum zvm_status_code {
 ZVM_STATUS_OK=0u, ZVM_STATUS_INVALID_ARGUMENT=1u, ZVM_STATUS_ABI_MISMATCH=2u,
 ZVM_STATUS_OUT_OF_MEMORY=3u, ZVM_STATUS_INVALID_HANDLE=4u, ZVM_STATUS_STALE_HANDLE=5u,
 ZVM_STATUS_UNSUPPORTED_CAPABILITY=6u, ZVM_STATUS_BACKEND_UNAVAILABLE=7u,
 ZVM_STATUS_INVALID_STATE=8u, ZVM_STATUS_RESOURCE_LIMIT=9u, ZVM_STATUS_INTERNAL=255u
};
typedef struct zvm_runtime_options { uint32_t abi_version; uint32_t struct_size; uint64_t flags; } zvm_runtime_options;
typedef struct zvm_error_info {
 uint32_t struct_size; uint32_t code; uint32_t domain; uint32_t reserved;
 uint64_t operation_id; char message[ZVM_ERROR_MESSAGE_CAPACITY];
} zvm_error_info;
ZVM_API uint32_t zvm_get_abi_version(void);
ZVM_API const char* zvm_status_name(zvm_status status);
ZVM_API zvm_status zvm_runtime_create(const zvm_runtime_options* options, zvm_handle* out_runtime, zvm_error_info* out_error);
ZVM_API zvm_status zvm_runtime_destroy(zvm_handle runtime, zvm_error_info* out_error);
#ifdef __cplusplus
}
#endif
#endif
