package dev.zerovm.core.nativebridge
/** JNI loading boundary; VM operations remain unavailable until a real backend is integrated. */
object NativeRuntime {
 init { System.loadLibrary("zerovm_jni") }
 external fun abiVersion():Int
}
