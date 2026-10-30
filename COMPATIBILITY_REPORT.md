# Android and game compatibility report

This report records source inspection and a Windows cross-build for Android ARM64. **VERIFIED** means observed in source/build output, **UNVERIFIED** means no exact-version game/device test, **BROKEN** means a concrete defect, and **NOT IMPLEMENTED** means no active integration. Build success does not establish game compatibility.

## Build and packaging

| Status | Finding |
| --- | --- |
| VERIFIED | `compile.sh`, `jni/Android.mk` and `jni/Application.mk` target `arm64-v8a`, Android API 26, C++17, and the checked-in ARM64 Dobby archive. NDK 29.0.13846066 built `artifacts/libs/arm64-v8a/libAkSoundEngine2.so`; ELF inspection showed an AArch64 shared object and exported `JNI_OnLoad`. Full output: `artifacts/logs/ndk-build-arm64-audit.log`. |
| VERIFIED | The repository's native target is a `.so`, not a game APK. The separate `diagnostic-app` is an Android application with its own JNI diagnostic library, a saved display preference, ARM64 packaging, no requested permissions, and no game access. Its APK build succeeded on Windows. See `TERMUX_BUILD.md`. |
| UNVERIFIED | The Windows-built `.so` has not been run on an Android device or in Termux. There is no verified on-device ARM64-host `ndk-build` installation in this environment. The official NDK downloads list Windows, macOS and x86-64 Linux development hosts, not native Android ARM64. [Official NDK downloads](https://developer.android.com/ndk/downloads). |
| UNVERIFIED | The bundled `jni/Tools/Dobby/libraries/arm64-v8a/libdobby.a` has no pinned source revision or reproducible archive-build recipe in this repository. Other bundled components include ImGui, xDL, KittyMemory, Substrate and xHook. Legacy cURL/OpenSSL/login and GlossHook trees are not linked by `jni/Android.mk`. Their provenance and redistribution rights need separate review. |

## Version-dependent assumptions and failure points

| Status | Source | Assumption or risk | Exact data needed |
| --- | --- | --- | --- |
| UNVERIFIED | `jni/Main.cpp:279-319` | `JNI_OnLoad` expects the host package, a `dragon2017/assets/comlibs/<ABI>/libTMH.bytes` file under its external app data, and a writable `/data/user/0/<package>/app_libs/` destination. `CopyFile` failure is not checked before two `dlopen` attempts. Android storage policy and linker namespace can reject those paths. | Legally obtained package tree, original library hash/ABI, actual load order, device logcat and path access on the target Android version. |
| UNVERIFIED | `jni/Main.cpp:244-275`, `jni/Tools/Il2Cpp.cpp:270+` | The overlay waits up to 30 seconds each for `liblogic.so` and IL2CPP assemblies, then instruments `eglSwapBuffers`. A renamed library, missing exports, changed initialization timing or different renderer prevents startup. `Il2CppAttach` checks only a subset of resolved function pointers. | Exact native library inventory/export table and permitted runtime load/renderer trace. |
| BROKEN | `jni/Tools/Il2Cpp.cpp:58-72`, `jni/Tools/Il2Cpp.cpp:223-241` | The image scan assumes a valid domain/assembly list when called; a missing field returns `size_t(-1)`. Callers that add that sentinel to an object pointer can access invalid memory. | Guard each feature path against missing resolution, then validate field names/layout for the exact build. |
| BROKEN | `jni/XYZ/SDK.h:4-50` | Entity field accessors directly dereference `this + field offset` without a null or `size_t(-1)` check. Method wrappers call resolver results as function pointers without null checks. Two wrappers cast integer return types through `uintptr_t`, which is ABI-sensitive. | Exact metadata, signatures, return types and null-state behavior for the exact version. |
| BROKEN | `jni/XYZ/DrawESP.h:147-151,194-215` | One write uses `showEntity + offset` pointer arithmetic on a `ShowEntity*`, scaling by object size rather than bytes. Other reads depend on live list layout and field offsets. Static-field guards cover only two lookups. | Verified object/list layouts and field offsets, plus controlled runtime tests. |
| BROKEN | `jni/Render.h:162,184-186`, `jni/XYZ/GameClass.h:6-27` | `Screen` method wrappers call returned addresses without verifying they are nonzero. Missing Unity methods can crash on the render thread. | Exact Unity assembly/type/method names, arity and calling conventions. |
| BROKEN | `jni/XYZ/GameClass.h:626-635` | `ACInterface` resolver macros use `Assebly-CSharp.dll`, inconsistent with `Assembly-CSharp.dll` elsewhere. These macros are not actively hooked, but their name would fail if the target assembly uses the latter spelling. | Exact-version metadata; do not infer a replacement from a different release. |
| UNVERIFIED | `jni/XYZ/GameClass.h`, `jni/XYZ/Minimap.h`, `jni/XYZ/DrawESP.h` | Numerous gameplay fields, class names, methods, arities, hardcoded values and object layouts may have changed. The compiler warns that `__fastcall` annotations are ignored on ARM64. | Legally obtained metadata and headers from the exact game release, then ABI review and permitted device testing. |
| UNVERIFIED | `jni/DrawMenu.h:50-58`, `jni/XYZ/IconMinimap/DrawIcon.h:98-147` | Settings use `/storage/emulated/0/Android/data/com.mobile.legends/tmh.ini`, and icon loading also hardcodes `com.mobile.legends`. Package variants and scoped storage may prevent access. | Package ID, resource layout, and storage behavior on the target device. |

The original game binaries and IL2CPP metadata are absent. No current Mobile Legends release was independently validated, and no new addresses, offsets, or functions are supplied here. Anti-cheat and DNS findings are in [ANTIBAN_AUDIT.md](ANTIBAN_AUDIT.md) and [DNS_AUDIT.md](DNS_AUDIT.md).

## Remaining build warnings

`artifacts/logs/ndk-build-arm64-audit.log` records ignored ARM64 calling-convention attributes, deprecated C++ conversion facilities, format-string type mismatches, Unicode whitespace, and warnings in bundled hook code. The linker succeeded; these warnings remain runtime/maintenance risks. They were not silenced by changing compiler flags.
