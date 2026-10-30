# Themaphack build and compatibility notes

This repository builds an Android ARM64 native library, **not an installable game APK**. The separate diagnostic-app project builds an APK that loads only its own diagnostic JNI library, reports device/build details, and saves its own display preference. It does not load the game-dependent library or inspect or modify the installed game.

Detailed findings: [anti-cheat audit](ANTIBAN_AUDIT.md), [DNS audit](DNS_AUDIT.md), [compatibility report](COMPATIBILITY_REPORT.md), and [Termux/build guide](TERMUX_BUILD.md). The current audit build logs are under `artifacts/logs/` in this workspace.

## Verified build

The source was compiled with Android NDK 29.0.13846066 on Windows, targeting arm64-v8a and Android API 26. The resulting libAkSoundEngine2.so is an ELF64 AArch64 shared object with an exported JNI_OnLoad. The audit build log is artifacts/logs/ndk-build-arm64-audit.log in this workspace. The standalone diagnostic APK was built with JDK 17, Gradle 8.14.6, Android Gradle Plugin 8.13.2, compile SDK 36, CMake 4.3.1, and Ninja 1.13.2; its audit log is artifacts/logs/diagnostic-apk-gradle-audit.log.

These checks establish that the files build and are correctly packaged. **No Android device or current Mobile Legends installation was available for runtime testing.** The library's compatibility with any current game release is unverified.

The build still uses the checked-in Dobby static archive at jni/Tools/Dobby/libraries/arm64-v8a/libdobby.a (SHA-256: b0128fe5e30b5401859c389703e0515c499e0af5db9d1e4b5e8face672d61cdc). Its ELF members are AArch64, but this repository does not identify the archive's source revision or provide a recipe to rebuild it. Other bundled dependencies include ImGui, xDL, KittyMemory, Substrate, xHook, and old login/cURL/OpenSSL trees. The login/cURL/OpenSSL archives and GlossHook are not linked by the current Android.mk. The repository has no top-level license file; check bundled component provenance and redistribution rights before publishing binaries.

## Build the native library

### Android ARM64 with Termux

Keep the checkout in Termux's private home directory so scripts can execute:

~~~sh
pkg update
pkg install git coreutils binutils file
cd "$HOME"
git clone https://github.com/MEROW-git/themaphack.git
cd themaphack
uname -m
command -v ndk-build
~~~

The last two commands should report aarch64 and a working ndk-build executable. **The official Google NDK downloads do not provide an Android ARM64 host toolchain**; the Linux NDK download is for an x86-64 development host. Termux's ndk-sysroot or clang package alone does not supply a Google-compatible ndk-build installation. On-device building therefore requires a separately obtained, ARM64-host-compatible NDK/ndk-build toolchain. This repository does not bundle or validate one. If you only have the official NDK, build on a supported desktop host instead. See the [NDK downloads](https://developer.android.com/ndk/downloads), [ndk-build guide](https://developer.android.com/ndk/guides/ndk-build), and [Termux build environment notes](https://github.com/termux/termux-packages/wiki/Build-environment).

With a compatible on-device NDK installed:

~~~sh
export ANDROID_NDK_HOME=/absolute/path/to/compatible/ndk
sh ./compile.sh -j2
file artifacts/libs/arm64-v8a/libAkSoundEngine2.so
readelf -h artifacts/libs/arm64-v8a/libAkSoundEngine2.so
(cd artifacts/libs/arm64-v8a && sha256sum -c libAkSoundEngine2.so.sha256)
~~~

The script finds ndk-build through ANDROID_NDK_HOME, ANDROID_NDK_ROOT, PATH, or the legacy ~/ndk-aide/ndk-build path. It passes the correct project directory, forces arm64-v8a, saves a full log at artifacts/logs/ndk-build-arm64.log, and creates the library under artifacts/libs/arm64-v8a/. A missing or incompatible host toolchain is an environment blocker, not a C++ build result.

### Supported desktop host

Install NDK 29.0.13846066 using the Android SDK manager, then run compile.sh from a Unix shell with ANDROID_NDK_HOME set. On Windows PowerShell, use the NDK's .cmd launcher directly:

~~~powershell
& "$env:LOCALAPPDATA\Android\Sdk\ndk\29.0.13846066\ndk-build.cmd" -C . NDK_OUT=artifacts/obj NDK_LIBS_OUT=artifacts/libs APP_ABI=arm64-v8a
~~~

The native build's active sources and dependencies are listed in jni/Android.mk; jni/Application.mk pins the ABI, platform, static C++ runtime, and release optimization. The [Android NDK documentation](https://developer.android.com/ndk/guides/application_mk) describes these settings. Build output under artifacts/ is ignored by Git.

## Build the standalone diagnostic APK

On a supported Android SDK host with JDK 17 and SDK platform 36:

~~~sh
cd diagnostic-app
sh ./gradlew :app:assembleDebug
~~~

On Windows use .\gradlew.bat :app:assembleDebug. The result is diagnostic-app/app/build/outputs/apk/debug/app-debug.apk. It is a separately packaged and debug-signed APK. Its manifest requests no permissions; it contains only its own ARM64 native diagnostic library, not the game-dependent library. The Gradle wrapper pins Gradle 8.14.6 and its distribution SHA-256; AGP is pinned to 8.13.2. CMake 4.3.1 and Ninja 1.13.2 are needed for the JNI module. The [AGP 8.13 compatibility table](https://developer.android.com/build/releases/agp-8-13-0-release-notes) specifies JDK 17 and Gradle 8.13 or newer.

The APK build was verified on Windows. The Android SDK build tools bundled for desktop hosts are not a verified on-device Termux APK build path.

## Current game compatibility audit

| Assumption in source | Effect if the game changed | Data needed to check it |
| --- | --- | --- |
| jni/Main.cpp expects a host to load libAkSoundEngine2.so and a separate libTMH.bytes at a package-specific path | JNI_OnLoad returns JNI_ERR if the original library cannot be loaded | A legally obtained, exact-version package inventory and original library, including ABI and hash |
| liblogic.so and specific IL2CPP exports are expected | Overlay initialization is skipped if the library or required exports are absent | Export table and exact native library names from that same version |
| Assembly-CSharp.dll and UnityEngine.CoreModule.dll names, classes, methods, arities, and fields in jni/XYZ/GameClass.h | Resolved addresses can be missing or wrong; most feature paths are not safe to run without validation | Legally obtained IL2CPP metadata and signatures for the exact game build |
| Renderer uses eglSwapBuffers through a bundled Dobby archive | A different renderer or EGL lifecycle can prevent the overlay from appearing | Renderer and library-load trace from a permitted test environment |
| Settings use /storage/emulated/0/Android/data/com.mobile.legends/tmh.ini | Saving/loading may fail due to package changes or scoped-storage access | Tested storage location and permissions on the target Android version |

The startup path now checks for the original library, required IL2CPP exports, assembly readiness, and the EGL symbol before instrumenting. It skips the overlay when these are missing. Two battle static-field lookups now reject the resolver's -1 failure sentinel before dereference. The compiler also reports that __fastcall annotations in jni/XYZ/GameClass.h are ignored for ARM64; those signatures still need exact-version ABI validation. Other game-derived calls and structures still need version-specific validation; a successful build does **not** establish runtime safety.

No game binary, libTMH.bytes, liblogic.so, or IL2CPP metadata is in this repository. There is no basis here to assert compatibility with a current Mobile Legends release or to supply new addresses, offsets, or function signatures. Do not copy values from a different release.

The old anti-cheat and DNS hook routines remain in legacy source files such as jni/XYZ/Bypass.h for repository history, but the active startup path no longer includes or registers them. unlockskins.json is not referenced by the native build.

## Private DNS and privacy

Android Private DNS protects DNS questions and answers when active. It does not hide game traffic, server connections, device identifiers, application changes, or account activity. A selected DNS provider may still receive domain lookups. Changing DNS cannot make a modified client undetectable or establish a ban-risk percentage. See [Android's Private DNS explanation](https://support.google.com/android/answer/9654714).

This project does not set Android's Private DNS setting. Legacy DNS-rewrite functions in jni/XYZ/Bypass.h are not part of the current startup build. The old key-login HTTP code is also not compiled. The earlier ban-prevention claim in libs/notes.txt was removed because the repository supplies no evidence for it.

## Troubleshooting and limits

- **ndk-build not found in Termux:** install an ARM64-host-compatible NDK toolchain or build on a supported desktop host. The official Linux x86-64 NDK cannot execute natively on an ARM64 phone.
- **Missing libdobby.a:** restore the checked-in archive for arm64-v8a. The project has no recorded source build for that dependency.
- **Compiler warnings:** the log intentionally keeps warnings visible. Legacy project and bundled code still report C++ and ABI warnings; review them before relying on runtime behavior.
- **No overlay or failed load:** check Android logs for the explicit JNI_OnLoad, IL2CPP, and EGL messages. Runtime compatibility and original-library availability remain unverified.
- **Need an APK:** build the independent diagnostic app above. The native library itself has no APK packaging or installer, and the diagnostic APK does not load it.

No anti-cheat bypass or ban-prevention guarantee is provided by this modernization.
