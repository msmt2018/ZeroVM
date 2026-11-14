#include <jni.h>
#include "zerodroid/zerodroid.h"
extern "C" JNIEXPORT jint JNICALL
Java_dev_zerodroid_core_nativebridge_NativeRuntime_abiVersion(JNIEnv*, jobject) {
 return static_cast<jint>(zdroid_get_abi_version());
}
