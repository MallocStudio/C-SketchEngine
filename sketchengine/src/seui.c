#include "seui.h"

static bool point_overlaps_rect(Vec2 point, Rect rect) {
    return (point.x > rect.x && point.x < rect.x + rect.w) && (point.y > rect.y && point.y < rect.y + rect.h);
}

static bool rect_overlaps_rect(Rect a, Rect b) {
    // following Ericson, C, 2004. Real-Time Collision Detection. 1.  CRC Press.
    // page 79, AABB vs AABB
    f32 t;
    if ((t = a.x - b.x) > b.w || -t > a.w) return false;
    if ((t = a.y - b.y) > b.h || -t > a.h) return false;
    return true;
}

static u32 generate_ui_id(SE_UI *ctx) {
    u32 id = ctx->max_id;
    ctx->max_id++;
}

static UI_STATES get_ui_state (SE_UI *ctx, u32 id, Rect rect, SE_Input *input) {
    UI_STATES result = UI_STATE_IDLE;

    bool mouse_up = !input->is_mouse_left_down;
    bool mouse_down = !mouse_up;
    bool mouse_inside = point_overlaps_rect(input->mouse_screen_pos, rect);

    if (ctx->hot == id) { // pressing down
        if (mouse_up) { // make active
            if (ctx->hot == id) result = UI_STATE_ACTIVE; // mouse up while pressing down
            ctx->hot = SEUI_ID_NULL; // no longer hot
        }
    } else if (ctx->warm == id) { // hover
        if (mouse_down) { // make hot
            input->is_mouse_left_handled = true; // tell input that we've used up this input
            ctx->hot = id;
            result = UI_STATE_HOT;
        } else { // make warm
            result = UI_STATE_WARM;
        }
    }

    if (mouse_inside) {
        // if no other item is hot, make us warm
        if (ctx->hot == SEUI_ID_NULL) {
            ctx->warm = id;
        }
    } else if (ctx->warm == id) {
        ctx->warm = SEUI_ID_NULL;
    } else if (ctx->hot == id) {
        ctx->hot = SEUI_ID_NULL;
    }

    return result;
}

bool seui_button(SE_UI *ctx, const char *text, Rect rect) {
    SE_Input *input = ctx->input;
    UI_Renderer *renderer = &ctx->renderer;

    u32 id = generate_ui_id(ctx);

    RGBA colour_normal  = (RGBA) {200, 150, 150, 255};
    RGBA colour_hover   = (RGBA) {250, 150, 150, 255};
    RGBA colour_pressed = (RGBA) {100, 50, 50, 255};
    RGBA colour = colour_normal;

    UI_STATES ui_state = get_ui_state(ctx, id, rect, input);
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
    // the reason text is not rendering is because depth buffer is enabled and for some reason they pick up text as behind
    setext_render_text_rect(&ctx->txt_renderer, text, rect, vec3_create(1, 1, 1));

    return ui_state == UI_STATE_ACTIVE;
}