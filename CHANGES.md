# 2.2.16 build-audit changes

This records the earlier audit commit. For the subsequent loading-freeze investigation and current artifact, see [LOADING_FREEZE_REPORT.md](LOADING_FREEZE_REPORT.md).

**Result: BUILD SUCCESSFUL — GAME COMPATIBILITY UNVERIFIED.**

## Source changes

| Files | Change |
|---|---|
| `compile.sh`, `jni/Android.mk`, `jni/Application.mk` | Check the required ARM64 Dobby archive before building, record preflight failures in the build log, reject non-ARM64 ABI targets, and document API 26. |
| `jni/Main.cpp`, `jni/Config/JNIStuff.h` | Validate the JNI environment and package lookup, avoid creating an empty copied library when the source is absent, and log copy failure before trying the original library path. |
| `jni/Tools/Il2Cpp.cpp` | Guard domain/image iteration and optional IL2CPP calls, require the exports used by lookup helpers, guard static-field base pointers, and resolve `il2cpp_string_new_utf16` by its correct export name. |
| `jni/XYZ/GameClass.h`, `jni/XYZ/SDK.h` | Check missing methods and field-offset sentinels in shared wrappers, initialize a static-field pointer, and correct wrapper return types that were inconsistent with their use. Exact game signatures remain unverified. |
| `jni/Config/TouchSystem.h` | Check Unity touch methods before calling them and remove ignored ARM64 `__fastcall` annotations in these wrappers. |
| `jni/DrawMenu.h` | Reject truncated legacy settings files before applying any loaded values. The hardcoded package-specific storage path remains unverified. |
| `jni/Render.h` | Disable unverified game ESP and host screen-resolution mutation in the default build. |
| `jni/XYZ/DrawESP.h`, `jni/XYZ/DrawMinimap.h`, `jni/XYZ/RInfo.h` | Initialize local pointers passed to static-field lookup in dormant or optional paths. |
| `tools/audit_il2cpp_lookups.py`, `UNVERIFIED_GAME_INTERFACES.md` | Generate a source-located inventory of 203 unique literal IL2CPP lookups. |
| `.gitignore`, `README.md`, `COMPATIBILITY_REPORT.md`, `COMPATIBILITY_2.2.16.md`, `CHANGES.md`, `TERMUX_BUILD.md` | Keep generated Python cache out of Git, point readers to the current audit, and record revision comparison, build evidence, exact-version limitations and reproducible commands. |

## Validation

The complete verbose build output is `artifacts/logs/ndk-build-arm64-2.2.16-full.log`. The linked result is `artifacts/libs/arm64-v8a/libAkSoundEngine2.so`. ELF and SHA-256 details are in [COMPATIBILITY_2.2.16.md](COMPATIBILITY_2.2.16.md) and [TERMUX_BUILD.md](TERMUX_BUILD.md). The compiler emitted warnings in bundled code and legacy headers; no compile or link errors remain in this build. A device/runtime compatibility test was unavailable.

Static validation passed: `python tools/audit_il2cpp_lookups.py`, `python -m py_compile tools/audit_il2cpp_lookups.py`, `bash -n compile.sh`, `git diff --check`, and `clang++ --target=aarch64-none-linux-android26 -std=c++17 -fsyntax-only tests/camera_view_state_test.cpp` using the NDK compiler. A prior host-only run of the unchanged camera state class passed (`artifacts/logs/camera-view-state-test.log`). The artifacts directory is ignored by Git and remains in this workspace.
