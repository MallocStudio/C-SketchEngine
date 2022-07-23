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
