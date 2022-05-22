#include "seui_components.h"

/// -----------------------------------------
///               UTILITIES
/// -----------------------------------------

static Rect apply_margin(Rect rect, Vec2 margin) {
    return (Rect) {
        rect.x + margin.x,
        rect.y + margin.y,
        rect.w - margin.x,
        rect.h - margin.y,
    };
}

u32 generate_ui_id(SE_UI *ctx) {
    ctx->max_id++; // we start with zero. So we increase first.
    u32 id = ctx->max_id;
    return id;
}

/// note that stay_active_on_mouse_leave is used for dragging ui items
UI_STATES get_ui_state (SE_UI *ctx, u32 id, Rect rect, bool stay_active_on_mouse_leave /* = false */) {
    UI_STATES result = UI_STATE_IDLE;
    SE_Input *input = ctx->input;
    bool mouse_down   = input->is_mouse_left_down;
    bool mouse_up     = !mouse_down;
    bool mouse_inside = rect_overlaps_point(rect, input->mouse_screen_pos);

    if (ctx->hot == id) { // pressing down
        if (mouse_up) { // make active
            if (ctx->hot == id) result = UI_STATE_ACTIVE; // mouse up while pressing down
            ctx->hot = SEUI_ID_NULL; // no longer hot
        }
    } else if (ctx->warm == id) { // hover
        if (mouse_down) { // make hot
            input->is_mouse_left_handled = true; // tell input that we've used up this input
            ctx->hot = id;
        }
        // else remain warm
    }

    if (mouse_inside) {
        // if no other item is hot, make us warm
        if (ctx->hot == SEUI_ID_NULL) {
            ctx->warm = id;
        }
    } else if (ctx->warm == id) {
        ctx->warm = SEUI_ID_NULL;
    } else if (ctx->hot == id && !stay_active_on_mouse_leave) {
        ctx->hot = SEUI_ID_NULL;
    }

    // figure out result
    if (result != UI_STATE_ACTIVE) {
        if (ctx->hot == id) {
            result = UI_STATE_HOT;
        } else if (ctx->warm == id) {
            result = UI_STATE_WARM;
        } else {
            result = UI_STATE_IDLE;
        }
    }
    return result;
}


bool seui_button_at(SE_UI *ctx, const char *text, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = ctx->theme.colour_normal;
    RGBA colour_hover   = ctx->theme.colour_hover;
    RGBA colour_pressed = ctx->theme.colour_pressed;
    RGBA colour = colour_normal;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);
    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_normal;
        } break;
        case UI_STATE_WARM: {
            colour = colour_hover;
        } break;
        case UI_STATE_HOT: {
            colour = colour_pressed;
        } break;
   }

    seui_render_rect(renderer, rect, colour);
    se_add_text_rect(&ctx->txt_renderer, text, rect);

    return ui_state == UI_STATE_ACTIVE;
}

bool seui_button_textured_at(SE_UI *ctx, Vec2 texture_index, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = ctx->theme.colour_normal;
    RGBA colour_hover   = ctx->theme.colour_hover;
    RGBA colour_pressed = ctx->theme.colour_pressed;
    RGBA colour = colour_normal;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);

    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_normal;
        } break;
        case UI_STATE_WARM: {
            colour = colour_hover;
        } break;
        case UI_STATE_HOT: {
            colour = colour_pressed;
        } break;
    }

    if (texture_index.x == 0 && texture_index.y == 0) {
        seui_render_rect(renderer, rect, colour);
    } else {
        seui_render_texture(renderer, rect, texture_index, colour);
    }

    return ui_state == UI_STATE_ACTIVE;
}

Vec2 seui_drag_button_textured_at(SE_UI *ctx, Rect rect, Vec2 texture_index, UI_STATES *state) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = ctx->theme.colour_normal;
    RGBA colour_hover   = ctx->theme.colour_hover;
    RGBA colour_pressed = ctx->theme.colour_pressed;
    RGBA colour = colour_normal;

    Vec2 drag = {0};
    UI_STATES ui_state = get_ui_state(ctx, id, rect, true);

    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_normal;
        } break;
        case UI_STATE_WARM: {
            colour = colour_hover;
        } break;
        case UI_STATE_HOT: {
            colour = colour_pressed;
            drag = input->mouse_screen_pos_delta;
        } break;
    }
    if (state != NULL) *state = ui_state;

    if (texture_index.x == 0 && texture_index.y == 0) {
        seui_render_rect(renderer, rect, colour);
    } else {
        seui_render_texture(renderer, rect, texture_index, colour);
    }

    return drag;
}

Vec2 seui_drag_button_at(SE_UI *ctx, Rect rect, UI_STATES *state) {
    return seui_drag_button_textured_at(ctx, rect, UI_ICON_INDEX_NULL, state);
}

void seui_label_at(SE_UI *ctx, const char *text, Rect rect) {
    RGBA colour = RGBA_WHITE;
    ctx->txt_renderer.config_centered = false;

    se_add_text_rect(&ctx->txt_renderer, text, apply_margin(rect, ctx->theme.margin));

    ctx->txt_renderer.config_centered = true;
    seui_render_rect(&ctx->renderer, rect, (RGBA) {10, 10, 10, 100});
}

void seui_slider_at(SE_UI *ctx, Vec2 pos1, Vec2 pos2, f32 *value) {
    // clamp before doing anything
    if (*value < 0) *value = 0;
    if (*value > 1) *value = 1;

    // apply padding to pos
    f32 padding = 16;
    pos1.x += padding;
    pos2.x -= padding;

    Vec2 button_size = {24, 24};
    Vec2 pos_rel1 = vec2_zero();
    Vec2 pos_rel2 = vec2_sub(pos2, pos1);
    Vec2 pos_offset = vec2_mul_scalar(button_size, -0.5f);
    Vec2 button_pos = vec2_add(vec2_add(vec2_mul_scalar(vec2_average(pos_rel1, pos_rel2), *value * 2), pos1), pos_offset);

    /* draw the line */
    seui_render_line(&ctx->renderer, pos1, pos2, 3, RGBA_BLACK);

    /* draw the slider button */
    Rect button_rect = rect_create(button_pos, button_size);

    UI_STATES ui_state = UI_STATE_IDLE;
    Vec2 drag = seui_drag_button_textured_at(ctx, button_rect, UI_ICON_INDEX_SLIDER, &ui_state);

    if (ui_state == UI_STATE_WARM) {
        *value += ctx->input->mouse_wheel * 0.01f;
    }

    // clamp before concluding
    *value += drag.x * 0.01f;
    if (*value < 0) *value = 0;
    if (*value > 1) *value = 1;
}

void seui_slider2d_at(SE_UI *ctx, Vec2 center, f32 radius, Vec2 *value) {
    // normalise before doing anything
    vec2_normalise(value);

    Vec2 button_size = {24, 24};
    Vec2 button_pos = center;

    // value
    button_pos = vec2_add(button_pos, vec2_mul_scalar(*value, radius * 0.5f));

    // size
    button_pos = vec2_sub(button_pos, vec2_mul_scalar(button_size, 0.5f));

    /* draw the border */
    seui_render_circle(&ctx->renderer, center, radius, (RGBA) {0, 0, 0, 50});

    /* draw the button */
    Rect button_rect = rect_create(button_pos, button_size);
    Vec2 drag = seui_drag_button_textured_at(ctx, button_rect, UI_ICON_INDEX_SLIDER, NULL);

    // clamp before concluding
    // *value += drag.x * 0.01f;
    *value = vec2_add(*value, vec2_mul_scalar(drag, 0.05f));
    vec2_normalise(value);
}

/// Draws the Saturation Value Box that's used to pick those values. Updates saturation and value if the user clicks on the box
static void colour_picker_saturation_value_box(SE_UI *ctx, Rect rect, i32 hue, i32 *saturation, i32 *value) {
    Rect cursor = {0};
    cursor.w = 16;
    cursor.h = 16;
    cursor.x = (*saturation) * rect.w;
    cursor.y = (*value)      * rect.h;

    cursor.x += rect.x - cursor.w * 0.5f;
    cursor.y += rect.y - cursor.h * 0.5f;

    u32 id = generate_ui_id(ctx);
    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);

    if (ui_state == UI_STATE_ACTIVE) { // select saturation and value
        Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
        i32 window_w;
        i32 window_h;
        SDL_GetWindowSize(ctx->input->window, &window_w, &window_h);
        mouse_pos.y = window_h - mouse_pos.y;
        mouse_pos.x -= rect.x;
        mouse_pos.y -= rect.y;
        mouse_pos.x /= rect.w;
        mouse_pos.y /= rect.h;

        (*saturation) = (i32)(mouse_pos.x * 100);
        (*value) = (i32)(mouse_pos.y * 100);
        printf("sat : %i\n", *saturation);
        printf("value : %i\n", *value);
        printf("mouse_pos : %f, %f\n", mouse_pos.x, mouse_pos.y);
    }

    seui_render_colour_box(&ctx->renderer, rect, hue);
    seui_drag_button_textured_at(ctx, cursor, UI_ICON_INDEX_CIRCLE_FILLED, NULL);
}

static colour_picker_hue_box(SE_UI *ctx, Rect rect, i32 *hue) {
    // Rect cursor = {0};
    // cursor.w = 16;
    // cursor.h = 16;
    // cursor.x = (*hue) *
    // u32 id = generate_ui_id(ctx);
    // UI_STATES ui_state = get_ui_state(ctx, id, hue_box, false);

    // if (ui_state == UI_STATE_WARM) {
    //     // select hue
    //     if (ctx->input->mouse_wheel != 0) {
    //         hue += ctx->input->mouse_wheel;
    //         if (hue > 359) hue = 0;
    //         if (hue < 0) hue = 359;
    //         printf("hue: %f\n", hue);
    //     }
    // }
    i32 hue_rounded = *hue;
    f32 value = hue_rounded / 360.0f;
    // printf("hue: %f\n", *hue);
    // printf("value: %f\n", value);
    seui_slider_at(ctx, v2f(rect.x, rect.y + rect.h / 2), v2f(rect.x + rect.w, rect.y + rect.h / 2), &value);
    hue_rounded = value * 360;
    (*hue) = hue_rounded;
}

void seui_colour_picker_at(SE_UI *ctx, Rect rect, RGBA *colour) {
#if 1 // the pretty gui way is broken atm
    // colour box
    i32 hue;
    i32 saturation;
    i32 value;
    rgb_to_hsv((RGB) {colour->r, colour->g, colour->b}, &hue, &saturation, &value);

    Rect hue_box = {
        rect.x, rect.y, rect.w, 16
    };
    Rect saturation_value_box = {
        rect.x, rect.y + hue_box.h,
        rect.w, rect.h - hue_box.h
    };
    colour_picker_saturation_value_box(ctx, saturation_value_box, hue, &saturation, &value);
    colour_picker_hue_box(ctx, hue_box, &hue);
    hsv_to_rgba(hue, saturation, value, colour);
#else
    // f32 r,g,b,a;
    // r = colour->r / 255;
    // g = colour->g / 255;
    // b = colour->b / 255;
    // a = colour->a / 255;

    // f32 h = rect.h / 6;
    // f32 y = rect.y + rect.h - h;

    // seui_render_rect(&ctx->renderer, rect, *colour);

    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &r);
    // y -= h;
    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &g);
    // y -= h;
    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &b);
    // y -= h;
    // seui_slider_at(ctx, v2f(rect.x, y), v2f(rect.x + rect.w, y), &a);

    // colour->r = r * 255;
    // colour->g = g * 255;
    // colour->b = b * 255;
    // colour->a = a * 255;
#endif
}

bool seui_selector_at(SE_UI *ctx, Rect rect, i32 *value, i32 min, i32 max) {
    u32 id = generate_ui_id(ctx);

    RGBA colour_bg = (RGBA) {10, 10, 10, 100}; // same colour as label bg
    RGBA colour_highlight = (RGBA) {80, 90, 150, 255};
    Vec3 colour_text = (Vec3) {255, 255, 255};
    RGBA colour = colour_bg;

    f32 button_size = rect.h;
    Rect label = {
        rect.x + button_size,
        rect.y,
        rect.w - button_size * 2,
        rect.h
    };
    UI_STATES ui_state = get_ui_state(ctx, id, label, false);
    switch (ui_state) {
        case UI_STATE_IDLE: {
        } break;
        case UI_STATE_WARM: {
        } break;
        case UI_STATE_ACTIVE: {
        } break;
    }

    /* background */
    seui_render_rect(&ctx->renderer, rect, colour_bg);
    /* left button */
    Rect button = {
        rect.x,
        rect.y,
        button_size,
        button_size
    };
    bool decrease = seui_button_textured_at(ctx, UI_ICON_INDEX_ARROW_LEFT, button);
    /* right button */
    button = (Rect) {
        rect.x + rect.w - button_size,
        rect.y + rect.h - button_size,
        button_size,
        button_size
    };
    bool increase = seui_button_textured_at(ctx, UI_ICON_INDEX_ARROW_RIGHT, button);

    /* value change */
    bool changed = false;
    if (increase) {
        (*value)++;
        changed = true;
    }
    if (decrease) {
        (*value)--;
        changed = true;
    }
    if (min != 0 || max != 0) {
        if (*value > max) *value = max;
        if (*value < min) *value = min;
    }

    /* display value */
    char buffer[SESTRING_MAX_NUM_OF_DIGITS];
    sprintf(buffer, "%i", *value);
    se_add_text_rect(&ctx->txt_renderer, buffer, label);

    return changed;
}

void seui_input_text_at(SE_UI *ctx, SE_String *text, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_bg = (RGBA) {50, 60, 120, 255};
    RGBA colour_highlight = (RGBA) {80, 90, 150, 255};
    Vec3 colour_text = (Vec3) {255, 255, 255};
    Vec3 colour_text_hint = (Vec3) {100, 100, 100};
    RGBA colour = colour_bg;

    SE_String *display_text = text;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, false);
    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_bg;
        } break;
        case UI_STATE_WARM: {
            colour = colour_highlight;

            // allow for increasing and decreasing numerical values while hovering
            if (ctx->text_input_only_numerical) {
                f32 numerical_value = sestring_as_f32(text);
                if (ctx->input->mouse_wheel != 0) {
                    char value_as_string[SESTRING_MAX_NUM_OF_DIGITS];
                    numerical_value += ctx->input->mouse_wheel;
                    sprintf(value_as_string, "%.2f", numerical_value);
                    sestring_clear(text);
                    sestring_append(text, value_as_string);
                }
            }

        } break;
        case UI_STATE_ACTIVE: {
            if (ctx->active != id) {
                // make this the current active widget
                ctx->active = id;
                sestring_duplicate(text, &ctx->text_input); // prepare the cache for new data
                seinput_text_input_activate(input, &ctx->text_input, ctx->text_input_only_numerical);
            }
        } break;
    }

    if (ctx->active == id) {
        display_text = &ctx->text_input;
        colour = RGBA_BLACK;

        // allow for deleting characters
        if (seinput_is_key_pressed(input, SDL_SCANCODE_BACKSPACE)) {
            sestring_delete_from_end(&ctx->text_input, 1);
        }

        // clear when double clicked on
        if (ctx->input->is_mouse_left_down && rect_overlaps_point(rect, ctx->input->mouse_screen_pos)) {
            sestring_clear(&ctx->text_input);
        }

        /* deactivate accept */
        if ((seinput_is_mouse_left_pressed(ctx->input) || seinput_is_mouse_right_pressed(input)) && ui_state != UI_STATE_WARM
            || seinput_is_key_pressed(ctx->input, SDL_SCANCODE_RETURN)) {
            // copy the input text data over to text
            sestring_clear(text); // get rid of what was there and put in the new edited text
            sestring_duplicate(&ctx->text_input, text);
            ctx->active = SEUI_ID_NULL;
            seinput_text_input_deactivate(input);
        } else
        /* deactivate cancel */
        if (seinput_is_key_pressed(ctx->input, SDL_SCANCODE_ESCAPE)) {
            // DON'T copy the input text data over to text
            ctx->active = SEUI_ID_NULL;
            seinput_text_input_deactivate(input);
        }
    }

    seui_render_rect(renderer, rect, colour);
    if (display_text->size > 0) {
        ctx->txt_renderer.config_colour = colour_text;
        se_add_text_rect(&ctx->txt_renderer, display_text->buffer, rect);
    } else if (!ctx->text_input_only_numerical) {
        ctx->txt_renderer.config_colour = colour_text_hint;
        se_add_text_rect(&ctx->txt_renderer, "press to type", rect);
    } else {
        ctx->txt_renderer.config_colour = colour_text_hint;
        se_add_text_rect(&ctx->txt_renderer, "...", rect);
    }
    se_text_reset_config(&ctx->txt_renderer);
}

bool seui_button_textured(SE_UI *ctx, Vec2 texture_index) {
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx, rect.w, rect.h, true);
    }
    return seui_button_textured_at(ctx, texture_index, rect);
}

bool seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max) {
    Rect rect = {0, 0, 100, 32}; // default
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx, rect.w, rect.h, true);
    }
    return seui_selector_at(ctx, rect, value, min, max);
}

/// Colour picker

RGBA seui_colour_picker_at_hsv(SE_UI *ctx, Rect rect, i32 *h, i32 *s, i32 *v) {
    Rect h_rect = {
        rect.x, rect.y, rect.w, 16
    };
    Rect sv_rect = {
        rect.x, rect.y + h_rect.h, rect.w, rect.h - h_rect.h
    };

    u32 id = generate_ui_id(ctx);
    UI_STATES ui_state = get_ui_state(ctx, id, sv_rect, false);
    if (ui_state == UI_STATE_HOT) {
        Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
        i32 window_w, window_h;
        SDL_GetWindowSize(ctx->input->window, &window_w, &window_h);

        // mouse_pos.y = window_h - mouse_pos.y;
        // mouse_pos.x = remapf(mouse_pos.x, window_w, sv_rect.w + sv_rect.x) - sv_rect.x;
        // mouse_pos.y = remapf(mouse_pos.y, window_h, sv_rect.h + sv_rect.y) - sv_rect.y;

        // *s = remapf(mouse_pos.x, sv_rect.w, 100);
        // *v = remapf(mouse_pos.y, sv_rect.h, 100);
        mouse_pos.y = window_h - mouse_pos.y;
        mouse_pos.x -= sv_rect.x;
        mouse_pos.y -= sv_rect.y;
        mouse_pos.x /= sv_rect.w;
        mouse_pos.y /= sv_rect.h;

        (*s) = (i32)(mouse_pos.x * 100);
        (*v) = (i32)(mouse_pos.y * 100);
    }

    // draw the saturation value rect
    seui_render_colour_box(&ctx->renderer, sv_rect, *h);
    // draw the cursor on sv rect
    Vec2 cursor;
    cursor.x = remapf(*s, 100, sv_rect.w) + sv_rect.x;
    cursor.y = remapf(*v, 100, sv_rect.h) + sv_rect.y;
    seui_render_texture(&ctx->renderer, (Rect) {cursor.x - 4, cursor.y - 4, 8, 8}, UI_ICON_INDEX_CIRCLE_EMPTY, RGBA_WHITE);
    // draw the hue slider
    static f32 slider_value = 0.5f;
    seui_slider_at(ctx, v2f(h_rect.x, h_rect.y + h_rect.h / 2), v2f(h_rect.x + h_rect.w, h_rect.y + h_rect.h / 2), &slider_value);
    *h = remapf(slider_value, 1, 360);

    RGBA result;
    hsv_to_rgba(*h, *s, *v, &result);
    return result;
}
