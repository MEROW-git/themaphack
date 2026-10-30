#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_org_themaphack_diagnostics_MainActivity_nativeBuildInfo(JNIEnv *env, jclass) {
#if defined(__aarch64__)
    const char *abi = "arm64-v8a";
#else
    const char *abi = "unexpected ABI";
#endif
    const std::string info = std::string(abi) + ", C++17, pointer size " +
                             std::to_string(sizeof(void *) * 8) + " bits";
    return env->NewStringUTF(info.c_str());
}
