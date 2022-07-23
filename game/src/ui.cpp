#include "ui.hpp"

void UI::drag_vec3(const char *label, Vec3 *value, f32 speed, f32 min, f32 max) {
    ImGui::DragFloat3(label, &value->x, speed, min, max);
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

void UI::rgb(const char *label, RGB *rgb) {
    f32 colour[3] = {
        (f32) (rgb->r / 255.0f),
        (f32) (rgb->g / 255.0f),
        (f32) (rgb->b / 255.0f)
    };
    ImGui::ColorPicker3(label, colour);
    rgb->r = (ubyte) (colour[0] * 255);
    rgb->g = (ubyte) (colour[1] * 255);
    rgb->b = (ubyte) (colour[2] * 255);
}

void UI::rgba(const char *label, RGBA *rgb) {
    f32 colour[4] = {
        (f32) (rgb->r / 255.0f),
        (f32) (rgb->g / 255.0f),
        (f32) (rgb->b / 255.0f),
        (f32) (rgb->a / 255.0f)
    };
    ImGui::ColorPicker4(label, colour);
    rgb->r = (ubyte) (colour[0] * 255);
    rgb->g = (ubyte) (colour[1] * 255);
    rgb->b = (ubyte) (colour[2] * 255);
    rgb->a = (ubyte) (colour[3] * 255);
}

///
///     WINDOWING
///

bool UI::window_begin(const char *label, Rect rect) {
    ImVec2 window_pos = ImVec2(rect.x, rect.y);
    ImVec2 window_size = ImVec2(rect.w, rect.h);
    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowPos(window_pos);
    return ImGui::Begin(label, (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
}

void UI::window_end() {
    ImGui::End();
}

// coords from top left
#define center_x 0.15f
#define center_y 0.05f
// #define center_y_px 32.0f
#define center_w (1 - (center_x * 2)) // allow center_x on each side
// #define center_h (1 - (center_y * 2))
#define center_h 0.6f

Rect UI::dock_space_left() {
    Vec2i screen;
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &screen.x, &screen.y);
    Rect percentages = {
        // 0, 0.1f, 0.2f, 0.8f
        0, center_y, center_x, center_h * 0.75f
    };
    Rect result = {
        screen.x * percentages.x,
        screen.y * percentages.y,
        screen.x * percentages.w,
        screen.y * percentages.h
    };
    return result;
}

Rect UI::dock_space_right() {
    Vec2i screen;
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &screen.x, &screen.y);
    Rect percentages = {
        // 0.2f, 0.1f, 0.2f, 0.8f
        center_x + center_w, center_y, center_x, center_h * 0.75f // we'll run into issue when trying to resize dynamically
    };
    Rect result = {
        screen.x * percentages.x,
        screen.y * percentages.y,
        screen.x * percentages.w,
        screen.y * percentages.h
    };
    return result;
}

Rect UI::dock_space_top() {
    Vec2i screen;
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &screen.x, &screen.y);
    Rect result = {
        0,
        0,
        (f32)screen.x,
        32
    };
    return result;
}

Rect UI::dock_space_bottom() {
    Vec2i screen;
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &screen.x, &screen.y);
    Rect percentages = {
        // 0, 0.9f, 1, 0.1f
        0, center_h, 1, 1 - center_h
    };
    Rect result = {
        screen.x * percentages.x,
        screen.y * percentages.y,
        screen.x * percentages.w,
        screen.y * percentages.h
    };
    return result;
}

void UI::material(SE_Material *material) {
    ImVec4 header_colour = ImVec4(1, 0.2, 0.2, 1);
        //- Type
    ImGui::TextColored(header_colour, "Type:");
    ImGui::SameLine();
    switch (material->type) {
        case SE_MATERIAL_TYPE_LIT: {
            ImGui::Text("Lit");
        } break;
        case SE_MATERIAL_TYPE_TRANSPARENT: {
            ImGui::Text("Transparent");
        } break;
        case SE_MATERIAL_TYPE_CUSTOM: {
            ImGui::Text("Custom");
        } break;
    }

        //- Shader Index
    ImGui::TextColored(header_colour, "Shader Index");
    ImGui::SameLine();
    ImGui::Text("%i", material->shader_index);

        //- Base Diffuse
    ImGui::ColorEdit4("Diffuse", &material->base_diffuse.x);

        //- Textures
    if (material->texture_diffuse.loaded) {
        ImGui::TextColored(header_colour, "Diffuse Texture");
        ImGui::SameLine();
        ImGui::Text("%i", material->texture_diffuse.id);
    }
    if (material->texture_specular.loaded) {
        ImGui::TextColored(header_colour, "Specular Texture");
        ImGui::SameLine();
        ImGui::Text("%i", material->texture_specular.id);
    }
    if (material->texture_normal.loaded) {
        ImGui::TextColored(header_colour, "Normal Texture");
        ImGui::SameLine();
        ImGui::Text("%i", material->texture_normal.id);
    }
}