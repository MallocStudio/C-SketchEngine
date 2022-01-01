#include "ui.h"
#include "renderer.h"

///
void ui_init_button (UI_Button *button) {
    button->current_color = (RGBA){1.f, 0.2f, 0.2f, 1.f};
    button->target_color  = button->current_color;
    button->rect          = (Rect){0, 0, 128, 48};
}

///
bool ui_draw_button(UI_Button *button) {
    // -- get the state of button
    UI_STATES state = ui_get_button_state(button);
    // -- update button based on state
    if (state == UI_STATES_SELECTED) {
        button->target_color = (RGBA){1, 1, 1, 1};
    }
    else button->target_color = (RGBA){1.f, 0, 0, 1.f};

    // -- lerp button color towards target color
    // lerp_rgba(&button->current_color, button->target_color, delta_time * 100); // @debug this turns the g and b value to -nan... something wrong with the lerp function
    printf("color: {%f, %f, %f, %f}\n", button->current_color.r, button->current_color.g, button->current_color.b, button->target_color.a);
    // -- draw the base based on state
    RGBA color = button->current_color;
    render_rect_filled_color(&button->rect, &color);
}

///
UI_STATES ui_get_button_state(UI_Button *button) {
    Vec2i mouse_pos;
    u32 mouse_state = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    if (SDL_PointInRect(&mouse_pos, &button->rect) == true) {
        return UI_STATES_SELECTED;
    }
}