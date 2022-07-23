#include "ui.hpp"

void UI::drag_vec3(const char *label, Vec3 *value, f32 speed) {
    ImGui::DragFloat3(label, &value->x, speed, -10000, 10000);
}

void UI::drag_vec3_rotation(const char *label, Vec3 *value) {
    f32 rotation[3] = {0};
    rotation[0] = SEMATH_RAD2DEG(value->x);
    rotation[1] = SEMATH_RAD2DEG(value->y);
    rotation[2] = SEMATH_RAD2DEG(value->z);
    ImGui::DragFloat3(label, rotation, 10, 0, 360);
    value->x = SEMATH_DEG2RAD(rotation[0]);
    value->y = SEMATH_DEG2RAD(rotation[1]);
    value->z = SEMATH_DEG2RAD(rotation[2]);
}

void UI::show_vec3(const char *label, Vec3 value) {
    ImGui::InputFloat3(label, &value.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
}

void UI::window_begin(const char *label, Rect rect) {
    ImGui::Begin(label, (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImVec2 window_pos = ImVec2(rect.x, rect.y);
    ImVec2 window_size = ImVec2(rect.w, rect.h);
    ImGui::SetWindowPos(window_pos);
    ImGui::SetWindowSize(window_size);
}

void UI::window_end() {
    ImGui::End();
}

Rect UI::dock_space_left() {
    Vec2i screen;
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &screen.x, &screen.y);
    Rect percentages = {
        0, 0.1f, 0.2f, 0.8f
    };
    Rect result = {
        screen.x * percentages.x,
        screen.y * percentages.y,
        screen.x * percentages.w,
        screen.y * percentages.h
    };
    return result;
}
