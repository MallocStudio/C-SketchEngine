#include "ui.h"
#include "renderer.h"
#include "input.h"

///
void ui_update_context(UI_Context *context) {
    context->mouse_pos = get_mouse_pos(&context->is_mouse_left_pressed, &context->is_mouse_right_pressed);
    if (context->is_mouse_busy) {
        // if we release the mouse button, the mouse is no longer considererd busy
        if (context->is_mouse_right_pressed == false) context->is_mouse_busy = false;
    }
}

///
void ui_init_theme (UI_Theme *theme) {
    theme->color_base     = (RGBA){1.f, 0.2f, 0.2f, 1.f};
    theme->color_pressed  = (RGBA){0.5f, 0.2f, 0.2f, 1.f};
    theme->color_selected = (RGBA){1.f, 0.4f, 0.4f, 1.f};
    theme->color_disabled = (RGBA){0.4f, 0.4f, 0.4f, 1.f};
    theme->color_panel_base = (RGBA){0.4f, 0.4f, 0.4f, 1.f};
    theme->color_transition_amount = 0.025f;
}

/// render a rearrangable rect
void ui_render_floating_rect(UI_Context *context, Rect *rect) {
    bool is_pressed = context->is_mouse_left_pressed;
    Vec2i mouse_pos = context->mouse_pos;
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
    render_rect_color(context->renderer->sdl_renderer, *rect, (RGBA) {1, 0, 0, 1});
    set_render_draw_color_rgba(context->renderer->sdl_renderer, (RGBA) {1, 0, 0, 1});
    render_circle(context->renderer->sdl_renderer, move_button.x, move_button.y, button_radius);
    render_circle(context->renderer->sdl_renderer, resize_button.x, resize_button.y, button_radius);
    reset_render_draw_color(context->renderer->sdl_renderer);
}

/// draw a simple button. returns true if the button is pressed
bool ui_draw_quick_button(UI_Context *ui_context, Rect rect, const char *text, UI_Theme *theme) {
    RGBA color = theme->color_base;
    bool result = false;
    
    UI_STATES button_state = UI_STATES_NORMAL;
    { // -- get button state
        if (ui_context->is_mouse_busy == false) {
            Vec2i mouse_pos = ui_context->mouse_pos;
            if (SDL_PointInRect(&mouse_pos, &rect) == true) {
                button_state = UI_STATES_SELECTED;
                if (ui_context->is_mouse_left_pressed) {
                    button_state = UI_STATES_PRESSED;
                }
            }
        }
    }

    switch (button_state) {
        case UI_STATES_NORMAL: {
            color = theme->color_base;
        } break;
        case UI_STATES_SELECTED: {
            color = theme->color_selected;
        } break;
        case UI_STATES_PRESSED: {
            color = theme->color_pressed;
            result = true;
        } break;
        case UI_STATES_DISABLED: {
            color = theme->color_disabled;
        } break;
    }

    // -- base
    render_rect_filled_color(ui_context->renderer->sdl_renderer, rect, color);
    // -- text
    if (text != NULL) {
        render_string(ui_context->renderer, text, rect, true);
    }
}

/// render a panel for other widgets
bool ui_begin_horizontal(UI_Context *ctx, const char *title, Rect rect) {
    bool result = true; // return true by default
    ctx->current_rect = rect;
    ctx->layout = UI_LAYOUT_HORIZONTAL;
    ctx->current_rect.w = 32;

    // -- render background
    render_rect_filled_color(ctx->renderer->sdl_renderer, rect, ctx->theme->color_panel_base);
    // -- render title
    ui_label(ctx, title);
    return result;
}
bool ui_begin_vertical(UI_Context *ctx, const char *title, Rect rect) {
    bool result = true; // return true by default
    ctx->current_rect = rect;
    ctx->layout = UI_LAYOUT_VERTICAL;
    ctx->current_rect.h = 32;

    // -- render background
    render_rect_filled_color(ctx->renderer->sdl_renderer, rect, ctx->theme->color_panel_base);
    // -- render title
    ui_label(ctx, title);
    return result;
}

/// render a button using context
bool ui_button(UI_Context *ctx, const char *string) {
    bool result = false;
    Rect rect = ctx->current_rect;
    ui_advance_context(ctx, rect);

    RGBA color = ctx->theme->color_base;
    
    UI_STATES button_state = UI_STATES_NORMAL;
    { // -- get button state
        if (ctx->is_mouse_busy == false) {
            Vec2i mouse_pos = ctx->mouse_pos;
            if (SDL_PointInRect(&mouse_pos, &rect) == true) {
                button_state = UI_STATES_SELECTED;
                if (ctx->is_mouse_left_pressed) {
                    button_state = UI_STATES_PRESSED;
                }
            }
        }
    }

    switch (button_state) {
        case UI_STATES_NORMAL: {
            color = ctx->theme->color_base;
        } break;
        case UI_STATES_SELECTED: {
            color = ctx->theme->color_selected;
        } break;
        case UI_STATES_PRESSED: {
            color = ctx->theme->color_pressed;
            result = true;
        } break;
        case UI_STATES_DISABLED: {
            color = ctx->theme->color_disabled;
        } break;
    }

    // -- base
    render_rect_filled_color(ctx->renderer->sdl_renderer, rect, color);
    // -- text
    if (string != NULL) {
        render_string(ctx->renderer, string, rect, true);
    }

    return result;
}

/// render a label using context.
void ui_label(UI_Context *ctx, const char *title) {
    Rect rect = ctx->current_rect;
    ui_advance_context(ctx, rect);

    render_string(ctx->renderer, title, rect, true);
}

/// Advance the UI_Context.current_rect attributes based on the latest widget rect
void ui_advance_context(UI_Context *ctx, Rect rect) {
    if (ctx->layout == UI_LAYOUT_VERTICAL) {
        ctx->current_rect.y += rect.h;
    } else
    if (ctx->layout == UI_LAYOUT_HORIZONTAL) {
        ctx->current_rect.x += rect.w;
    }
}