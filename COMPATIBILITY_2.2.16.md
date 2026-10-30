# MLBB 2.2.16.12322 / 2.2.16a compatibility audit

**Result: BUILD SUCCESSFUL — GAME COMPATIBILITY UNVERIFIED.** This is a source and cross-build audit, not a game runtime test. Status labels below mean **VERIFIED** by this checkout or build output, **UNVERIFIED** without exact-version runtime evidence, **BROKEN** for a demonstrated defect, and **NOT IMPLEMENTED** when no active integration exists.

## Revisions and scope

| Status | Item | Evidence |
|---|---|---|
| VERIFIED | Latest fetched `MEROW-git/themaphack` `main` | `d80076e4076309e52f0f4fa66503617692d70aeb`, committed 2026-10-30 21:27:33 +07:00, “Remove login gate and build Android libraries”. Fetched with `git fetch origin main`. |
| VERIFIED | Named ancestor `anggorodhanumurti/themaphack` `main` | `327b9ccad7ca36a76aeff09e78799df17a611eef`, committed 2026-04-30 07:23:10 +07:00. It is an ancestor of the fetched `origin/main`. |
| VERIFIED | Working branch before this audit | `android-arm64-modernization` at `72bd4e72d52cbfce5ec4734a1e270ce6aaab4ff4`, three commits ahead of `origin/main`. Original history remains in Git. |
| UNVERIFIED | Exact target release | No MLBB 2.2.16.12322 APK, `liblogic.so`, IL2CPP metadata, or authorized device session was provided. No project-owned source identifies this build or patch. |

Relative to the named ancestor, fetched `origin/main` changed native build and login files, removed the original README and LICENSE, and added prebuilt libraries. The modernization branch adds documentation, a separate diagnostic APK and the Layla View menu. A successful build of any of these revisions does not establish compatibility with the target game version.

## Build and packaging

| Status | Finding |
|---|---|
| VERIFIED | NDK 29.0.13846066 on Windows cross-compiled the project for `arm64-v8a`, API 26 and C++17. `artifacts/libs/arm64-v8a/libAkSoundEngine2.so` is ELF64 AArch64, type DYN, with exported `JNI_OnLoad`; size 5,705,432 bytes; SHA-256 `7a6533825870609ff62683aceb1ae08157071d84ee67c9255403535bf11b7e94`. Complete verbose log: `artifacts/logs/ndk-build-arm64-2.2.16-full.log` (122,498 bytes). |
| VERIFIED | Linked inputs include ImGui, xDL, fake_dlfcn, Il2Cpp, KittyMemory, Substrate, xHook and the checked-in ARM64 Dobby static archive. Legacy cURL/OpenSSL/login archives are present but absent from `jni/Android.mk` link inputs. The Dobby archive lacks a pinned source/build recipe. |
| VERIFIED | The native target is a `.so`; `diagnostic-app/` is a separate, game-independent Android APK project from the prior modernization work. It is not an isolated Unity camera demonstration. |
| UNVERIFIED | Native Termux `ndk-build` on an Android ARM64 host was not run. A host-compatible NDK is required; the desktop Windows NDK used here cannot run as a phone-native ARM64 tool. See [official NDK downloads](https://developer.android.com/ndk/downloads). |

## Interface and runtime findings

| Status | Component | Finding and required evidence |
|---|---|---|
| UNVERIFIED | `liblogic.so` and IL2CPP exports | `Main.cpp` waits for `liblogic.so`, attaches IL2CPP, waits for assemblies, then instruments `eglSwapBuffers`. Exact library name, exports, initialization order and renderer on 2.2.16.12322 require authorized binary inventory and runtime traces. The loader now declines to initialize when required exports are missing. |
| UNVERIFIED | 203 literal IL2CPP lookups | The generated [interface inventory](UNVERIFIED_GAME_INTERFACES.md) lists assembly, namespace, class, member, arity when supplied, and source lines. This includes dormant declarations; no entry is marked compatible. Compare each against legally obtained exact-version metadata and inspect overload signatures, return types, static/instance status and generated native ABI. |
| UNVERIFIED | `MethodInfo` and `FieldInfo` layouts | `Il2CppGetMethodOffset` treats the first word of a method descriptor as a code pointer. Static-field resolution reads `FieldInfo::parent->static_fields`. These layouts and the project's dictionary/list templates must be checked against the exact IL2CPP runtime. Null checks cannot prove a nonnull address is correct. |
| VERIFIED | Fail-safe lookup boundaries | The image scan checks domain/list/image pointers; startup checks a fuller set of IL2CPP exports; common Unity, player and field wrappers return safe defaults for unresolved methods or `size_t(-1)` field offsets. Uninitialized static-field pointer locals in dormant paths were initialized. |
| VERIFIED | Game rendering disabled by default | `NewDrawESP` and host `Screen::SetResolution` calls are compiled out unless `TMH_ENABLE_UNVERIFIED_GAME_RENDERING` is explicitly defined. This prevents the most direct known invalid-offset path in the default build. This is a build-time safety gate, not a compatibility validation or detection bypass. |
| UNVERIFIED | Remaining optional rendering paths | If deliberately compiled in, `NewDrawESP` still assumes live game object layouts, unverified fields, list lengths, nonzero HP maxima and function signatures. The old `showEntity + offset` pointer arithmetic and several dormant callbacks also remain unsafe. Do not enable on the target until audited. |
| UNVERIFIED | Touch and display integration | `TouchSystem.h` assumes Unity `Input.GetTouch` returns the bundled `UnityEngine_Touch_Fields` layout and looks in `UnityEngine.dll`; method calls now check missing pointers. Runtime struct layout, touch coordinates and screen DPI still require device testing. |
| UNVERIFIED | Original library forwarding | `JNI_OnLoad` expects an external `libTMH.bytes` and a particular `dragon2017` directory tree; its existence, ABI, Android storage access and `JNI_OnLoad` export are unknown. Copy failure is now logged, then the original path is attempted. |
| BROKEN | Source-only typos/ABI assumptions | `GameClass.h` has dormant `Assebly-CSharp.dll` references and several historical `__fastcall` annotations. Neither was treated as evidence of a valid current-game interface. Anti-cheat related declarations were not changed. |

## Camera review

**VERIFIED:** The current modernization branch has a “Layla View” menu toggle and Normal, Layla Style and Extra Wide presets. The normal default invokes no FOV write; `CameraViewState` keeps a baseline and requests restoration on toggle off. Its host-only state test exercises enable, preset changes, camera changes and restoration. The fetched `origin/main` does not contain this menu option.

**UNVERIFIED:** Layla's actual game camera behavior, `Camera.main`, FOV method signatures, render-thread safety, scene changes, combat, death, respawn, hero changes, UI placement and skill aiming on the target build. The feature modifies local camera FOV only; this audit did not verify any game behavior or server vision effect.

**NOT IMPLEMENTED:** An isolated Unity camera demonstration. To create one, use a separate Unity project with one `Camera`, UI toggle and three preset controls, a script that captures/restores that camera's original FOV, and scene-transition tests. The existing diagnostic APK is a native JNI/Gradle project and contains no Unity runtime.

## What was tested and what remains

**VERIFIED:** NDK cross-build and linker, ELF header, `JNI_OnLoad` export, Python inventory generation, `compile.sh` shell syntax, Git diff check, and an Android-target C++17 syntax check of the camera state test. A prior host-only camera state test passed on the unchanged state class. **UNVERIFIED:** Android load, UI rendering, JNI forwarding, all target game interfaces, Layla View behavior in Unity, multiplayer rules, and Termux-native compilation. No device was connected and no exact-version game binary was supplied. Anti-cheat evasion was not implemented or tested. See [ANTIBAN_AUDIT.md](ANTIBAN_AUDIT.md) and [DNS_AUDIT.md](DNS_AUDIT.md) for earlier source audits.
