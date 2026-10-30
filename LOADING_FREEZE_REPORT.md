# Loading freeze investigation — BlueStacks 5 Pie 64-bit

Target reported by the user: MLBB `2.2.16.1232.1`, Windows 11, BlueStacks 5 Pie 64-bit. This version string differs from the earlier `2.2.16.12322` audit target; no compatibility inference is transferred between them. The user reports that replacing `libAkSoundEngine.bytes` with a renamed build freezes near the end of loading, and restoring the original allows startup. No BlueStacks instance, game package, original `.bytes` file, or runtime log was available in this workspace. The freeze was **not reproduced here**.

## Startup sequence observed in source

1. The Android linker maps the replacement and its `DT_NEEDED` libraries and runs its native static initializers. The unstripped ELF has a 120-byte `.init_array` (15 pointer slots). Their execution on BlueStacks was not observed.
2. If the host invokes `JNI_OnLoad`, it obtains a `JNIEnv`, calls the Android `ActivityThread` path to find the package name, and constructs a `libTMH.bytes` source path under `/sdcard/Android/data/<package>/files/dragon2017/assets/comlibs/arm64-v8a/`.
3. It creates the app-private `app_libs` directory when possible, copies the original library through a temporary file and atomic rename, opens the copied file, finds its `JNI_OnLoad`, and calls it **synchronously**. If the copy fails, it skips any stale destination and tries the source path directly. Every stage now emits `XYOURZONE` logcat messages.
4. Only after the forwarded `JNI_OnLoad` succeeds does it start a detached worker. The worker waits up to about 30 seconds for `liblogic.so`, attaches IL2CPP, waits up to about 30 seconds for assemblies, then instruments `eglSwapBuffers`. The waits occur on the worker, not in the replacement's `JNI_OnLoad`.
5. After ten swap callbacks, the render callback may initialize ImGui, load icon resources, query Unity methods and draw. The callback now requires an EGL context and prevents concurrent `Render()` calls. This path remains untested on the reported game build.

`JNI_OnLoad` is part of the VM's native-library loading contract; a direct native `dlopen` path does not by itself establish that this function ran. The actual host load path must be determined from runtime logs or package code. [Android JNI guidance](https://developer.android.com/ndk/guides/jni-tips).

## Confirmed source and ELF findings

| Status | Finding |
|---|---|
| VERIFIED | The checkout contains no original `libTMH.bytes` or `libAkSoundEngine.bytes`. The replacement expects `libTMH.bytes` at the path above and returns `JNI_ERR` if it cannot open it, resolve `JNI_OnLoad`, or obtain a supported JNI version. If only the renamed replacement was installed as described, this required forwarding input was not supplied by that action. |
| VERIFIED | The previous copy routine wrote directly to the destination, did not create its parent directory and could leave a truncated file. A failed copy was followed by an attempt to load the destination anyway. The new routine creates the directory, copies to a temporary file, checks the byte count, and renames only after success; the forwarder skips a stale destination after failure. |
| VERIFIED | The built library is ELF64 AArch64, type DYN, with `JNI_OnLoad` exported. Its internal SONAME remains `libAkSoundEngine2.so` after a filename rename. Its direct dependencies are `liblog.so`, `libandroid.so`, `libEGL.so`, `libGLESv2.so`, `libGLESv3.so`, `libdl.so`, `libz.so`, `libc.so`, and `libm.so`. The NDK link used `--no-undefined`. |
| VERIFIED | `libTMH.bytes`, `liblogic.so` and `eglSwapBuffers` are looked up at runtime; they are not `DT_NEEDED` dependencies and successful linking cannot validate them. The replacement does not export a known original audio-engine API; only the original file can establish what symbols the game expects. |
| VERIFIED | The unstripped `Main.o` has a C++ global-initialization function and globals such as `std::map` containers and icon/config state. Source inspection found no file copy, worker creation, `dlopen`, or EGL instrumentation in those global definitions; the ELF check alone cannot rule out a static-initializer problem on BlueStacks. |
| VERIFIED | `bypassMemcmp`, `hideLibrary`, `detectDump` and `YaserAntiCrack1` exist in source but have no active call site in this startup path. They were not enabled or changed in this investigation. The commented `YaserAntiCrack1` call in `Render.h` remains commented. |
| VERIFIED | The standalone forwarding harness passes eight forwarding scenarios on Windows, including copy failure, fallback, missing entry, self-forwarding and rejected/unsupported JNI versions. Log: `artifacts/logs/startup-forwarder-host.log`. It does not execute the game library. |

Other source risks remain: dormant `PatchOffset` and hook callbacks perform raw page writes; optional ESP paths contain unchecked game-object layouts and a pointer-arithmetic error noted in [COMPATIBILITY_2.2.16.md](COMPATIBILITY_2.2.16.md). They were not active in the default rendering build. The default renderer still calls Unity touch/camera methods and loads icon assets after EGL instrumentation; null guards do not validate the target game's method signatures or object layouts. A freeze after the `eglSwapBuffers instrumentation installed` marker needs an authorized render-thread trace before changing those interfaces.

## Likely explanations requiring runtime evidence

| Status | Cause to check | Distinguishing evidence |
|---|---|---|
| SUSPECTED | The required `libTMH.bytes` is absent, inaccessible, not ARM64, missing `JNI_OnLoad`, or cannot be copied/opened. This is the first source-supported explanation given the reported single-file replacement. | `XYOURZONE` copy/open/lookup failure, `JNI_ERR`, or linker error in logcat. The exact original file and its ABI/hash are needed. |
| SUSPECTED | The replacement is not a drop-in substitute for the original `libAkSoundEngine.bytes`. Renaming a file changes neither its SONAME nor exports. Original audio/native callers may require symbols other than `JNI_OnLoad`. | Compare export tables and `DT_NEEDED`/SONAME of the legally obtained original and replacement; inspect `dlopen`/`dlsym` or `UnsatisfiedLinkError` messages. |
| SUSPECTED | The game process uses x86-64 or another ABI despite the Pie 64-bit instance supporting ARM64 translation. BlueStacks lets users choose ABI libraries, so “Pie 64-bit” alone does not prove the selected app ABI. [BlueStacks ABI settings](https://support.bluestacks.com/hc/en-us/articles/360058929011-What-is-Application-Binary-Interface-ABI-in-BlueStacks-5). | Instance ABI settings, `ro.product.cpu.abilist`, package `primaryCpuAbi`, and linker error such as `wrong ELF class` or `unexpected e_machine`. |
| SUSPECTED | The `ActivityThread`/`getApplication` lookup is unavailable at this load stage or blocked as a non-SDK interface on Android Pie. Android's restrictions apply to JNI access to non-SDK interfaces. [Android non-SDK restrictions](https://developer.android.com/guide/app-compatibility/restrictions-non-sdk-interfaces). | `XYOURZONE` package-name failure plus a Java exception or hidden-API message in logcat. Do not change Android's API enforcement to diagnose this. |
| SUSPECTED | The synchronous `dlopen` or forwarded original `JNI_OnLoad` stalls; alternatively the later EGL instrumentation or render callback stalls near loading completion. | Last emitted stage marker: before/after `Opening original library`, `Calling original JNI_OnLoad`, `Overlay worker started`, and `eglSwapBuffers instrumentation installed`. A native stack or ANR trace may be needed. |
| UNVERIFIED | An original library with the same SONAME may alias or recursively load the replacement, or the hardcoded app-data paths may differ in this package/Android profile. The self-entry guard catches the same function address but cannot prove two distinct mappings contain different code. | Original ELF, app-private directory layout and runtime linker trace. |

## Build and independent tests

Windows NDK 29.0.13846066 completed a full verbose `arm64-v8a`/API 26/C++17 build. Artifact: `artifacts/libs/arm64-v8a/libAkSoundEngine2.so` (5,710,968 bytes), SHA-256 `b8454cfe384bbaabf2c90753d3ae74c45489fa0f69f0a3f9e13f866a2953eaa4`. Full log: `artifacts/logs/ndk-build-arm64-loading-debug-full.log`. The host harness was built with Visual Studio C++ Build Tools using `tests/run_startup_forwarder_windows.bat`. Both builds passed; the NDK log still contains warnings in legacy/vendor code. No BlueStacks runtime test was possible (`adb devices` showed no attached device).

## Runtime diagnostics on the reported BlueStacks instance

Keep the original library available and collect logs in an authorized test installation. With Android platform-tools on Windows PowerShell:

```powershell
$adb = "$env:LOCALAPPDATA\Android\Sdk\platform-tools\adb.exe"
& $adb devices
& $adb shell getprop ro.build.version.sdk
& $adb shell getprop ro.product.cpu.abilist
& $adb shell dumpsys package com.mobile.legends | findstr /i "primaryCpuAbi secondaryCpuAbi nativeLibraryDir"
& $adb logcat -c
# Start the game manually and wait until the loading screen stops progressing.
& $adb logcat -d -v threadtime > loading-freeze-logcat.txt
Select-String -Path loading-freeze-logcat.txt -Pattern 'XYOURZONE|libTMH|libAkSoundEngine|linker|dlopen|JNI_OnLoad|UnsatisfiedLinkError|FATAL EXCEPTION|signal 11|ANR|eglSwapBuffers'
```

If `adb devices` is empty, enable BlueStacks Android Debug Bridge access for the selected instance and connect using its documented instance address. A shell permission error when listing app data is not proof that the app itself cannot access the file. Preserve the unfiltered log for timestamps and native stack traces. The new markers identify the last completed stage; they cannot by themselves prove game-version compatibility.

This investigation did not add anti-cheat evasion, change integrity reporting, or claim the replacement is safe to use with the reported MLBB release.
