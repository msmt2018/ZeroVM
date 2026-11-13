#include "zerovm/zerovm.h"
#include <cassert>
#include <cstddef>
int main() {
 static_assert(sizeof(zvm_handle)==8);
 static_assert(offsetof(zvm_runtime_options,abi_version)==0);
 static_assert(offsetof(zvm_runtime_options,struct_size)==4);
 assert(zvm_get_abi_version()==ZVM_ABI_VERSION);
 assert(zvm_status_name(ZVM_STATUS_OK)[0]=='O');
 zvm_runtime_options o{ZVM_ABI_VERSION,sizeof(zvm_runtime_options),0};
 zvm_error_info e{}; e.struct_size=sizeof(zvm_error_info);
 zvm_handle h=0;
 assert(zvm_runtime_create(&o,&h,&e)==ZVM_STATUS_OK && h!=0);
 assert(zvm_runtime_destroy(h,&e)==ZVM_STATUS_OK);
 assert(zvm_runtime_destroy(h,&e)==ZVM_STATUS_STALE_HANDLE);
 o.abi_version=99;
 assert(zvm_runtime_create(&o,&h,&e)==ZVM_STATUS_ABI_MISMATCH);
}
