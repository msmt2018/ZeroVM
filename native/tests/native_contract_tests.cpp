#include "zerodroid/zerodroid.h"
#include <cassert>
#include <cstddef>

int main() {
    static_assert(sizeof(zdroid_handle) == 8);
    static_assert(offsetof(zdroid_runtime_options, abi_version) == 0);
    static_assert(offsetof(zdroid_runtime_options, struct_size) == 4);
    static_assert(offsetof(zdroid_error_info, operation_id) == 16);
    assert(zdroid_get_abi_version() == ZDROID_ABI_VERSION);
    assert(zdroid_status_name(ZDROID_STATUS_OK)[0] == 'O');

    zdroid_runtime_options options{ZDROID_ABI_VERSION, sizeof(zdroid_runtime_options), 0};
    zdroid_error_info error{};
    error.struct_size = sizeof(zdroid_error_info);
    zdroid_handle runtime = 0;

    assert(zdroid_runtime_create(&options, &runtime, &error) == ZDROID_STATUS_OK);
    assert(runtime != 0);
    assert(error.code == ZDROID_STATUS_OK);
    assert(zdroid_runtime_destroy(runtime, &error) == ZDROID_STATUS_OK);
    assert(zdroid_runtime_destroy(runtime, &error) == ZDROID_STATUS_STALE_HANDLE);

    runtime = 99;
    options.abi_version = 99;
    assert(zdroid_runtime_create(&options, &runtime, &error) == ZDROID_STATUS_ABI_MISMATCH);
    assert(runtime == 0);
    assert(error.code == ZDROID_STATUS_ABI_MISMATCH);

    assert(zdroid_runtime_destroy(0, &error) == ZDROID_STATUS_INVALID_ARGUMENT);
    return 0;
}
