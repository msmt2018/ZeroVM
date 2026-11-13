#include <jni.h>
#include "zerovm/zerovm.h"
extern "C" JNIEXPORT jint JNICALL
Java_dev_zerovm_core_nativebridge_NativeRuntime_abiVersion(JNIEnv*, jobject) {
 return static_cast<jint>(zvm_get_abi_version());
}
