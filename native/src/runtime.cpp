#include "zerovm/zerovm.h"
#include <atomic>
#include <cstring>
#include <mutex>
#include <new>
#include <unordered_set>
namespace {
std::mutex g_mutex;
std::unordered_set<zvm_handle> g_runtimes;
std::atomic<zvm_handle> g_next_handle{1};
void clear_error(zvm_error_info* e) {
 if (!e || e->struct_size < sizeof(zvm_error_info)) return;
 e->code=ZVM_STATUS_OK; e->domain=0; e->reserved=0; e->operation_id=0; e->message[0]='\0';
}
zvm_status fail(zvm_status c,const char* m,zvm_error_info* e) {
 if (e && e->struct_size >= sizeof(zvm_error_info)) {
  e->code=c; std::strncpy(e->message,m,ZVM_ERROR_MESSAGE_CAPACITY-1);
  e->message[ZVM_ERROR_MESSAGE_CAPACITY-1]='\0';
 }
 return c;
}
}
extern "C" ZVM_API uint32_t zvm_get_abi_version(void) { return ZVM_ABI_VERSION; }
extern "C" ZVM_API zvm_status zvm_runtime_create(const zvm_runtime_options* o,zvm_handle* out,zvm_error_info* e) {
 clear_error(e);
 if (!o || !out || o->struct_size < sizeof(zvm_runtime_options))
  return fail(ZVM_STATUS_INVALID_ARGUMENT,"Invalid runtime options or output",e);
 if (o->abi_version != ZVM_ABI_VERSION)
  return fail(ZVM_STATUS_ABI_MISMATCH,"Unsupported ZeroVM ABI version",e);
 const auto h=g_next_handle.fetch_add(1,std::memory_order_relaxed);
 if (!h) return fail(ZVM_STATUS_RESOURCE_LIMIT,"Runtime handle space exhausted",e);
 try { std::lock_guard<std::mutex> lock(g_mutex); g_runtimes.insert(h); }
 catch (const std::bad_alloc&) { return fail(ZVM_STATUS_OUT_OF_MEMORY,"Runtime registry allocation failed",e); }
 catch (...) { return fail(ZVM_STATUS_INTERNAL,"Unexpected runtime registry failure",e); }
 *out=h; return ZVM_STATUS_OK;
}
extern "C" ZVM_API zvm_status zvm_runtime_destroy(zvm_handle h,zvm_error_info* e) {
 clear_error(e);
 if (!h) return fail(ZVM_STATUS_INVALID_ARGUMENT,"Runtime handle must be non-zero",e);
 std::lock_guard<std::mutex> lock(g_mutex);
 if (!g_runtimes.erase(h)) return fail(ZVM_STATUS_STALE_HANDLE,"Runtime handle is no longer valid",e);
 return ZVM_STATUS_OK;
}
