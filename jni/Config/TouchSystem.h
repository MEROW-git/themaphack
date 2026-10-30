#pragma once

enum TouchPhase : int {
	Began = 0,
	Moved = 1,
	Stationary = 2,
	Ended = 3,
	Canceled = 4,
};

struct UnityEngine_Touch_Fields {
    int32_t m_FingerId;
    Vector2 m_Position;
    Vector2 m_RawPosition;
    Vector2 m_PositionDelta;
    float m_TimeDelta;
    int32_t m_TapCount;
    int32_t m_Phase;
    int32_t m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_RadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};
/*
#define Input_GetTouch (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "GetTouch", 1)
#define Input_get_touchCount (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_touchCount")
#define Input_get_mousePosition (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_mousePosition")
*/
uintptr_t Input_GetTouch(){
	return (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "GetTouch", 1);
}

uintptr_t Input_get_touchCount(){
	return (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_touchCount");
}

uintptr_t Input_get_mousePosition(){
	return (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_mousePosition");
}

int get_touchCount() {
	auto fn = reinterpret_cast<int (*)()>(Input_get_touchCount());
	return fn ? fn() : 0;
}

Vector3 get_mousePosition(void *instance) {
	auto fn = reinterpret_cast<Vector3 (*)(void *)>(Input_get_mousePosition());
	return fn ? fn(instance) : Vector3();
}

void ImGui_GetTouch(ImGuiIO* io, int screenHeight) {
    if (!io) return;
    auto getTouch = reinterpret_cast<UnityEngine_Touch_Fields (*)(void *, int)>(Input_GetTouch());
    if (getTouch && get_touchCount() > 0) {
        UnityEngine_Touch_Fields touch = getTouch(nullptr, 0);
        switch (touch.m_Phase) {
        case TouchPhase::Began:
        case TouchPhase::Stationary:
            io->MouseDown[0] = true;
            ImGui::GetIO().MousePos = ImVec2(get_mousePosition((void *)0).x, screenHeight - get_mousePosition((void *)0).y);
            break;
        case TouchPhase::Ended:
        case TouchPhase::Canceled:
            io->MouseDown[0] = false;
            clearMousePos = true;
            break;
        case TouchPhase::Moved:
            ImGui::GetIO().MousePos = ImVec2(get_mousePosition((void *)0).x, screenHeight - get_mousePosition((void *)0).y);
            break;
        default:
            break;
        }
    }
}

