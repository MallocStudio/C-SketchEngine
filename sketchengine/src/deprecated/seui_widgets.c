// - seui_ctx (ctx functionalities)
// - seui_panel (panel functionalities and layouts)
// - seui_components (basic widgets such as buttons, labels, text inputs)
// - seui_widgets (complex forms of widgets made out of other widgets)
#include "seui_components.h"

/// -----------------------------------------
///                WIDGETS
/// -----------------------------------------

bool seui_button(SE_UI *ctx, const char *text) {
    Vec2 text_size = se_size_text(&ctx->txt_renderer, text);
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, text_size.x, true);
    }
    return seui_button_at(ctx, text, rect);
}

void seui_label(SE_UI *ctx, const char *text) {
    Rect rect = {0, 0, 16, 16}; // default
    if (ctx->current_panel != NULL) {
        Vec2 text_size = se_size_text(&ctx->txt_renderer, text);
        rect = seui_panel_put(ctx, text_size.x, true);
    }
    seui_label_at(ctx, text, rect);
}

void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, bool editable) {
    char label_buffer[255];
    seui_panel_row(ctx, 32, 1);

    bool previous_setting = ctx->current_panel->config_item_centered;
    ctx->current_panel->config_item_centered = true;
    seui_label(ctx, title);
        // reset setting
    ctx->current_panel->config_item_centered = previous_setting;

    if (!editable) {
        seui_panel_row(ctx, 32, 3);
        sprintf(label_buffer, "x: %.2f", value->x);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "y: %.2f", value->y);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "z: %.2f", value->z);
        seui_label(ctx, label_buffer);
    } else {
        ctx->text_input_only_numerical = true;
        seui_panel_row(ctx, 32, 6);

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

void seui_label_hsv(SE_UI *ctx, const char *title, HSV *value, bool editable) {
    char label_buffer[255];
    seui_panel_row(ctx, 32, 1);

    bool previous_setting = ctx->current_panel->config_item_centered;
    ctx->current_panel->config_item_centered = true;
    seui_label(ctx, title);
        // reset setting
    ctx->current_panel->config_item_centered = previous_setting;

    if (!editable) {
        seui_panel_row(ctx, 32, 3);
        sprintf(label_buffer, "h: %i", value->h);
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "s: %i", (i32)(value->s * 100));
        seui_label(ctx, label_buffer);
        sprintf(label_buffer, "v: %i", (i32)(value->v * 100));
        seui_label(ctx, label_buffer);
    } else {
        ctx->text_input_only_numerical = true;
        seui_panel_row(ctx, 32, 6);

        sprintf(label_buffer, "h:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%i", value->h);
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->h = (i32)sestring_as_f32(&ctx->text_input_cache);
        if (value->h < 0) value->h = 359;
        if (value->h > 359) value->h = 0;

        sprintf(label_buffer, "s:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%i", (i32)(value->s * 100));
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->s = sestring_as_f32(&ctx->text_input_cache) / 100.0f;
        if (value->s < 0) value->s = 0;
        if (value->s > 1) value->s = 1;

        sprintf(label_buffer, "v:");
        seui_label(ctx, label_buffer);

        sprintf(label_buffer, "%i", (i32)(value->v * 100));
        sestring_clear(&ctx->text_input_cache);
        sestring_append(&ctx->text_input_cache, label_buffer);
        seui_input_text(ctx, &ctx->text_input_cache);
        value->v = sestring_as_f32(&ctx->text_input_cache) / 100.0f;
        if (value->v < 0) value->v = 0;
        if (value->v > 1) value->v = 1;

        seui_configure_text_input_reset(ctx); // reset configurations
    }
}

void seui_slider(SE_UI *ctx, f32 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, 24, true);
    }
    Vec2 pos1 = {rect.x, (rect.y + rect.y + rect.h) * 0.5f};
    Vec2 pos2 = {rect.x + rect.w, (rect.y + rect.y + rect.h) * 0.5f};
    seui_slider_at(ctx, pos1, pos2, value);
}

void seui_slider2d(SE_UI *ctx, Vec2 *value) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, 32, true);
    }
    Vec2 center = {
        (rect.x + rect.x + rect.w) * 0.5f,
        (rect.y + rect.y + rect.h) * 0.5f
    };
    f32 radius = rect.h * 0.5f;
    seui_slider2d_at(ctx, center, radius, value);
}

void seui_input_text(SE_UI *ctx, SE_String *text) {
    Rect rect = {0, 0, 16, 16}; // default label size
    if (ctx->current_panel != NULL) {
        rect = seui_panel_put(ctx, 100, true);
    }
    seui_input_text_at(ctx, text, rect);
}