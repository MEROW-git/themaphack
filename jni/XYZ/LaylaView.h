#pragma once

#include "CameraViewState.h"

// Called on the existing EGL render thread, after the menu has read input.
// No game class, hero, attack, or server-vision state is changed.
inline void UpdateLaylaView() {
    using MainCamera = void *(*)();
    using GetFov = float (*)(void *);
    using SetFov = void (*)(void *, float);

    static const MainCamera getMain = reinterpret_cast<MainCamera>(Camera_get_main());
    static const GetFov getFov = reinterpret_cast<GetFov>(Camera_get_fieldOfView());
    static const SetFov setFov = reinterpret_cast<SetFov>(Camera_set_fieldOfView());
    static CameraViewState state;

    if (!getMain || !getFov || !setFov) return;
    void *camera = getMain();
    if (!camera) {
        state.Reset();
        return;
    }

    const float currentFov = getFov(camera);
    const auto requestedFov = state.Update(reinterpret_cast<uintptr_t>(camera),
                                           currentFov, LaylaViewEnabled, LaylaViewPreset);
    if (requestedFov) setFov(camera, *requestedFov);
}
