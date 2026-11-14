#include "zerodroid/zerodroid.h"
#include <atomic>
#include <cstring>
#include <mutex>
#include <new>
#include <system_error>
#include <unordered_set>

namespace {
std::mutex g_mutex;
std::unordered_set<zdroid_handle> g_runtimes;
std::atomic<zdroid_handle> g_next_handle{1};

bool allocate_handle(zdroid_handle* out) noexcept {
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

extern "C" ZDROID_API uint32_t zdroid_get_abi_version(void) {
    return ZDROID_ABI_VERSION;
}

extern "C" ZDROID_API zdroid_status zdroid_runtime_create(
    const zdroid_runtime_options* options, zdroid_handle* out_runtime, zdroid_error_info* out_error) {
    clear_error(out_error);
    if (out_runtime != nullptr) *out_runtime = 0;
    if (options == nullptr || out_runtime == nullptr ||
        options->struct_size < sizeof(zdroid_runtime_options)) {
        return fail(ZDROID_STATUS_INVALID_ARGUMENT, "Invalid runtime options or output", out_error);
    }
    if (options->abi_version != ZDROID_ABI_VERSION) {
        return fail(ZDROID_STATUS_ABI_MISMATCH, "Unsupported ZeroDroid ABI version", out_error);
    }

    zdroid_handle handle = 0;
    if (!allocate_handle(&handle)) {
        return fail(ZDROID_STATUS_RESOURCE_LIMIT, "Runtime handle space exhausted", out_error);
    }
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        const auto inserted = g_runtimes.insert(handle).second;
        if (!inserted) {
            return fail(ZDROID_STATUS_INTERNAL, "Runtime handle collision", out_error);
        }
    } catch (const std::bad_alloc&) {
        return fail(ZDROID_STATUS_OUT_OF_MEMORY, "Runtime registry allocation failed", out_error);
    } catch (...) {
        return fail(ZDROID_STATUS_INTERNAL, "Unexpected runtime registry failure", out_error);
    }

    *out_runtime = handle;
    return ZDROID_STATUS_OK;
}

extern "C" ZDROID_API zdroid_status zdroid_runtime_destroy(zdroid_handle runtime, zdroid_error_info* out_error) {
    clear_error(out_error);
    if (runtime == 0) {
        return fail(ZDROID_STATUS_INVALID_ARGUMENT, "Runtime handle must be non-zero", out_error);
    }
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_runtimes.erase(runtime) == 0) {
            return fail(ZDROID_STATUS_STALE_HANDLE, "Runtime handle is no longer valid", out_error);
        }
        return ZDROID_STATUS_OK;
    } catch (...) {
        return fail(ZDROID_STATUS_INTERNAL, "Unexpected runtime registry failure", out_error);
    }
}
