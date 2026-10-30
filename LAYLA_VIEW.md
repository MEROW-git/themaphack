# Layla View

Layla View is an optional camera FOV preset in the existing **Setting** tab. It uses the resolved Unity `Camera.main` `get_fieldOfView` and `set_fieldOfView` methods, which were already named in `jni/XYZ/GameClass.h`. The previous `DroneView()` call in the battle ESP path used a single cached FOV and had no menu control; it has been replaced by the guarded camera state in `jni/XYZ/CameraViewState.h` and `jni/XYZ/LaylaView.h`.

| Toggle / preset | Effect on the current camera |
| --- | --- |
| OFF | Restore the most recent camera baseline when that camera is still live. |
| ON + Normal | Keep the game's FOV. |
| ON + Layla Style | Add 8 degrees to the game's vertical FOV, capped at 95 degrees. |
| ON + Extra Wide | Add 16 degrees, capped at 95 degrees. |

The two wider values are **chosen presets**, not measured Layla camera values. No exact-version Layla camera data is available in this repository, so the name describes the intended visual style rather than a reproduction of her game logic. Unity documents `fieldOfView` as a vertical angle whose horizontal result depends on the viewport aspect ratio. The feature changes no screen resolution, viewport dimensions, touch coordinates, hero controls, skills, attack range, damage, or server vision state. [Unity Camera.fieldOfView](https://docs.unity3d.com/es/current/ScriptReference/Camera-fieldOfView.html).

The controller re-reads `Camera.main` on each render frame. If the camera changes or disappears during a scene transition, it discards the old pointer and captures the new camera's FOV when available. If the game changes FOV while the preset is active, that new value becomes the baseline before the preset is reapplied. OFF restores the latest captured baseline on the same live camera. Unity allows `Camera.main` to be null when no enabled camera carries the MainCamera tag, so the feature skips that frame. [Unity Camera.main](https://docs.unity3d.com/cn/6000.0/ScriptReference/Camera-main.html).

The menu's Save/Load buttons store a six-byte `LVW1` trailer after the old configuration fields; the existing raw `sConfig` layout is unchanged. Older files without the trailer load with Layla View OFF and Normal selected. The legacy `SetFieldOfView` float remains in the file for layout compatibility but is no longer applied by `DroneView()`.

## Validation and limits

- **VERIFIED:** The ARM64 `.so` builds with NDK 29.0.13846066. The ELF is AArch64 and exports `JNI_OnLoad`. Full log: `artifacts/logs/ndk-build-arm64-layla-view.log`.
- **VERIFIED:** A host C++ test covers enable, preset changes, no FOV accumulation, camera replacement, temporary camera loss, a game FOV change, Normal mode, and OFF restoration. Source: `tests/camera_view_state_test.cpp`; log: `artifacts/logs/camera-view-state-test.log`.
- **UNVERIFIED:** On-device rendering, the exact game method signatures, UI camera separation, controls/aiming visuals, walking, combat, death, respawn, hero changes, and final FOV restoration in a live game. The game may use an orthographic camera, override its projection matrix, or change camera objects in ways that make this FOV setting ineffective. No current game binary or device was available.

No anti-cheat evasion was added or tested, and no detection or ban-safety claim is made.
