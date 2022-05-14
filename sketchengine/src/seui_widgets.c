// - seui_ctx (ctx functionalities)
// - seui_panel (panel functionalities and layouts)
// - seui_components (basic widgets such as buttons, labels, text inputs)
// - seui_widgets (complex forms of widgets made out of other widgets)
#include "seui_widgets.h"


/// -----------------------------------------
///                WIDGETS
/// -----------------------------------------

bool seui_button(SE_UI *ctx, const char *text) {
    Vec2 text_size = setext_size_string(&ctx->txt_renderer, text);
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, text_size.x, text_size.y, true);
    }
    return seui_button_at(ctx, text, rect);
}

void seui_label(SE_UI *ctx, const char *text) {
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        Vec2 text_size = setext_size_string(&ctx->txt_renderer, text);
        rect = panel_put(ctx->current_panel, text_size.x, text_size.y, true);
    }
    seui_label_at(ctx, text, rect);
}

void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, bool editable) {
    char label_buffer[255];
    seui_panel_row(ctx->current_panel, 1);
    seui_label(ctx, title);

    if (!editable) {
        seui_panel_row(ctx->current_panel, 3);
        sprintf(label_buffer, "x: %.2f", value->x);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "y: %.2f", value->y);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "z: %.2f", value->z);
        seui_label(ctx, label_buffer);
    } else {
        ctx->text_input_only_numerical = true;
        seui_panel_row(ctx->current_panel, 6);

        sprintf(label_buffer, "x:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%.2f", value->x);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->x = sestring_as_f32(&ctx->text_input_cache);

        sprintf(label_buffer, "y:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%.2f", value->y);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->y = sestring_as_f32(&ctx->text_input_cache);

        sprintf(label_buffer, "z:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%.2f", value->z);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->z = sestring_as_f32(&ctx->text_input_cache);

        seui_configure_text_input_reset(ctx); // reset configurations
    }
}

void seui_slider(SE_UI *ctx, f32 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 24, 24, true);
    }
    Vec2 pos1 = {rect.x, (rect.y + rect.y + rect.h) * 0.5f};
    Vec2 pos2 = {rect.x + rect.w, (rect.y + rect.y + rect.h) * 0.5f};
    seui_slider_at(ctx, pos1, pos2, value);
}

void seui_slider2d(SE_UI *ctx, Vec2 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 32, 32, true);
    }
    Vec2 center = {
        (rect.x + rect.x + rect.w) * 0.5f,
        (rect.y + rect.y + rect.h) * 0.5f
    };
    f32 radius = rect.h * 0.8f;
    seui_slider2d_at(ctx, center, radius, value);
}

void seui_colour_picker(SE_UI *ctx, RGBA *value) {
    Rect rect = {0, 0, 128, 128}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, rect.w, rect.h, true);
    }
    seui_colour_picker_at(ctx, rect, value);
}

void seui_input_text(SE_UI *ctx, SE_String *text) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, 32, 32, true);
    }
    seui_input_text_at(ctx, text, rect);
}

RGBA seui_colour_picker_hsv(SE_UI *ctx, i32 *h, i32 *s, i32 *v) {
    Rect rect = {0, 0, 128, 128}; // default label size
    if (ctx->current_panel != NULL) {
        rect = panel_put(ctx->current_panel, rect.w, rect.h, true);
    }
    return seui_colour_picker_at_hsv(ctx, rect, h, s, v);
}