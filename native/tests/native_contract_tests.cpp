#include "zerovm/zerovm.h"
#include <cassert>
#include <cstddef>

int main() {
    static_assert(sizeof(zvm_handle) == 8);
    static_assert(offsetof(zvm_runtime_options, abi_version) == 0);
    static_assert(offsetof(zvm_runtime_options, struct_size) == 4);
    static_assert(offsetof(zvm_error_info, operation_id) == 16);
    assert(zvm_get_abi_version() == ZVM_ABI_VERSION);
    assert(zvm_status_name(ZVM_STATUS_OK)[0] == 'O');

    zvm_runtime_options options{ZVM_ABI_VERSION, sizeof(zvm_runtime_options), 0};
    zvm_error_info error{};
    error.struct_size = sizeof(zvm_error_info);
    zvm_handle runtime = 0;

    assert(zvm_runtime_create(&options, &runtime, &error) == ZVM_STATUS_OK);
    assert(runtime != 0);
    assert(error.code == ZVM_STATUS_OK);
    assert(zvm_runtime_destroy(runtime, &error) == ZVM_STATUS_OK);
    assert(zvm_runtime_destroy(runtime, &error) == ZVM_STATUS_STALE_HANDLE);

    runtime = 99;
    options.abi_version = 99;
    assert(zvm_runtime_create(&options, &runtime, &error) == ZVM_STATUS_ABI_MISMATCH);
    assert(runtime == 0);
    assert(error.code == ZVM_STATUS_ABI_MISMATCH);

    assert(zvm_runtime_destroy(0, &error) == ZVM_STATUS_INVALID_ARGUMENT);
    return 0;
}
