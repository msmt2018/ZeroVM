package dev.zerodroid.core.nativebridge
/** JNI loading boundary; VM operations remain unavailable until a real backend is integrated. */
object NativeRuntime {
 init { System.loadLibrary("zerodroid_jni") }
 external fun abiVersion():Int
}
