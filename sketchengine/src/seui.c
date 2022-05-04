#include "seui.h"

static u32 generate_ui_id(SE_UI *ctx) {
    ctx->max_id++; // we start with zero. So we increase first.
    u32 id = ctx->max_id;
    return id;
}

/// note that stay_active_on_mouse_leave is used for dragging ui items
static UI_STATES get_ui_state (SE_UI *ctx, u32 id, Rect rect, SE_Input *input, bool stay_active_on_mouse_leave /* = false */, bool remain_active /* = false (used for text input)*/) {
    UI_STATES result = UI_STATE_IDLE;

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
    } else {
        // if ACTIVE
        if (remain_active) {
            ctx->active = id;
        } else {
            ctx->active = SEUI_ID_NULL;
        }
    }

    if (ctx->active == id) result = UI_STATE_ACTIVE;

    return result;
}

static Rect expand_view_region(SE_UI *ctx, Rect normalised_rect) {
    Rect result = normalised_rect;
    result.x *= ctx->renderer.view_width;
    result.w *= ctx->renderer.view_width;
    result.y *= ctx->renderer.view_height;
    result.h *= ctx->renderer.view_height;
    return result;
}

void seui_panel_row(SEUI_Panel *panel, f32 num_of_columns) {
    panel->cursor.x = 0;
    panel->item_height = panel->min_item_height; // reset row height
    panel->columns = num_of_columns;
}

/// Returns a rectangle that's suppose to be the rect
/// of the new item inside of the current panel.
static Rect panel_put(SEUI_Panel *panel, f32 min_width, f32 min_height) {
    if (panel == NULL) {
        printf("ERROR: panel_put but panel was null\n");
        return (Rect) {0, 0, 32, 32};
    }

    Rect panel_rect = panel->rect;
    Vec2 cursor = panel->cursor; // relative space
    cursor = vec2_add(cursor, (Vec2) {panel_rect.x, panel_rect.y}); // global space

    Rect result = {0};
    {
        f32 height = min_height; // this item's height

        if (height <= 0) {
            height = panel->item_height;
        }

        if (min_height > panel->item_height) {
            panel->item_height = min_height; // update panel item height so other widgets in the same row will have expanded size
        }

        result.w = panel_rect.w / panel->columns;
        result.h = panel->item_height;
        result.x = cursor.x;
        result.y = cursor.y - result.h;
    }

    // increment min width
    if (panel->cursor.x > min_width) {
        panel->min_size.x += min_width;
    }

    // Increment the cursor
    panel->cursor.x += result.w;
    if (panel->cursor.x > panel_rect.w - 1) { // -1 gets rid of floating point error while comparing
        seui_panel_row(panel, panel->columns);
        panel->cursor.y -= panel->item_height;
        panel->min_size.y += panel->item_height;
    }

    panel->item_count++;
    return result;
}

bool seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel_data) {
    Rect *initial_rect = &panel_data->initial_rect; // ! note that initial rect gets updated as we add widgets to this panel
    bool *minimised    = &panel_data->minimised;

    Rect rect = *initial_rect; // the panel's dimensions of this frame
    bool is_minimised = *minimised;
    RGBA colour = ctx->theme.colour_bg;

    ctx->current_panel = panel_data;
    panel_data->rect = rect;
    panel_data->cursor = (Vec2) {
        0, // start from the top left
        rect.h
    };
    panel_data->item_height = panel_data->min_item_height;

    // draw a rectangle that represents the panel's dimensions
    if (!is_minimised) seui_render_rect(&ctx->renderer, rect, colour);

    { // panel widgets
        f32 minimise_button_size = 16;
        Vec2 cursor = vec2_add(panel_data->cursor, (Vec2) {rect.x, rect.y});

        Rect drag_button_rect = (Rect) {cursor.x, cursor.y, rect.w - minimise_button_size, minimise_button_size};
        Rect minimise_button_rect = (Rect) {cursor.x + rect.w - minimise_button_size, cursor.y, minimise_button_size, minimise_button_size};

        UI_STATES drag_state;
        Vec2 drag = seui_drag_button_at(ctx, drag_button_rect, &drag_state);
        initial_rect->x += drag.x;
        initial_rect->y += drag.y;

        if (seui_button_at(ctx, "", minimise_button_rect)) {
            *minimised = !*minimised;
        }

        /* resizeing */
        if (!is_minimised) {
            Vec2 min_size = panel_data->min_size;
            Rect resize_button = {
                rect.x + rect.w- 16, rect.y, 16, 16
            };
            Vec2 resize = seui_drag_button_at(ctx, resize_button, NULL);

            initial_rect->w += resize.x;

            if (initial_rect->h - resize.y > min_size.y) {
                initial_rect->h -= resize.y;
                initial_rect->y += resize.y;
            }
            // clamp to min size
            if (initial_rect->w < min_size.x) initial_rect->w = min_size.x;
            if (initial_rect->h < min_size.y) initial_rect->h = min_size.y;
        }

        // minimise button
        Vec2 index = is_minimised ? UI_ICON_INDEX_UNCOLLAPSE : UI_ICON_INDEX_COLLAPSE;
        seui_render_texture(&ctx->renderer, minimise_button_rect, index, RGBA_WHITE);

        { // -- docking
            RGBA dock_colour = {150, 0, 0, 100};
            Rect normalised_rect = *initial_rect;
            normalised_rect.x /= ctx->renderer.view_width;
            normalised_rect.w /= ctx->renderer.view_width;
            normalised_rect.y /= ctx->renderer.view_height;
            normalised_rect.h /= ctx->renderer.view_height;

            Vec2 normalised_cursor = ctx->input->mouse_screen_pos;
            normalised_cursor.x /= ctx->renderer.view_width;
            normalised_cursor.y /= ctx->renderer.view_height;

            // @note UI_STATE_ACTIVE means that the button was just released
            if (rect_overlaps_point(SEUI_VIEW_REGION_COLLISION_RIGHT, normalised_cursor)) { // right
                if (drag_state == UI_STATE_HOT) {
                    seui_render_rect(&ctx->renderer, expand_view_region(ctx, SEUI_VIEW_REGION_RIGHT), dock_colour);
                }
                if (drag_state == UI_STATE_ACTIVE && !ctx->input->is_mouse_left_down) { // mouse released so dock
                    printf("docked\n");
                    normalised_rect = expand_view_region(ctx, SEUI_VIEW_REGION_RIGHT);
                    *initial_rect = normalised_rect;
                }
            } else
            if (rect_overlaps_point(SEUI_VIEW_REGION_COLLISION_LEFT, normalised_cursor)) { // left
                if (drag_state == UI_STATE_HOT) {
                    seui_render_rect(&ctx->renderer, expand_view_region(ctx, SEUI_VIEW_REGION_LEFT), dock_colour);
                }
                if (drag_state == UI_STATE_ACTIVE && !ctx->input->is_mouse_left_down) { // mouse released so dock
                    printf("docked\n");
                    normalised_rect = expand_view_region(ctx, SEUI_VIEW_REGION_LEFT);
                    *initial_rect = normalised_rect;
                }
            }
        }
    }

    // @TODO investigate what this does
    panel_data->min_size = (Vec2) {0};
    panel_data->min_size.x = panel_data->columns * 64; // 64 is default item width
    panel_data->min_size.y = panel_data->item_height;

    return !is_minimised;
}


bool seui_panel(SE_UI *ctx, const char *title, SEUI_Panel *panel_data) {

    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, panel_data->initial_rect.w, panel_data->initial_rect.h);
        panel_data->initial_rect = rect;
    }
    return seui_panel_at(ctx, title, panel_data);
}

bool seui_button(SE_UI *ctx, const char *text) {
    Vec2 text_size = setext_size_string(&ctx->txt_renderer, text);
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, text_size.x, text_size.y);
    }
    return seui_button_at(ctx, text, rect);
}

void seui_label(SE_UI *ctx, const char *text) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        Vec2 text_size = setext_size_string(&ctx->txt_renderer, text);
        rect = panel_put(ctx->current_panel, text_size.x, text_size.y);
    }
    seui_label_at(ctx, text, rect);
}

void seui_slider(SE_UI *ctx, f32 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 24, 24);
    }
    Vec2 pos1 = {rect.x, (rect.y + rect.y + rect.h) * 0.5f};
    Vec2 pos2 = {rect.x + rect.w, (rect.y + rect.y + rect.h) * 0.5f};
    seui_slider_at(ctx, pos1, pos2, value);
}

void seui_slider2d(SE_UI *ctx, Vec2 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 32, 32);
    }
    Vec2 center = {
        (rect.x + rect.x + rect.w) * 0.5f,
        (rect.y + rect.y + rect.h) * 0.5f
    };
    f32 radius = rect.h * 0.8f;
    seui_slider2d_at(ctx, center, radius, value);
}

void seui_colour_picker(SE_UI *ctx, RGBA hue, RGBA *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 32, 32);
    }
    seui_colour_picker_at(ctx, rect, hue, value);
}

void seui_input_text(SE_UI *ctx, SE_String *text) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 32, 32);
    }
    seui_input_text_at(ctx, text, rect);
}

bool seui_button_at(SE_UI *ctx, const char *text, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = ctx->theme.colour_normal;
    RGBA colour_hover   = ctx->theme.colour_hover;
    RGBA colour_pressed = ctx->theme.colour_pressed;
    RGBA colour = colour_normal;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, input, false, false);
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
    setext_render_text_rect(&ctx->txt_renderer, text, rect, vec3_create(1, 1, 1), true);

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
    UI_STATES ui_state = get_ui_state(ctx, id, rect, input, true, false);

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
    setext_render_text_rect(&ctx->txt_renderer, text, rect, vec3_create(1, 1, 1), true);
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
    seui_render_line(&ctx->renderer, pos1, pos2, 3);

    /* draw the slider button */
    Rect button_rect = rect_create(button_pos, button_size);

    Vec2 drag = seui_drag_button_textured_at(ctx, button_rect, UI_ICON_INDEX_SLIDER, NULL);

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

void seui_colour_picker_at(SE_UI *ctx, Rect rect, RGBA hue, RGBA *value) {
    seui_render_colour_picker(&ctx->renderer, rect, hue);
}

void seui_input_text_at(SE_UI *ctx, SE_String *text, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_bg = (RGBA) {50, 60, 120, 255};
    RGBA colour_highlight = (RGBA) {80, 90, 150, 255};
    Vec3 colour_text = (Vec3) {255, 255, 255};
    RGBA colour = colour_bg;

    SE_String *displayed_text = text;

    u32 current_active = ctx->active;
    UI_STATES ui_state = get_ui_state(ctx, id, rect, input, false, true);
    switch (ui_state) {
        case UI_STATE_IDLE: {
            colour = colour_bg;
        } break;
        case UI_STATE_WARM: {
            colour = colour_highlight;
        } break;
        case UI_STATE_ACTIVE: {
            colour = RGBA_BLACK;

            if (input->is_text_input_activated == false) {
                sestring_clear(text);
                sestring_duplicate(&input->text_input, text);
                seinput_text_input_activate(input, text->buffer);
            }

            displayed_text = &input->text_input;
            if (seinput_is_key_pressed(input, SDL_SCANCODE_BACKSPACE) || seinput_is_mouse_right_pressed(input)) {
                sestring_delete_from_end(displayed_text, 1);
            }
        } break;
    }

    if (current_active != id) { // if we were previously active but not anymore
        sestring_clear(text);
        sestring_duplicate(&input->text_input, text);
        seinput_text_input_deactivate(input);
    }

    seui_render_rect(renderer, rect, colour);
    setext_render_text_rect(&ctx->txt_renderer, displayed_text->buffer, rect, colour_text, true);
}