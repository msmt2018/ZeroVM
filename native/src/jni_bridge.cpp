#include <jni.h>
#include "zerodroid/zerodroid.h"
extern "C" JNIEXPORT jint JNICALL
Java_dev_zerodroid_core_nativebridge_NativeRuntime_abiVersion(JNIEnv*, jobject) {
 return static_cast<jint>(zdroid_get_abi_version());
}

extern "C" JNIEXPORT jintArray JNICALL
Java_dev_zerodroid_core_nativebridge_NativeRuntime_probeHostCapabilitiesRaw(JNIEnv* env, jobject) {
 zdroid_host_capabilities capabilities{};
 capabilities.struct_size = sizeof(capabilities);
 zdroid_error_info error{};
 error.struct_size = sizeof(error);
 if (zdroid_host_probe_capabilities(&capabilities, &error) != ZDROID_STATUS_OK) return nullptr;
 const jint values[] = {
  static_cast<jint>(capabilities.abi_version),
  static_cast<jint>(capabilities.architecture),
  static_cast<jint>(capabilities.kvm_state),
  static_cast<jint>(capabilities.kvm_api_version),
  static_cast<jint>(capabilities.kvm_errno)
 };
 jintArray result = env->NewIntArray(5);
 if (result == nullptr) return nullptr;
 env->SetIntArrayRegion(result, 0, 5, values);
 return result;
}
