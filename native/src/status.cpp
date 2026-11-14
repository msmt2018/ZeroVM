#include "zerodroid/zerodroid.h"
extern "C" ZVM_API const char* zvm_status_name(zvm_status status) {
 switch (status) {
 case ZVM_STATUS_OK: return "OK";
 case ZVM_STATUS_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
 case ZVM_STATUS_ABI_MISMATCH: return "ABI_MISMATCH";
 case ZVM_STATUS_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
 case ZVM_STATUS_INVALID_HANDLE: return "INVALID_HANDLE";
 case ZVM_STATUS_STALE_HANDLE: return "STALE_HANDLE";
 case ZVM_STATUS_UNSUPPORTED_CAPABILITY: return "UNSUPPORTED_CAPABILITY";
 case ZVM_STATUS_BACKEND_UNAVAILABLE: return "BACKEND_UNAVAILABLE";
 case ZVM_STATUS_INVALID_STATE: return "INVALID_STATE";
 case ZVM_STATUS_RESOURCE_LIMIT: return "RESOURCE_LIMIT";
 case ZVM_STATUS_INTERNAL: return "INTERNAL";
 default: return "UNKNOWN_STATUS";
 }
}
