#ifndef SEUI_H
#define SEUI_H

#include "seui_renderer.h"
#include "seinput.h"
#include "setext.h"

typedef enum UI_STATES {
    UI_STATE_DISABLED, // this UI element is unusable but is rendererd
    UI_STATE_IDLE,     // this UI element is usable and is rendered
    UI_STATE_WARM,     // about to be selected (hover, selection)
    UI_STATE_HOT,      // this UI element is being used (pressed, a text editor that might be active)
    UI_STATE_ACTIVE,   // this UI element must execute (mouse released on a button...)
} UI_STATES;

#define SEUI_ID_NULL 0
typedef struct SE_UI {
    u32 warm; // hover / selection
    u32 hot;  // pressed / active
    u32 max_id; // the maximum generated id
    struct UI_Renderer renderer;
    SE_Text_Renderer txt_renderer;
    struct SE_Input *input; // ! not owned
} SE_UI;

/// call this at the beginning of every frame before creating other widgets
SEINLINE void seui_reset(SE_UI *ctx) {
    ctx->max_id = SEUI_ID_NULL;
}

SEINLINE void seui_init(SE_UI *ctx, SE_Input *input, u32 window_w, u32 window_h) {
    ctx->warm = SEUI_ID_NULL;
    ctx->hot = SEUI_ID_NULL;
    seui_reset(ctx);
    ctx->input = input;
    seui_renderer_init(&ctx->renderer, "shaders/UI.vsd", "shaders/UI.fsd", window_w, window_h);
    setext_init (&ctx->txt_renderer, (Rect) {0, 0, window_w, window_h});
}


SEINLINE void seui_deinit(SE_UI *ctx) {
    seui_renderer_deinit(&ctx->renderer);
    setext_deinit(&ctx->txt_renderer);
}

SEINLINE void seui_render(SE_UI *ctx) {
    seui_renderer_upload(&ctx->renderer);
    seui_renderer_draw  (&ctx->renderer);
    seui_renderer_clear (&ctx->renderer);
    setext_render(&ctx->txt_renderer);
}

bool seui_button(SE_UI *ctx, const char *text, Rect rect);

#endif // SEUI_H