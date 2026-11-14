#include "zerodroid/zerodroid.h"
#include <atomic>
#include <cstring>
#include <mutex>
#include <new>
#include <system_error>
#include <unordered_set>

namespace {
std::mutex g_mutex;
std::unordered_set<zvm_handle> g_runtimes;
std::atomic<zvm_handle> g_next_handle{1};

bool allocate_handle(zvm_handle* out) noexcept {
    auto candidate = g_next_handle.load(std::memory_order_relaxed);
    while (candidate != UINT64_MAX) {
        if (g_next_handle.compare_exchange_weak(
                candidate, candidate + 1,
                std::memory_order_relaxed, std::memory_order_relaxed)) {
            *out = candidate;
            return true;
        }
    }
    return false;
}

void clear_error(zvm_error_info* error) noexcept {
    if (error == nullptr || error->struct_size < sizeof(zvm_error_info)) return;
    error->code = ZVM_STATUS_OK;
    error->domain = 0;
    error->reserved = 0;
    error->operation_id = 0;
    error->message[0] = '\0';
}

zvm_status fail(zvm_status code, const char* message, zvm_error_info* error) noexcept {
    if (error != nullptr && error->struct_size >= sizeof(zvm_error_info)) {
        error->code = code;
        std::strncpy(error->message, message, ZVM_ERROR_MESSAGE_CAPACITY - 1);
        error->message[ZVM_ERROR_MESSAGE_CAPACITY - 1] = '\0';
    }
    return code;
}
} // namespace

extern "C" ZVM_API uint32_t zvm_get_abi_version(void) {
    return ZVM_ABI_VERSION;
}

extern "C" ZVM_API zvm_status zvm_runtime_create(
    const zvm_runtime_options* options, zvm_handle* out_runtime, zvm_error_info* out_error) {
    clear_error(out_error);
    if (out_runtime != nullptr) *out_runtime = 0;
    if (options == nullptr || out_runtime == nullptr ||
        options->struct_size < sizeof(zvm_runtime_options)) {
        return fail(ZVM_STATUS_INVALID_ARGUMENT, "Invalid runtime options or output", out_error);
    }
    if (options->abi_version != ZVM_ABI_VERSION) {
        return fail(ZVM_STATUS_ABI_MISMATCH, "Unsupported ZeroDroid ABI version", out_error);
    }

    zvm_handle handle = 0;
    if (!allocate_handle(&handle)) {
        return fail(ZVM_STATUS_RESOURCE_LIMIT, "Runtime handle space exhausted", out_error);
    }
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        const auto inserted = g_runtimes.insert(handle).second;
        if (!inserted) {
            return fail(ZVM_STATUS_INTERNAL, "Runtime handle collision", out_error);
        }
    } catch (const std::bad_alloc&) {
        return fail(ZVM_STATUS_OUT_OF_MEMORY, "Runtime registry allocation failed", out_error);
    } catch (...) {
        return fail(ZVM_STATUS_INTERNAL, "Unexpected runtime registry failure", out_error);
    }

    *out_runtime = handle;
    return ZVM_STATUS_OK;
}

extern "C" ZVM_API zvm_status zvm_runtime_destroy(zvm_handle runtime, zvm_error_info* out_error) {
    clear_error(out_error);
    if (runtime == 0) {
        return fail(ZVM_STATUS_INVALID_ARGUMENT, "Runtime handle must be non-zero", out_error);
    }
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_runtimes.erase(runtime) == 0) {
            return fail(ZVM_STATUS_STALE_HANDLE, "Runtime handle is no longer valid", out_error);
        }
        return ZVM_STATUS_OK;
    } catch (...) {
        return fail(ZVM_STATUS_INTERNAL, "Unexpected runtime registry failure", out_error);
    }
}
