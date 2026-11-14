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
typedef uint32_t zdroid_host_architecture;
enum zdroid_host_architecture_code {
 ZDROID_HOST_ARCH_UNKNOWN=0u, ZDROID_HOST_ARCH_AARCH64=1u,
 ZDROID_HOST_ARCH_X86_64=2u, ZDROID_HOST_ARCH_OTHER=3u
};
typedef uint32_t zdroid_kvm_probe_state;
enum zdroid_kvm_probe_state_code {
 ZDROID_KVM_PROBE_UNKNOWN=0u, ZDROID_KVM_PROBE_AVAILABLE=1u,
 ZDROID_KVM_PROBE_UNAVAILABLE=2u, ZDROID_KVM_PROBE_ACCESS_DENIED=3u,
 ZDROID_KVM_PROBE_INCOMPATIBLE=4u, ZDROID_KVM_PROBE_ERROR=5u
};
typedef struct zdroid_host_capabilities {
 uint32_t struct_size;
 uint32_t abi_version;
 zdroid_host_architecture architecture;
 zdroid_kvm_probe_state kvm_state;
 int32_t kvm_api_version;
 int32_t kvm_errno;
} zdroid_host_capabilities;
typedef struct zdroid_error_info {
 uint32_t struct_size; uint32_t code; uint32_t domain; uint32_t reserved;
 uint64_t operation_id; char message[ZDROID_ERROR_MESSAGE_CAPACITY];
} zdroid_error_info;
ZDROID_API uint32_t zdroid_get_abi_version(void);
ZDROID_API const char* zdroid_status_name(zdroid_status status);
ZDROID_API zdroid_status zdroid_host_probe_capabilities(zdroid_host_capabilities* out_capabilities, zdroid_error_info* out_error);
ZDROID_API zdroid_status zdroid_runtime_create(const zdroid_runtime_options* options, zdroid_handle* out_runtime, zdroid_error_info* out_error);
ZDROID_API zdroid_status zdroid_runtime_destroy(zdroid_handle runtime, zdroid_error_info* out_error);
#ifdef __cplusplus
}
#endif
#endif
