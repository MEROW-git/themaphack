#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>

// Tracks only the current Unity camera. A changed camera is captured afresh;
// an old camera pointer is never dereferenced after a scene transition.
class CameraViewState {
public:
    std::optional<float> Update(uintptr_t camera, float currentFov, bool enabled, int preset) {
        if (!camera) {
            Reset();
            return std::nullopt;
        }
        if (!std::isfinite(currentFov) || currentFov <= 1.0f || currentFov >= 179.0f) {
            return std::nullopt;
        }

        if (camera != camera_) {
            camera_ = camera;
            originalFov_ = currentFov;
            modified_ = false;
        } else if (modified_ && !Near(currentFov, appliedFov_)) {
            // A game camera update takes precedence over the previous baseline.
            originalFov_ = currentFov;
            modified_ = false;
        }

        if (!enabled || preset <= 0 || preset > 2) {
            const bool restore = modified_ && !Near(currentFov, originalFov_);
            modified_ = false;
            return restore ? std::optional<float>(originalFov_) : std::nullopt;
        }

        const float increase = preset == 1 ? 8.0f : 16.0f;
        const float target = std::max(originalFov_, std::min(originalFov_ + increase, 95.0f));
        if (Near(target, originalFov_)) {
            modified_ = false;
            return std::nullopt;
        }
        appliedFov_ = target;
        modified_ = true;
        return Near(currentFov, target) ? std::nullopt : std::optional<float>(target);
    }

    void Reset() {
        camera_ = 0;
        originalFov_ = 0.0f;
        appliedFov_ = 0.0f;
        modified_ = false;
    }

private:
    static bool Near(float a, float b) { return std::fabs(a - b) < 0.25f; }

    uintptr_t camera_ = 0;
    float originalFov_ = 0.0f;
    float appliedFov_ = 0.0f;
    bool modified_ = false;
};
