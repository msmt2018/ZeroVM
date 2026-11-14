package dev.zerodroid.core.nativebridge

enum class HostArchitecture(val wireValue: Int) {
    UNKNOWN(0), AARCH64(1), X86_64(2), OTHER(3);

    companion object {
        fun fromWireValue(value: Int): HostArchitecture =
            values().firstOrNull { it.wireValue == value } ?: UNKNOWN
    }
}

enum class KvmProbeState(val wireValue: Int) {
    UNKNOWN(0),
    AVAILABLE(1),
    UNAVAILABLE(2),
    ACCESS_DENIED(3),
    INCOMPATIBLE(4),
    ERROR(5);

    companion object {
        fun fromWireValue(value: Int): KvmProbeState =
            values().firstOrNull { it.wireValue == value } ?: UNKNOWN
    }
}

/** Process-visible native host evidence; this is not an end-to-end VM readiness result. */
data class NativeHostCapabilities(
    val abiVersion: Int,
    val architecture: HostArchitecture,
    val kvmState: KvmProbeState,
    val kvmApiVersion: Int,
    /** errno captured at the failed open/ioctl call, or zero when not applicable. */
    val kvmErrno: Int
)
