#include "ui.h"
#include "renderer.h"
#include "input.h"

///
void ui_init_button (UI_Button *button, Text *text, UI_Theme *theme) {
    button->color_current = theme->color_base;
    button->color_target  = button->color_current;
    button->rect          = (Rect){0, 0, 128, 48};
    button->text = text; // @incomplete regenerate text's color based on theme
}

///
bool ui_render_button(SDL_Renderer *sdl_renderer, UI_Button *button, UI_Theme *theme) {
    bool result = false;
    // -- get the state of button
    UI_STATES state = ui_get_button_state(button);
    // -- update button based on state
    switch (state) {
        case UI_STATES_NORMAL: {
            button->color_target = theme->color_base;
        } break;
        case UI_STATES_SELECTED: {
            button->color_target = theme->color_selected;
        } break;
        case UI_STATES_PRESSED: {
            button->color_target = theme->color_pressed;
            result = true;
        } break;
        case UI_STATES_DISABLED: {
            button->color_target = theme->color_disabled;
        } break;
    }

    // -- lerp button color towards target color
    lerp_rgba(&button->color_current, button->color_target, delta_time * theme->color_transition_amount);
    
    // -- draw the base based on state
    RGBA color = button->color_current;
    render_rect_filled_color(sdl_renderer, button->rect, color);

    // -- draw text
    render_text_at_rect_clipped(sdl_renderer, button->text, button->rect);
    return result;
}

///
UI_STATES ui_get_button_state(UI_Button *button) {
    UI_STATES result = UI_STATES_NORMAL;
    Vec2i mouse_pos;
    u32 mouse_state = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    if (SDL_PointInRect(&mouse_pos, &button->rect) == true) {
        result = UI_STATES_SELECTED;
        if (mouse_state & SDL_BUTTON_LMASK) {
            result = UI_STATES_PRESSED;
        }
    }
    return result;
}

///
void ui_init_theme (UI_Theme *theme) {
    theme->color_base     = (RGBA){1.f, 0.2f, 0.2f, 1.f};
    theme->color_pressed  = (RGBA){0.5f, 0.2f, 0.2f, 1.f};
    theme->color_selected = (RGBA){1.f, 0.4f, 0.4f, 1.f};
    theme->color_disabled = (RGBA){0.4f, 0.4f, 0.4f, 1.f};
    theme->color_transition_amount = 0.025f;
}

/// render a rearrangable rect
void ui_render_floating_rect(Renderer *renderer, Rect *rect) {
    bool is_pressed;
    Vec2i mouse_pos = get_mouse_pos(&is_pressed, NULL);
    f32 button_radius = 16.f;
    Vec2i move_button = {rect->x, rect->y};
    Vec2i resize_button = {rect->x + rect->w, rect->y + rect->h};
    bool is_move = false, is_resize = false;
    
    if (is_pressed) {
        if (point_in_circle(mouse_pos, move_button, button_radius)) {
            is_move = true;
        }
        if (point_in_circle(mouse_pos, resize_button, button_radius)) {
            is_resize = true;
        }
    }

    if (is_move) {
        rect->x = mouse_pos.x;
        rect->y = mouse_pos.y;
    }
    if (is_resize) {
        rect->w = mouse_pos.x - rect->x;
        rect->h = mouse_pos.y - rect->y;
        rect->w = SDL_clamp(rect->w, button_radius, 9999999);
        rect->h = SDL_clamp(rect->h, button_radius, 9999999);
    }
    render_rect_color(renderer->sdl_renderer, *rect, (RGBA) {1, 0, 0, 1});
    set_render_draw_color_rgba(renderer->sdl_renderer, (RGBA) {1, 0, 0, 1});
    render_circle_filled(renderer->sdl_renderer, move_button.x, move_button.y, button_radius);
    render_circle_filled(renderer->sdl_renderer, resize_button.x, resize_button.y, button_radius);
    reset_render_draw_color(renderer->sdl_renderer);
}