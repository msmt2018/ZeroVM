#include "zerodroid/zerodroid.h"
#include <cstdio>
#include <cstddef>

#define CHECK(expression) do { \
    if (!(expression)) { \
        std::fprintf(stderr, "CHECK failed at %s:%d: %s\\n", __FILE__, __LINE__, #expression); \
        return 1; \
    } \
} while (false)

int main() {
    static_assert(sizeof(zdroid_handle) == 8);
    static_assert(offsetof(zdroid_runtime_options, abi_version) == 0);
    static_assert(offsetof(zdroid_runtime_options, struct_size) == 4);
    static_assert(offsetof(zdroid_error_info, operation_id) == 16);
    CHECK(zdroid_get_abi_version() == ZDROID_ABI_VERSION);
    CHECK(zdroid_status_name(ZDROID_STATUS_OK)[0] == 'O');

    zdroid_runtime_options options{ZDROID_ABI_VERSION, sizeof(zdroid_runtime_options), 0};
    zdroid_error_info error{};
    error.struct_size = sizeof(zdroid_error_info);
    zdroid_handle runtime = 0;

    CHECK(zdroid_runtime_create(&options, &runtime, &error) == ZDROID_STATUS_OK);
    CHECK(runtime != 0);
    CHECK(error.code == ZDROID_STATUS_OK);
    CHECK(zdroid_runtime_destroy(runtime, &error) == ZDROID_STATUS_OK);
    CHECK(zdroid_runtime_destroy(runtime, &error) == ZDROID_STATUS_STALE_HANDLE);

    runtime = 99;
    options.abi_version = 99;
    CHECK(zdroid_runtime_create(&options, &runtime, &error) == ZDROID_STATUS_ABI_MISMATCH);
    CHECK(runtime == 0);
    CHECK(error.code == ZDROID_STATUS_ABI_MISMATCH);

    CHECK(zdroid_runtime_destroy(0, &error) == ZDROID_STATUS_INVALID_ARGUMENT);
    return 0;
}
