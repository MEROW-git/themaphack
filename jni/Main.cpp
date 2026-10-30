#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <thread>
#include <exception>
#include <cerrno>
#include <atomic>
#include <pthread.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <jni.h>
#include <android/log.h>
#include <elf.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dlfcn.h>

// internal module start here
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_additional.h"
#include "imgui_themes.h"
#include <android_native_app_glue.h>
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"
#include "fonts/GoogleSans.h"
#include "imgui/icon.h"

#include "KittyMemory/MemoryPatch.h"

#include "Includes/Logger.h"
#include "Includes/Utils.h"
#include "Includes/Macros.h"

#include "Tools/xDL/xdl.h"
#include "Tools/fake_dlfcn.h"
#include "Tools/Il2Cpp.h"
#include "Tools/Tools.h"

#include "XYZ/Unity/Struct/Vector3.hpp"
#include "XYZ/Unity/Struct/Vector2.h"
#include "XYZ/Unity/Struct/Rect.h"
#include "XYZ/Unity/Struct/Quaternion.h"
#include "XYZ/ConfigName.h"
#include "Config/setup.h"
#include "Config/JNIStuff.h"
#include "StartupForwarder.h"
#include "Config/TouchSystem.h"
#include "XYZ/IconMinimap/DrawIcon.h"
#include "XYZ/GameClass.h"
#include "XYZ/LaylaView.h"
#include "XYZ/ToString.h"
#include "XYZ/SDK.h"
#include "XYZ/DrawMinimap.h"
#include "XYZ/Minimap.h"
#include "XYZ/DrawESP.h"
#include "DrawMenu.h"

#undef stderr
#undef stdout
#undef stdin

#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread> // Added for sleep

using std::chrono::seconds;
using std::this_thread::sleep_for;

#include "xHook/xhook.h"
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#endif
#include "consolas.hpp"
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#include "Render.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

bool PatchOffset(uintptr_t address, const void *buffer, size_t length) {
	unsigned long page_size = sysconf(_SC_PAGESIZE);
    unsigned long size = page_size * sizeof(uintptr_t);
    return mprotect((void *)(address - (address % page_size) - page_size), (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && memcpy((void *)address, (void *)buffer, length) != 0;
}

void detectDump() {
    std::ifstream mapsFile("/proc/self/maps");
    std::string line;

    while (std::getline(mapsFile, line)) {
        if (line.find("liblua") != std::string::npos || line.find(".lua") != std::string::npos) {
            std::cerr << "[dump detected everything is stored ." << std::endl;
            kill(getpid(), SIGKILL);
        }
    }
}


int yaserClose() {
    JavaVM* java_vm = i_App->activity->vm;
    JNIEnv* java_env = NULL;
    jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;
    jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
    if (jni_return != JNI_OK)
        return -2;
    jclass native_activity_clazz = java_env->GetObjectClass(i_App->activity->clazz);
    if (native_activity_clazz == NULL)
        return -3;
    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "AndroidThunkJava_RestartGame","()V");
    if (method_id == NULL)
        return -4;
    java_env->CallVoidMethod(i_App->activity->clazz, method_id);
    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;
    return 0;
}
bool isyaserFolderHere(const std::string& folderPath) {
    return (access(folderPath.c_str(), F_OK) == 0);
}

void YaserAntiCrack1() {
    std::string folderPath = "/storage/emulated/0/Android/data/com.guoshi.httpcanary"
    "/storage/emulated/0/Android/data/com.sniffer"
    "/storage/emulated/0/Android/data/com.guoshi.httpcanary.premium"
    "/storage/emulated/0/Android/data/com.httpcanary.pro"
    "/storage/emulated/0/Android/data/com.minhui.networkcapture"
    "/storage/emulated/0/Android/data/app.greyshirts.sslcapture"
    "/storage/emulated/0/Android/data/com.emanuelef.remote_capture"
    "/storage/emulated/0/Android/data/com.minhui.networkcapture.pro";
    if (isyaserFolderHere(folderPath)) {
        yaserClose();
    } else {
        
    }
}

bool IsVPNEnabled() {
    JNIEnv *env;
    g_vm->AttachCurrentThread(&env, 0);
    jclass ctx = env->FindClass("android/content/Context");
    jobject context = getJNIContext(env);
    jmethodID service = env->GetMethodID(ctx, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring str = env->NewStringUTF("connectivity");
    jobject conn_service = env->CallObjectMethod(context, service, str);
    env->DeleteLocalRef(str);
    jclass connectivity = env->FindClass("android/net/ConnectivityManager");
    jclass capabils = env->FindClass("android/net/NetworkCapabilities");
    jmethodID has1 = env->GetMethodID(capabils, "hasCapability", "(I)Z");
    jmethodID has = env->GetMethodID(capabils, "hasTransport", "(I)Z");
    jmethodID getCapabil = env->GetMethodID(connectivity, "getNetworkCapabilities", "(Landroid/net/Network;)Landroid/net/NetworkCapabilities;");
    jmethodID getActive = env->GetMethodID(connectivity, "getActiveNetwork", "()Landroid/net/Network;");
    jobject activenetwork = env->CallObjectMethod(conn_service, getActive);
    jobject activecapabilities = env->CallObjectMethod(conn_service, getCapabil, activenetwork);
    jboolean hasvpn1 = env->CallBooleanMethod(activecapabilities, has, 4);
    jboolean hasvpn2 = env->CallBooleanMethod(activecapabilities, has1, 4);
    if (hasvpn1 || hasvpn2) {
        env->DeleteLocalRef(activenetwork);
        env->DeleteLocalRef(activecapabilities);
        env->DeleteLocalRef(conn_service);
        env->DeleteLocalRef(ctx);
        env->DeleteLocalRef(context);
        env->DeleteLocalRef(capabils);
        env->DeleteLocalRef(connectivity);
        return true;
    } else {
        env->DeleteLocalRef(activenetwork);
        env->DeleteLocalRef(activecapabilities);
        env->DeleteLocalRef(conn_service);
        env->DeleteLocalRef(ctx);
        env->DeleteLocalRef(context);
        env->DeleteLocalRef(capabils);
        env->DeleteLocalRef(connectivity);
        return false;
    }
}


void eglSwapBuffers_handler(RegisterContext * ctx, const HookEntryInfo * info)
{
    static std::atomic<int> count{0};
    static std::atomic_flag rendering = ATOMIC_FLAG_INIT;
    if (count.fetch_add(1, std::memory_order_relaxed) < 10 ||
        eglGetCurrentContext() == EGL_NO_CONTEXT ||
        rendering.test_and_set(std::memory_order_acquire)) return;
    try {
        Render();
    } catch (const std::exception& error) {
        LOGE("C++ exception in render callback: %s", error.what());
    } catch (...) {
        LOGE("Unknown C++ exception in render callback");
    }
    rendering.clear(std::memory_order_release);
}

void *bypassMemcmp(void *arg) {
    void *handle = dlopen("libc.so", RTLD_LAZY);
    if (!handle) return NULL;

    void *memcmpAddr = dlsym(handle, "memcmp");
    if (!memcmpAddr) return NULL;

    uint8_t patch[] = { 0x01, 0x00, 0xA0, 0xE3 }; // MOV R0, #1 (Always return success)
    mprotect((void *)((uintptr_t)memcmpAddr & ~0xFFF), 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(memcmpAddr, patch, sizeof(patch));
    return NULL;
}

void hideLibrary() {
    //unlink("/proc/self/maps");
    char path[64];
    sprintf(path, "/proc/%d/maps", getpid());
    unlink(path);
}

void *main_thread(void *) {
    LOGI("Overlay worker started; waiting for liblogic.so");
    for (int attempt = 0; attempt < 30 && !m_IL2CPP; ++attempt) {
        m_IL2CPP = Tools::GetBaseAddress("liblogic.so");
        if (!m_IL2CPP) sleep(1);
    }
    if (!m_IL2CPP) {
        LOGE("liblogic.so was not found; overlay initialization skipped");
        return nullptr;
    }
    LOGI("Initialized Logic");
    if (!Il2CppAttach("liblogic.so")) {
        LOGE("Required IL2CPP exports are missing; overlay initialization skipped");
        return nullptr;
    }
    bool assemblies_ready = false;
    for (int attempt = 0; attempt < 30; ++attempt) {
        if (Il2CppIsAssembliesLoaded()) {
            assemblies_ready = true;
            break;
        }
        sleep(1);
    }
    if (!assemblies_ready) {
        LOGE("IL2CPP assemblies were not ready; overlay initialization skipped");
        return nullptr;
    }

    void *swap_buffers = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    if (!swap_buffers || DobbyInstrument(swap_buffers, eglSwapBuffers_handler) != 0) {
        LOGE("eglSwapBuffers instrumentation failed; overlay initialization skipped");
    } else {
        LOGI("eglSwapBuffers instrumentation installed");
    }
    return nullptr;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);

static void* OpenOriginalLibrary(const char* path) {
    LOGI("Opening original library at %s", path);
    dlerror();
    void* handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        const char* error = dlerror();
        LOGE("Original library open failed at %s: %s", path, error ? error : "unknown error");
    } else {
        LOGI("Original library opened at %s", path);
    }
    return handle;
}

static bool CopyOriginalLibrary(const char* source, const char* destination) {
    if (CopyFile(source, destination)) return true;
    LOGE("Original library copy failed from %s to %s: errno=%d",
         source, destination, errno);
    return false;
}

static void* FindOriginalEntry(void* handle, const char* name) {
    dlerror();
    void* entry = dlsym(handle, name);
    const char* error = dlerror();
    if (!entry || error) LOGE("Original library %s lookup failed: %s", name, error ? error : "missing symbol");
    return error ? nullptr : entry;
}

static int InvokeOriginalEntry(void* entry, void* vm, void* reserved) {
    auto onLoad = reinterpret_cast<jint (*)(JavaVM*, void*)>(entry);
    LOGI("Calling original JNI_OnLoad");
    const auto started = std::chrono::steady_clock::now();
    const int version = onLoad(static_cast<JavaVM*>(vm), reserved);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started).count();
    LOGI("Original JNI_OnLoad completed in %lld ms", static_cast<long long>(elapsed));
    return version;
}

JNIEXPORT jint JNICALL Call_JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    if (!vm || vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK || !env) {
        LOGE("JNI environment is unavailable during library initialization");
        return JNI_ERR;
    }
    std::string apkPkg = getPackageName(env);
    if (apkPkg.empty()) {
        if (env->ExceptionCheck()) {
            LOGE("Java exception while obtaining host package name");
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        LOGE("Unable to determine host package name");
        return JNI_ERR;
    }
    startup::Paths paths;
    if (!startup::BuildPaths(apkPkg, ARCH, paths)) {
        LOGE("Invalid package or ABI while locating original library");
        return JNI_ERR;
    }
    LOGI("Original library source: %s", paths.source.c_str());
    LOGI("Original library copy: %s", paths.copied.c_str());
    const std::string copyDirectory = paths.copied.substr(0, paths.copied.find_last_of('/'));
    if (mkdir(copyDirectory.c_str(), 0700) != 0 && errno != EEXIST) {
        LOGE("Could not create original-library directory %s: errno=%d",
             copyDirectory.c_str(), errno);
    }
    const startup::Operations ops{CopyOriginalLibrary, OpenOriginalLibrary, FindOriginalEntry,
                                  InvokeOriginalEntry};
    const auto result = startup::ForwardOriginal(paths, vm, reserved,
                                                 reinterpret_cast<void*>(&JNI_OnLoad), ops);
    if (!result.copySucceeded) LOGE("Original library copy failed; stale destination skipped");
    if (result.status != startup::Status::Success) {
        LOGE("Original JNI forwarding failed: %s", startup::StatusName(result.status));
        return JNI_ERR;
    }
    LOGI("Original JNI_OnLoad returned 0x%x (copied=%d)", result.jniVersion,
         result.usedCopy ? 1 : 0);
    return result.jniVersion;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    if (!vm) return JNI_ERR;
    LOGI("Replacement JNI_OnLoad entered, ABI=%s", ARCH);
    g_vm = vm;
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jint result;
    try {
        result = Call_JNI_OnLoad(vm, reserved);
    } catch (const std::exception& error) {
        LOGE("C++ exception during JNI_OnLoad: %s", error.what());
        return JNI_ERR;
    } catch (...) {
        LOGE("Unknown C++ exception during JNI_OnLoad");
        return JNI_ERR;
    }
    if (result == JNI_ERR) return JNI_ERR;
    pthread_t t;
    const int threadResult = pthread_create(&t, nullptr, main_thread, nullptr);
    if (threadResult == 0) {
        pthread_detach(t);
        LOGI("Overlay worker thread created");
    } else {
        LOGE("Overlay worker thread could not start: pthread_create=%d", threadResult);
    }
    return result;
}
