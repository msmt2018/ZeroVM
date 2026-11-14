#include "zerodroid/zerodroid.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
constexpr unsigned long kvm_get_api_version = _IO(0xAE, 0x00);
constexpr int expected_kvm_api_version = 12;

void clear_error(zdroid_error_info* error) noexcept {
    if (error == nullptr || error->struct_size < sizeof(zdroid_error_info)) return;
    error->code = ZDROID_STATUS_OK;
    error->domain = 0;
    error->reserved = 0;
    error->operation_id = 0;
    error->message[0] = '\0';
}

zdroid_status fail(zdroid_status code, const char* message, zdroid_error_info* error) noexcept {
    if (error != nullptr && error->struct_size >= sizeof(zdroid_error_info)) {
        error->code = code;
        std::strncpy(error->message, message, ZDROID_ERROR_MESSAGE_CAPACITY - 1);
        error->message[ZDROID_ERROR_MESSAGE_CAPACITY - 1] = '\0';
    }
    return code;
}
} // namespace

extern "C" ZDROID_API zdroid_status zdroid_host_probe_capabilities(
    zdroid_host_capabilities* out_capabilities,
    zdroid_error_info* out_error) {
    clear_error(out_error);
    if (out_capabilities == nullptr ||
        out_capabilities->struct_size < sizeof(zdroid_host_capabilities)) {
        return fail(ZDROID_STATUS_INVALID_ARGUMENT,
                    "Host capability output is null or has an unsupported struct size",
                    out_error);
    }

    const uint32_t requested_size = out_capabilities->struct_size;
    *out_capabilities = {};
    out_capabilities->struct_size = requested_size;
    out_capabilities->abi_version = ZDROID_ABI_VERSION;

#if defined(__aarch64__)
    out_capabilities->architecture = ZDROID_HOST_ARCH_AARCH64;
#elif defined(__x86_64__)
    out_capabilities->architecture = ZDROID_HOST_ARCH_X86_64;
#else
    out_capabilities->architecture = ZDROID_HOST_ARCH_OTHER;
#endif

    const int fd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        const int error_number = errno;
        out_capabilities->kvm_errno = error_number;
        if (error_number == EACCES || error_number == EPERM) {
            out_capabilities->kvm_state = ZDROID_KVM_PROBE_ACCESS_DENIED;
        } else if (error_number == ENOENT || error_number == ENODEV || error_number == ENXIO) {
            out_capabilities->kvm_state = ZDROID_KVM_PROBE_UNAVAILABLE;
        } else {
            out_capabilities->kvm_state = ZDROID_KVM_PROBE_ERROR;
        }
        return ZDROID_STATUS_OK;
    }

    const int api_version = ioctl(fd, kvm_get_api_version, 0);
    const int ioctl_error = api_version < 0 ? errno : 0;
    close(fd);
    if (api_version < 0) {
        out_capabilities->kvm_errno = ioctl_error;
        out_capabilities->kvm_state =
            (ioctl_error == EACCES || ioctl_error == EPERM)
                ? ZDROID_KVM_PROBE_ACCESS_DENIED
                : ZDROID_KVM_PROBE_ERROR;
        return ZDROID_STATUS_OK;
    }

    out_capabilities->kvm_api_version = api_version;
    out_capabilities->kvm_state = api_version == expected_kvm_api_version
        ? ZDROID_KVM_PROBE_AVAILABLE
        : ZDROID_KVM_PROBE_INCOMPATIBLE;
    return ZDROID_STATUS_OK;
}
