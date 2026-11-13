#include "zerovm/zerovm.h"
#include <stddef.h>

_Static_assert(sizeof(zvm_handle) == 8, "zvm_handle ABI must remain 64-bit");
_Static_assert(offsetof(zvm_runtime_options, abi_version) == 0, "abi_version offset changed");
_Static_assert(offsetof(zvm_runtime_options, struct_size) == 4, "struct_size offset changed");
_Static_assert(offsetof(zvm_runtime_options, flags) == 8, "flags offset changed");

int main(void) {
    zvm_runtime_options options = { ZVM_ABI_VERSION, sizeof(zvm_runtime_options), 0 };
    zvm_error_info error = {0};
    error.struct_size = sizeof(zvm_error_info);
    zvm_handle runtime = 0;
    if (zvm_get_abi_version() != ZVM_ABI_VERSION) return 1;
    if (zvm_runtime_create(&options, &runtime, &error) != ZVM_STATUS_OK) return 2;
    if (runtime == 0) return 3;
    if (zvm_runtime_destroy(runtime, &error) != ZVM_STATUS_OK) return 4;
    return 0;
}
