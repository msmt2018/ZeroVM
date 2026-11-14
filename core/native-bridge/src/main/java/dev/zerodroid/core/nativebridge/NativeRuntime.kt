package dev.zerodroid.core.nativebridge

/** JNI loading boundary; VM operations remain unavailable until a real backend is integrated. */
object NativeRuntime {
    init { System.loadLibrary("zerodroid_jni") }

    external fun abiVersion(): Int
    private external fun probeHostCapabilitiesRaw(): IntArray?

    /**
     * Probes the process-visible KVM device interface.
     *
     * This does not prove AVF authorization, VM creation permission, EL2 ownership,
     * guest boot support, or end-to-end virtualization. A denied probe is preserved
     * as a distinct state and is never treated as an invitation to use CPU emulation.
     */
    fun probeHostCapabilities(): NativeHostCapabilities {
        val values = checkNotNull(probeHostCapabilitiesRaw()) {
            "Native host capability probe failed"
        }
        require(values.size == 5) { "Unexpected native host capability payload" }
        return NativeHostCapabilities(
            abiVersion = values[0],
            architecture = HostArchitecture.fromWireValue(values[1]),
            kvmState = KvmProbeState.fromWireValue(values[2]),
            kvmApiVersion = values[3],
            kvmErrno = values[4]
        )
    }
}
