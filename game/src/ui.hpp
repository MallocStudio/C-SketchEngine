#pragma once
#include "sketchengine.h"
#include "imgui.h"

namespace UI {
    void drag_vec3(const char *label, Vec3 *value, f32 speed = 1);
    void show_vec3(const char *label, Vec3 value);
    void drag_vec3_rotation(const char *label, Vec3 *value);
    void window_begin(const char *label, Rect rect);
    void window_end();

    Rect dock_space_left();
    // vec2 dock_space_right();
}