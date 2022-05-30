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
        rect = seui_panel_put(ctx, rect.w, true);
    }
    return seui_button_textured_at(ctx, texture_index, rect);
}

bool seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max) {
    Rect rect = {0, 0, 100, 32}; // default
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, rect.w, true);
    }
    return seui_selector_at(ctx, rect, value, min, max);
}

void seui_hsv_picker(SE_UI *ctx, HSV *hsv) {
    if (hsv->h < 0) hsv->h = 0;
    if (hsv->h >= 360) hsv->h = 0;

    if (seui_panel_at(ctx, "hsv picker")) {
        { /* colour picker wheel and triangle and functionality */
            seui_panel_row(ctx, 128, 1);
            Rect rect = seui_panel_put(ctx, 128, false);
            Vec2 center = v2f(rect.x + rect.w / 2, rect.y + rect.h / 2);
            f32 outer_radius = rect.h / 2;
            f32 thickness = 16;
            /* colour wheel */
            seui_render_shape_colour_wheel(&ctx->renderer, center, outer_radius, thickness);

            f32 angle = hsv->h * SEMATH_DEG2RAD_MULTIPLIER;
            f32 radius = outer_radius - thickness;

            { /* colour triangle */
                seui_render_shape_colour_triangle(&ctx->renderer, center, radius, angle);
                if (seinput_is_key_down(ctx->input, SDL_SCANCODE_H)) {
                    hsv->h += 5;// * SEMATH_DEG2RAD_MULTIPLIER;
                    printf("h: %i\n", hsv->h);
                }
            }
            { /* the cursor on the triangle */
                Vec2 colour_tip = {
                    semath_cos(angle) * radius + center.x,
                    semath_sin(angle) * radius + center.y,
                };
                Vec2 white_tip = {
                    semath_cos(angle + 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.x,
                    semath_sin(angle + 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.y,
                };
                Vec2 black_tip = {
                    semath_cos(angle - 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.x,
                    semath_sin(angle - 120 * SEMATH_DEG2RAD_MULTIPLIER) * radius + center.y,
                };
                Vec2 hsv_point = colour_tip;

                hsv_point.x = lerp(hsv_point.x, white_tip.x, 1 - hsv->s);
                hsv_point.y = lerp(hsv_point.y, white_tip.y, 1 - hsv->s);

                hsv_point.x = lerp(hsv_point.x, black_tip.x, 1 - hsv->v);
                hsv_point.y = lerp(hsv_point.y, black_tip.y, 1 - hsv->v);

                seui_render_circle_outline_ext(&ctx->renderer, hsv_point, 4, RGBA_WHITE, 8);

                /* changing the saturation and value based on mouse input */
                Vec2 mouse_pos = get_mouse_pos(NULL, NULL);
                mouse_pos.y = ctx->txt_renderer.viewport.h - mouse_pos.y;

                if (seinput_is_mouse_left_pressed(ctx->input) && point_overlaps_circle(mouse_pos, center, radius)) {
                    static i32 i = 0;
                    printf("pressed %i\n", i++);
                    Vec3 result = cartesian_to_barycentric_coordinates(mouse_pos, colour_tip, white_tip, black_tip);
                    if (result.x >= 0 && result.x <= 1 && result.y >= 0 && result.y <= 1 && result.z >= 0 && result.z <= 1) {
                        hsv->s = result.x - result.y;
                        hsv->v = result.x - result.z;
                    }
                }
            }
        }
        { /* buttons and sliders */
            seui_panel_row(ctx, 32, 3);
            Vec3 label_val = {
                hsv->h,
                hsv->s * 100,
                hsv->v * 100
            };
            seui_label_vec3(ctx, "hsv", &label_val, true);
            if (label_val.x >= 360) label_val.x = 0;
            if (label_val.x < 0) label_val.x = 359;
            if (label_val.y < 0) label_val.y = 0;
            if (label_val.y > 100) label_val.y = 100;
            if (label_val.z < 0) label_val.z = 0;
            if (label_val.z > 100) label_val.z = 100;
            hsv->h = label_val.x;
            hsv->s = label_val.y / 100;
            hsv->v = label_val.z / 100;
        }
    }
}

void seui_texture_viewer(SE_UI *ctx, u32 texture_index) {
    if (seui_panel_at(ctx, "texture viewer")) {
        Rect rect = ctx->current_panel->cached_rect;
        rect.h -= 48;
        rect.y += 16;
        ctx->current_panel->fit_size = v2f(128, 128);

        if (rect.h > rect.w) {
            rect.h = rect.w;
        }
        if (rect.w > rect.h) {
            rect.w = rect.h;
        }
        seui_render_texture_raw(&ctx->renderer, rect, texture_index);
    }
}