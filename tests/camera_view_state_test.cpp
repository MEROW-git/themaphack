#include "../jni/XYZ/CameraViewState.h"

#include <cassert>
#include <cmath>

static void Expect(const std::optional<float> &value, float expected) {
    assert(value.has_value());
    assert(std::fabs(*value - expected) < 0.01f);
}

int main() {
    CameraViewState state;
    constexpr uintptr_t first = 0x1000;
    constexpr uintptr_t second = 0x2000;

    assert(!state.Update(first, 60.0f, false, 1));
    Expect(state.Update(first, 60.0f, true, 1), 68.0f);
    assert(!state.Update(first, 68.0f, true, 1));
    Expect(state.Update(first, 68.0f, true, 2), 76.0f);
    Expect(state.Update(first, 76.0f, false, 2), 60.0f);
    assert(!state.Update(first, 60.0f, false, 2));

    Expect(state.Update(second, 70.0f, true, 2), 86.0f);
    assert(!state.Update(0, 0.0f, true, 2));
    Expect(state.Update(first, 65.0f, true, 1), 73.0f);
    // The game can change FOV during a transition; this becomes the new baseline.
    Expect(state.Update(first, 74.0f, true, 1), 82.0f);
    Expect(state.Update(first, 82.0f, false, 1), 74.0f);

    assert(!state.Update(first, 74.0f, true, 0));
    assert(!state.Update(first, NAN, true, 1));
    Expect(state.Update(first, 74.0f, true, 2), 90.0f);
    Expect(state.Update(first, 90.0f, false, 2), 74.0f);

    Expect(state.Update(second, 90.0f, true, 2), 95.0f);
    Expect(state.Update(second, 95.0f, false, 2), 90.0f);
    assert(!state.Update(first, 100.0f, true, 2)); // never narrow a wider game camera
}
