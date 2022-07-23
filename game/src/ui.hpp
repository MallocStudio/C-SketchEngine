#pragma once
#include "sketchengine.h"
#include "imgui.h"

namespace UI {
        // widgets
    void drag_vec3(const char *label, Vec3 *value, f32 speed = 1, f32 min = -1000, f32 max = 1000);
    void show_vec3(const char *label, Vec3 value);
    void drag_vec3_rotation(const char *label, Vec3 *value);
    void rgb(const char *label, RGB *rgb);
    void rgba(const char *label, RGBA *rgb);

        // assets
    void material(SE_Material *material);

        // windowing
    bool window_begin(const char *label, Rect rect);
    void window_end();

        // docking
    Rect dock_space_left();
    Rect dock_space_right();
    Rect dock_space_top();
    Rect dock_space_bottom();
}