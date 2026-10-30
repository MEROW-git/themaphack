# Anti-cheat and anti-analysis audit

Scope: source inspection of this checkout and an ARM64 build on Windows with NDK 29.0.13846066. No game process, account, network trace, or anti-cheat service was tested. Status labels describe the cited evidence: **VERIFIED** (checked by source/build inspection), **UNVERIFIED** (requires runtime or exact-version data), **BROKEN** (source contains a concrete defect), and **NOT IMPLEMENTED** (present as legacy text or declarations but absent from active registration). None of these labels means ban-safe.

## Registration and load path

| Status | Evidence | Finding |
| --- | --- | --- |
| VERIFIED | `jni/Android.mk`, `jni/Main.cpp:1-108`, `jni/Main.cpp:244-275` | The build compiles `Main.cpp`, `Tools/Il2Cpp.cpp`, render and hook libraries. `Main.cpp` does not include `XYZ/Bypass.h` or `XYZ/Protection.h`. After waiting for `liblogic.so` and assemblies, the sole active registration in startup is `DobbyInstrument` on `eglSwapBuffers` to call `Render()`. This is an overlay hook, not an anti-cheat hook. |
| NOT IMPLEMENTED | `jni/Render.h:204-207`, `jni/Tools/Tools.cpp:52-57` | Gameplay `Tools::Hook` calls are commented out. `Tools::Hook`, `newHook`, and `XYZ::Hook` are helper definitions, but no active call registers the anti-cheat wrappers. |
| VERIFIED | ARM64 ELF dynamic symbol and byte inspection | The built library exports `JNI_OnLoad`. No `Dns_Get*`, `iAntiCheatReporter*`, or `iACInterface*` dynamic symbols were found, and the legacy DNS hostname is absent from the binary. Hidden/local symbols cannot be excluded from the symbol check alone; the source registration trace establishes that the legacy wrappers are not compiled. |
| UNVERIFIED | `jni/Main.cpp:279-319` | `JNI_OnLoad` chains to a separate `libTMH.bytes` and starts a worker. The required original file, package layout and device behavior were unavailable. |

## Legacy anti-cheat functions

All functions below are **NOT IMPLEMENTED in the active build** because their defining headers are not included and there is no registration call. The stated intended effects are inferred from their bodies, not confirmed against any game release.

| Status if used | Definitions | Intended effect and defect |
| --- | --- | --- |
| BROKEN | `jni/XYZ/Bypass.h:1-38` `iAntiCheatReporter_StartBattle`, `OnReleaseUseSkill`, `OnTryUseSkill`, `OnTryUseSkill2` | Suppress battle/skill reporting. `StartBattle` has unreachable original call; `OnTryUseSkill` declares a `void*` return although its original is `void`, and falls through without returning; the others call potentially null originals on some paths. Variadic original pointer for `OnReleaseUseSkill` avoids signature checking. |
| BROKEN | `jni/XYZ/Bypass.h:40-85` `iTestCase_gp_start_anti_cheat`, `iAntiCheatReporter_OnRequestSkillMsg`, `HasSkillInfo` | Stall or suppress anti-cheat start and skill messages. `sleep_for(hours::max())` can effectively hang a thread; `HasSkillInfo` never returns its declared `bool`; `OnRequestSkillMsg` has unreachable original call and drops the null-instance path. |
| BROKEN | `jni/XYZ/Bypass.h:47-96` `iDeviceUtil_GetRefreshRate`, `GetIsRoot`, `OpenURL`, `iAPKSignature_IsSignatureSame`, `iBattleBridge_OnSignReport` | Spoof refresh rate, root/signature result, alter URL behavior, and suppress a report. `GetRefreshRate` recursively calls itself on its fallback path. `GetIsRoot` never reaches the original. Signature wrapper adds an unverified extra parameter; report wrapper's return type differs from the original and lacks a return on one path. `OpenURL` redirects to a hardcoded third-party site when its flag is set. |
| BROKEN | `jni/XYZ/Bypass.h:99-143` `iBattleBridge_SafeSendGameReportData`, `SendGameReportData`, `SendLargeGameReportData`, `iAntiCheatReporter_ctor`, `cctor`, `iCheat_Start`, `Apply`, `iLuaHelper_SaveActivityClientInfo` | Suppress reports or initialization. The three send wrappers call themselves recursively when `thiz` is null, causing stack exhaustion. Several wrappers return `void*` for originals declared `void`; original signatures are variadic or unknown. |
| BROKEN | `jni/XYZ/Bypass.h:148-195` `iACInterface_GetTimeOfDay`, `GetSystemTime`, `LoadCert`, `SignErrorVerify`, `GetSign`, `CheckHook`, `iGameReportMgr_Report_client_match_box`, `Report_MemoryHack`, `iSleepingFunctionHook` | Return zero instead of checking time, certificates, signatures, hooks, or submitting reports. Return types and argument lists are guessed/variadic; use without exact ABI verification is unsafe. |
| BROKEN | `jni/XYZ/Bypass.h:266-297` `DisposeServerObject`, `ReportShowEntityReportData`, `set_DisableUndeclaredEntityCheck`, `iSetAntiCheatReport` | Drop disposal, overwrite a report field, change XML validation input, or empty a report string. `ReportShowEntityReportData` writes at a hardcoded `+0x48` with no layout proof; original pointers may be null. The meaning of the XML setting is not established. |
| BROKEN | `jni/XYZ/Protection.h:1-25` duplicate `iAntiCheatReporter_StartBattle`, `OnReleaseUseSkill`, `OnTryUseSkill`, `OnTryUseSkill2` | Stall reporting with huge sleeps or a permanent loop. This header is not included; including it alongside `Bypass.h` would also redefine names. Its copied `OnReleaseUseSkill` loop contains malformed encoded whitespace in this checkout. |
| UNVERIFIED | `jni/XYZ/GameClass.h:582-655` | Macros and functions name anti-cheat/report targets, but do not hook them. The names, namespaces, arities and signatures are historical assumptions. The `ACInterface` entries spell the assembly `Assebly-CSharp.dll`, unlike the `Assembly-CSharp.dll` used elsewhere, so resolution would fail unless such an assembly actually existed. No exact-version metadata was available. |

`Bypass.h` also defines seven DNS wrappers; see [DNS_AUDIT.md](DNS_AUDIT.md). It does not provide evidence that modifying or suppressing these functions prevents server-side detection.

## Compiled but unused anti-analysis code

| Status | Code | Finding |
| --- | --- | --- |
| BROKEN | `jni/Main.cpp:110-117` `PatchOffset` | Calculates an unusual writable range starting one page before the address, without checking `length` against that range. It is not called. |
| BROKEN | `jni/Main.cpp:119-132` `detectDump` | Would kill the process if `/proc/self/maps` contains `liblua` or `.lua`, with no proof that either indicates dumping. It is not called. |
| BROKEN | `jni/Main.cpp:135-171` `yaserClose`, `YaserAntiCrack1` | The activity pointer may be null; consecutive path literals concatenate into a single impossible directory path. The call in `Render.h:210` is commented out. |
| BROKEN | `jni/Main.cpp:173-207` `IsVPNEnabled` | Uses raw `4` for both `hasTransport` and `hasCapability`; the latter is `NET_CAPABILITY_IMS`, not a VPN capability. JNI objects/calls are not fully checked and the attached thread is not detached. Its render call is commented out. [Android NetworkCapabilities](https://developer.android.com/reference/android/net/NetworkCapabilities). |
| BROKEN | `jni/Main.cpp:224-242` `bypassMemcmp`, `hideLibrary` | The former writes a 32-bit ARM instruction into `libc.so` `memcmp` even on ARM64, and ignores `mprotect` failure. The latter attempts to unlink `/proc/<pid>/maps`, a procfs pseudo-file. Neither is called. |

No anti-cheat evasion was added or enabled in this audit. **Ban risk and detection behavior remain UNVERIFIED.**
