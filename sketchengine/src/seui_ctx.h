#ifndef SEUI_H_CTX
#define SEUI_H_CTX

#include "serenderer2D.h"
#include "seinput.h"
#include "setext.h"

#define UI_ICON_INDEX_NULL       (Vec2) {0, 0}
#define UI_ICON_INDEX_COLLAPSE   (Vec2) {1, 0}
#define UI_ICON_INDEX_UNCOLLAPSE (Vec2) {2, 0}
#define UI_ICON_INDEX_CLOSE      (Vec2) {0, 1}
#define UI_ICON_INDEX_SLIDER     (Vec2) {3, 0}
#define UI_ICON_INDEX_CIRCLE_FILLED (Vec2) {1, 1}
#define UI_ICON_INDEX_CIRCLE_EMPTY (Vec2) {1, 2}
#define UI_ICON_INDEX_CHECKBOX_FILLED (Vec2) {3, 2}
#define UI_ICON_INDEX_CHECKBOX_EMPTY (Vec2) {2, 2}
#define UI_ICON_INDEX_ARROW_RIGHT (Vec2) {2, 1}
#define UI_ICON_INDEX_ARROW_LEFT (Vec2) {3, 1}

// #define SEUI_VIEW_REGION_PADDING 0.1f
#define SEUI_VIEW_REGION_SIZE_X 0.2f
#define SEUI_VIEW_REGION_SIZE_Y 0.9f
#define SEUI_VIEW_REGION_COLLISION_SIZE_X 0.01f
#define SEUI_VIEW_REGION_COLLISION_SIZE_Y SEUI_VIEW_REGION_SIZE_Y
// #define SEUI_VIEW_REGION_CENTER (Rect) {SEUI_VIEW_REGION_PADDING, SEUI_VIEW_REGION_PADDING, 1 - SEUI_VIEW_REGION_SIZE, SEUI_VIEW_REGION_PADDING * 2}
#define SEUI_VIEW_REGION_RIGHT  (Rect) {1 - SEUI_VIEW_REGION_SIZE_X, (1 - SEUI_VIEW_REGION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_SIZE_X, SEUI_VIEW_REGION_SIZE_Y}
#define SEUI_VIEW_REGION_LEFT   (Rect) {0, (1 - SEUI_VIEW_REGION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_SIZE_X, SEUI_VIEW_REGION_SIZE_Y}

#define SEUI_VIEW_REGION_COLLISION_RIGHT  (Rect) {1 - SEUI_VIEW_REGION_COLLISION_SIZE_X, (1 - SEUI_VIEW_REGION_COLLISION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_COLLISION_SIZE_X, SEUI_VIEW_REGION_COLLISION_SIZE_Y}
#define SEUI_VIEW_REGION_COLLISION_LEFT   (Rect) {0, (1 - SEUI_VIEW_REGION_COLLISION_SIZE_Y) *0.5f, SEUI_VIEW_REGION_COLLISION_SIZE_X, SEUI_VIEW_REGION_COLLISION_SIZE_Y}

typedef struct SEUI_Panel {
    /* CAN BE SET DIRECTLY ---------------------------------------------------- */
        /* positioning of panel */
        Rect calc_rect;
        bool minimised;
        f32 min_item_height;
        // 0 means not docked, 1 means left, 2 means right
        u32 docked_dir;
        // this is the minimum size the user can resize the panel to.
        // note that this is not automatically calculated, so this can be set
        // by the user at any point.
        // Vec2 min_size; // not ethat fit_size can be set directly by the user if not putting any widgets inside of the panel

        /* layouting configuration */
        f32 config_row_left_margin;
        f32 config_row_right_margin;
        bool config_item_centered;

        const char *title;

    /* auto calculated ------------------------------------------------------- */
        u32 index; // the index identifier of the panel tracked by SE_UI
        bool is_closed;
        u32 depth; // the depth that the panel is drawn with
        /* next item: (calculated based on layout) */
        // sense some items can be bigger than min row height in the same row,
        // we want all the items on that row to be of the same height
        f32 next_item_height;
        // the relative cursor used to position the placement of the items
        Vec2 cursor;

        /* layouting */
        i32 row_width;   // the width of the current row
        i32 row_height;  // the height of the current row
        i32 row_columns; // number of the columns in the current row

        /* positioning of panel */
        // this size is calculated based on the widgets that populate the panel.
        // this is the minimum size that it takes to fit everything inside of the
        // panel without clipping or scissoring.
        Vec2 fit_size;
        Vec2 fit_cursor;
        Rect cached_rect; // the rect of the panel from the previous frame
        bool is_embedded; // is inside of another panel
} SEUI_Panel;

typedef struct SE_Theme {
    /* colours */
    RGBA colour_normal;
    RGBA colour_hover;
    RGBA colour_pressed;
    RGBA colour_bg; // background
    RGBA colour_fg; // foreground
    RGBA colour_bg_2;

    /* positioning */
    Vec2 margin; // x for horizontal margin, y for vertical margin
} SE_Theme;
SEINLINE void seui_theme_default(SE_Theme *theme) {
    // theme->colour_normal  = (RGBA) {65, 84, 105, 255};
    // theme->colour_hover   = (RGBA) {108, 145, 173, 255};
    // theme->colour_pressed = (RGBA) {43, 56, 71, 255};
    // theme->colour_bg      = (RGBA) {33, 39, 43, 200};
    // theme->colour_fg      = (RGBA) {56, 95, 161, 255};
    // theme->margin = v2f(8, 0);
    theme->colour_normal  = (RGBA) {227, 126, 39, 255};
    theme->colour_hover   = (RGBA) {242, 145, 85, 255};
    theme->colour_pressed = (RGBA) {156, 67, 12, 255};
    theme->colour_bg      = (RGBA) {59, 34, 32, 230};
    theme->colour_fg      = (RGBA) {56, 95, 161, 255};
    // theme->colour_bg_2    = (RGBA) {10, 10, 10, 230};
    theme->colour_bg_2      = (RGBA) {59, 34, 32, 230};
    theme->margin = v2f(8, 0);
}

typedef enum UI_STATES {
    UI_STATE_DISABLED, // this UI element is unusable but is rendererd
    UI_STATE_IDLE,     // this UI element is usable and is rendered
    UI_STATE_WARM,     // about to be selected (hover, selection)
    UI_STATE_HOT,      // this UI element is being used (pressed, a text editor that might be active)
    UI_STATE_ACTIVE,   // this UI element must execute (mouse released on a button...)
} UI_STATES;

#define SEUI_ID_NULL 0
#define SEUI_PANEL_CONTAINER_CAPACITY 100
typedef struct SE_UI {
    /* UI Widgets */
    u32 warm; // hover / selection
    u32 hot;  // pressed / active
    u32 max_id; // the maximum generated id
    Rect viewport;

    /* Renderes and Inputs */
    SE_Renderer2D renderer;
    SE_Text txt_renderer;
    struct SE_Input *input; // ! not owned
    SE_Theme theme;

    /* Panels */
    u32 panel_container_count;
        // a list of panel containers that can be used to refer to other existing panels
        // this list refers to the index of the panels in the @panels of SE_UI
        // note that a panel_container of value (0) means that this container does not refer to anything
        // and if the value was (1) it means that the container is refering to panel at index 0
    u32 panel_containers[SEUI_PANEL_CONTAINER_CAPACITY];

    u32 panel_capacity;
    u32 panel_count;
    SEUI_Panel *panels;

    SEUI_Panel *current_panel; // the panel we put the widgets on
    SEUI_Panel *current_dragging_panel; // the panel that the user is dragging

    /* data */
    // data slots are places where widgets can store user data to, such as text input, colour, etc.
    HSV data_hsv; // stored hsv (this is used by colour pickers to store their value to)
    SE_Texture_Atlas icon_atlas;
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

/// Set text input configurations to default values. It's good practice to
/// do call this procedure after modifying the configuration for a usecase.
SEINLINE void seui_configure_text_input_reset(SE_UI *ctx) {
    ctx->text_input_only_numerical = false;
}

void seui_panel_setup(SEUI_Panel *panel, Rect initial_rect, bool minimised, f32 min_item_height, i32 docked_dir /* = 0*/);

/// Reset the panel configurations to their default values.
/// These are the values that the user can manually set before each widget call
/// to customise their appearance.
SEINLINE void seui_configure_panel_reset(SEUI_Panel *panel) {
    panel->config_row_left_margin = 0;
    panel->config_row_right_margin = 0;
    panel->config_item_centered = false;
}

/// Start a panel at the given position. Aligns the items inside of the panel
/// based on the given number of columns.
/// Returns true if the panel is not closed.
bool seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel);
bool seui_panel(SE_UI *ctx, const char *title);

void seui_panel_row(SE_UI *ctx, f32 height, u32 columns);
Rect seui_panel_put(SE_UI *ctx, f32 min_width, bool expand);

/// call this at the beginning of every frame before creating other widgets
SEINLINE void seui_reset(SE_UI *ctx) {
    ctx->max_id = SEUI_ID_NULL;
    ctx->current_panel = NULL;
    ctx->panel_count = 0;
    ctx->panel_container_count = 0;
}

SEINLINE void seui_resize(SE_UI *ctx, u32 window_w, u32 window_h) {
    ctx->viewport = (Rect) {0, 0, window_w, window_h};
    serender2d_resize(&ctx->renderer, ctx->viewport);
    se_set_text_viewport(&ctx->txt_renderer, ctx->viewport);
}

SEINLINE void seui_init(SE_UI *ctx, SE_Input *input, u32 window_w, u32 window_h) {
    ctx->warm = SEUI_ID_NULL;
    ctx->hot = SEUI_ID_NULL;
    ctx->active = SEUI_ID_NULL;
    seui_reset(ctx);
    ctx->input = input;

    ctx->viewport = (Rect) {0, 0, window_w, window_h};
    serender2d_init(&ctx->renderer, ctx->viewport);
    se_init_text_default(&ctx->txt_renderer, ctx->viewport);

    seui_theme_default(&ctx->theme);

    setexture_atlas_load(&ctx->icon_atlas, "assets/UI/icons/ui_icons_atlas.png", 4, 4);

    /* panels */
    ctx->current_panel = NULL;

    ctx->panel_capacity = 100;
    ctx->panel_count = 0;
    ctx->panels = (SEUI_Panel*) malloc(sizeof(SEUI_Panel) * ctx->panel_capacity);
    memset(ctx->panels, 0, sizeof(SEUI_Panel) * ctx->panel_capacity);

    ctx->panel_container_count = 0;
    memset(ctx->panel_containers, 0, sizeof(u32) * SEUI_PANEL_CONTAINER_CAPACITY);

    sestring_init(&ctx->text_input_cache, "");
    sestring_init(&ctx->text_input, "");
}

SEINLINE void seui_deinit(SE_UI *ctx) {
    serender2d_deinit(&ctx->renderer);
    se_deinit_text(&ctx->txt_renderer);
    sestring_deinit(&ctx->text_input_cache);
    sestring_deinit(&ctx->text_input);
    setexture_atlas_unload(&ctx->icon_atlas);
    free(ctx->panels);
    ctx->panel_count = 0;
}

SEINLINE void seui_close_panel(SE_UI *ctx, u32 panel_index) {
    if (panel_index < ctx->panel_count) ctx->panels[panel_index].is_closed = true;
}

SEINLINE void seui_render(SE_UI *ctx) {
    /* upload data */
    serender2d_upload_to_gpu(&ctx->renderer);

    /* configure */

    /* draw call */
    serender2d_render_uploaded_shapes(&ctx->renderer);
    serender2d_clear_shapes(&ctx->renderer);

    /* text */
    se_render_text(&ctx->txt_renderer);
    se_clear_text_render_queue(&ctx->txt_renderer); // sense we're gonna recreate the queue next frame
}

SEINLINE SEUI_Panel* seui_ctx_get_panel(SE_UI *ctx) {
    if (ctx->panel_count >= ctx->panel_capacity) {
        ctx->panel_capacity += (ctx->panel_capacity+1) * 0.5f;
        ctx->panels = realloc(ctx->panels, sizeof(SEUI_Panel) * ctx->panel_capacity);
        memset(ctx->panels + ctx->panel_count, 0, sizeof(SEUI_Panel) * (ctx->panel_capacity - ctx->panel_count)); // @TODO test this
    }
    u32 panel = ctx->panel_count;
    if (ctx->panels[panel].index == ctx->panel_count) {
        // the indices match so we won't reset this panel, because we want to keep the data from the previous frame
        ctx->panel_count++;
        return &ctx->panels[panel];
    } else {
        // the indices don't match, so this is a different panel and we have to reset it
        Rect init_rect = {
            ctx->viewport.w / 2 - 128, // the hackiest way to get the viewport
            ctx->viewport.h / 2 - 128, // the hackiest way to get the viewport
            128 * 2,
            128 * 2
        };
        seui_panel_setup(&ctx->panels[panel], init_rect, false, 32, 0);
        ctx->panels[panel].index = panel;
        ctx->panels[panel].is_closed = false;
        ctx->panels[panel].depth = 50;
        ctx->panel_count++;
        return &ctx->panels[panel];
    }
}

SEINLINE SEUI_Panel* seui_ctx_get_panel_container(SE_UI *ctx) {
    u32 panel = ctx->panel_containers[ctx->panel_container_count];
    ctx->panel_container_count++;
    if (ctx->panels[panel].is_closed) return NULL;
    return &ctx->panels[panel];
}

#endif // SEUI_H_CTX
