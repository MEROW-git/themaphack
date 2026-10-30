# Android ARM64 build and validation

The native target is `libAkSoundEngine2.so`. It is not an installable APK. The independent `diagnostic-app` APK tests its own harmless JNI library and saves a display preference; it neither loads the game-dependent library nor modifies Mobile Legends.

The optional camera presets and their test limits are described in [LAYLA_VIEW.md](LAYLA_VIEW.md).

## Requirements and Termux path

**VERIFIED:** This checkout builds on Windows with Android NDK 29.0.13846066 for `arm64-v8a`, Android API 26 and C++17. **UNVERIFIED:** Running `ndk-build` natively on an ARM64 Termux phone. Google's official NDK distributions do not list an Android ARM64 development host; an x86-64 Linux NDK cannot execute directly on ARM64. An independently sourced ARM64-host-compatible NDK with `ndk-build` is required for the on-device path. [NDK downloads](https://developer.android.com/ndk/downloads), [ndk-build guide](https://developer.android.com/ndk/guides/ndk-build).

Keep a Termux checkout in its private home directory, where scripts can execute:

```sh
pkg update
pkg install git coreutils binutils file
cd "$HOME"
git clone https://github.com/MEROW-git/themaphack.git
cd themaphack
uname -m
```

The clone command fetches the public repository. If the audited changes have not been pushed, use this branch/checkout instead of assuming the public clone contains them. After installing a compatible on-device NDK:

```sh
export ANDROID_NDK_HOME=/absolute/path/to/arm64-host-ndk
"$ANDROID_NDK_HOME/ndk-build" --version
sh ./compile.sh -j2
file artifacts/libs/arm64-v8a/libAkSoundEngine2.so
readelf -h artifacts/libs/arm64-v8a/libAkSoundEngine2.so
(cd artifacts/libs/arm64-v8a && sha256sum -c libAkSoundEngine2.so.sha256)
```

`compile.sh` locates `ndk-build` through `ANDROID_NDK_HOME`, `ANDROID_NDK_ROOT`, `PATH`, or the legacy `~/ndk-aide/ndk-build` location, forces ARM64, writes `artifacts/logs/ndk-build-arm64.log`, and checks that the library exists. `jni/Android.mk` lists all linked sources and the checked-in ARM64 Dobby archive. `jni/Application.mk` sets ABI, API level, static C++ runtime and release optimization. A missing host toolchain is an environment blocker, not a C++ compiler result.

## Reproduce the desktop cross-build

On a supported desktop host, install NDK 29.0.13846066. From PowerShell in the repository root:

```powershell
& "$env:LOCALAPPDATA\Android\Sdk\ndk\29.0.13846066\ndk-build.cmd" -C . NDK_OUT=artifacts/obj NDK_LIBS_OUT=artifacts/libs APP_ABI=arm64-v8a -B -j4
```

**VERIFIED:** The current Layla View build succeeded on Windows. Full log: `artifacts/logs/ndk-build-arm64-layla-view.log`. The ELF header reports `ELF64`, `AArch64`, shared object; its dynamic symbol table exports `JNI_OnLoad`. SHA-256: `ce30d8210cb66ef1dd10a3162b58b3183ea45c44d73f8e082d234c0f1d2c67f2`. Artifact: `artifacts/libs/arm64-v8a/libAkSoundEngine2.so` (5,733,144 bytes). The earlier audit log remains at `artifacts/logs/ndk-build-arm64-audit.log`.

## Build the separate diagnostic APK

On a supported desktop Android SDK host, install JDK 17, SDK platform 36, NDK 29.0.13846066, CMake 4.3.1 and Ninja 1.13.2. The Gradle wrapper is pinned to Gradle 8.14.6 and Android Gradle Plugin 8.13.2. From `diagnostic-app/`:

```sh
./gradlew :app:assembleDebug
```

On Windows use `.\gradlew.bat :app:assembleDebug`. The APK is `diagnostic-app/app/build/outputs/apk/debug/app-debug.apk`; a copy is `artifacts/themaphack-diagnostics-debug.apk`. The diagnostic CMake module compiles C++17 into `lib/arm64-v8a/libthemaphack_diagnostics.so`. It reports its ABI and pointer size through JNI. A checkbox saves whether those details are shown in this app's own preferences. It has no requested Android permissions.

**VERIFIED:** The APK assembled on Windows; full log: `artifacts/logs/diagnostic-apk-gradle-audit.log`. APK inspection found the ARM64 diagnostic `.so`; `aapt dump permissions` found no requested permissions; `apksigner verify` reported a valid v2 debug signature. SHA-256: `50a8f9e7928f2c4561caf032f5c670a0b96f80e8932a57dbe44265b760ca27f8` (362,545 bytes). **UNVERIFIED:** APK launch and JNI execution on a device. The game-dependent `libAkSoundEngine2.so` is intentionally not packaged or loaded by this app because its `JNI_OnLoad` expects the separate original `libTMH.bytes` and game context.

The first attempt to use Gradle's `ndkBuild` integration failed on this Windows checkout because its absolute path contains a space. The diagnostic module now uses CMake; its successful build requires Ninja on `PATH` or in an SDK CMake installation. The audit build used CMake 4.3.1 and Ninja 1.13.2. The delivered Gradle log is from the final successful build.

## Troubleshooting and limits

| Status | Symptom | Action |
| --- | --- | --- |
| BROKEN | `ndk-build not found` in Termux | Supply an ARM64-host-compatible NDK/`ndk-build` and set `ANDROID_NDK_HOME`, or use a supported desktop host. |
| BROKEN | Missing `jni/Tools/Dobby/libraries/arm64-v8a/libdobby.a` | Restore the checked-in archive. No reproducible source recipe for it is present. |
| BROKEN | Gradle says it cannot find Ninja | Install Ninja 1.13.2 or a compatible SDK CMake/Ninja package and ensure Gradle can find it. |
| UNVERIFIED | Native library loads poorly, overlay missing, or game crashes | Inspect logcat for `JNI_OnLoad`, `libTMH.bytes`, `liblogic.so`, IL2CPP and EGL failures. Validate against legally obtained exact-version game binaries/metadata in a permitted test environment; do not infer compatibility from the build. |

Generated outputs and logs under `artifacts/` are ignored by Git and remain in this workspace. `artifacts/SHA256SUMS` records the two delivered artifact hashes. No device runtime or anti-cheat behavior was tested.
