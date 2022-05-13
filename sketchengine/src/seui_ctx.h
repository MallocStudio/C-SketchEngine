#ifndef SEUI_H_CTX
#define SEUI_H_CTX

#include "seui_renderer.h"
#include "seinput.h"
#include "setext.h"

typedef struct SE_Theme {
    RGBA colour_normal;
    RGBA colour_hover;
    RGBA colour_pressed;
    RGBA colour_bg; // background
    RGBA colour_fg; // foreground
} SE_Theme;
SEINLINE void seui_theme_default(SE_Theme *theme) {
    theme->colour_normal  = (RGBA) {65, 84, 105, 255};
    theme->colour_hover   = (RGBA) {108, 145, 173, 255};
    theme->colour_pressed = (RGBA) {43, 56, 71, 255};
    theme->colour_bg      = (RGBA) {33, 39, 43, 200};
    theme->colour_fg      = (RGBA) {56, 95, 161, 255};
}

typedef enum UI_STATES {
    UI_STATE_DISABLED, // this UI element is unusable but is rendererd
    UI_STATE_IDLE,     // this UI element is usable and is rendered
    UI_STATE_WARM,     // about to be selected (hover, selection)
    UI_STATE_HOT,      // this UI element is being used (pressed, a text editor that might be active)
    UI_STATE_ACTIVE,   // this UI element must execute (mouse released on a button...)
} UI_STATES;

#define SEUI_ID_NULL 0

typedef struct SE_UI {
    /* UI Widgets */
    u32 warm; // hover / selection
    u32 hot;  // pressed / active
    u32 max_id; // the maximum generated id

    /* Renderes and Inputs */
    struct UI_Renderer renderer;
    SE_Text_Renderer txt_renderer;
    struct SE_Input *input; // ! not owned
    SE_Theme theme;

    /* Panels */
    struct SEUI_Panel *current_panel;         // the panel we put the widgets on

    /* text input */
    // if some widget is constantly active, we set active to their id
    // this is to allow certain widgets hold the attention of input.
    // For example this is used for text input. If we press on text input,
    // active will be set to that ID, and until the user clicks somewhere else
    // the active ID will remain the same. (This case is handled inside of seui_input_text_at())
    u32 active; // being used (used for text input)
    SE_String text_input_cache;   // used for all sorts of stuff. For example editing floating point values
    SE_String text_input; // store a copy of what's being typed into the active text input
    bool text_input_only_numerical;
} SE_UI;

/// call this at the beginning of every frame before creating other widgets
SEINLINE void seui_reset(SE_UI *ctx) {
    ctx->max_id = SEUI_ID_NULL;
    ctx->current_panel = NULL;
}

SEINLINE void seui_resize(SE_UI *ctx, u32 window_w, u32 window_h) {
    seui_renderer_resize(&ctx->renderer, window_w, window_h);
    setext_set_viewport(&ctx->txt_renderer, (Rect) {0, 0, window_w, window_h});
}

SEINLINE void seui_init(SE_UI *ctx, SE_Input *input, u32 window_w, u32 window_h) {
    ctx->warm = SEUI_ID_NULL;
    ctx->hot = SEUI_ID_NULL;
    ctx->active = SEUI_ID_NULL;
    seui_reset(ctx);
    ctx->input = input;
    seui_renderer_init(&ctx->renderer, "shaders/UI.vsd", "shaders/UI.fsd", window_w, window_h);
    setext_init (&ctx->txt_renderer, (Rect) {0, 0, window_w, window_h});
    seui_theme_default(&ctx->theme);

    sestring_init(&ctx->text_input_cache, "");
    sestring_init(&ctx->text_input, "");
}

SEINLINE void seui_deinit(SE_UI *ctx) {
    seui_renderer_deinit(&ctx->renderer);
    setext_deinit(&ctx->txt_renderer);
    sestring_deinit(&ctx->text_input_cache);
    sestring_deinit(&ctx->text_input);
}

SEINLINE void seui_render(SE_UI *ctx) {
    seui_renderer_upload(&ctx->renderer);
    glDisable(GL_CULL_FACE);
    seui_renderer_draw(&ctx->renderer);
    setext_render(&ctx->txt_renderer);
    glEnable(GL_CULL_FACE);
    seui_renderer_clear (&ctx->renderer);
}

/// Set text input configurations to default values. It's good practice to
/// do call this procedure after modifying the configuration for a usecase.
SEINLINE void seui_configure_text_input_reset(SE_UI *ctx) {
    ctx->text_input_only_numerical = false;
}

#endif // SEUI_H_CTX
