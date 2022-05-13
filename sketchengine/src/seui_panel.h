#ifndef SEUI_PANEL_H
#define SEUI_PANEL_H

#include "seui_ctx.h"

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
        Rect rect;
        bool minimised;
        f32 min_item_height;
        // 0 means not docked, 1 means left, 2 means right
        u32 docked_dir;
        // this is the minimum size the user can resize the panel to.
        // note that this is not automatically calculated, so this can be set
        // by the user at any point.
        Vec2 min_size;

        /* layouting configuration */
        f32 config_row_left_margin;
        f32 config_row_right_margin;

    /* auto calculated ------------------------------------------------------- */
        /* next item: (calculated based on layout) */
        // sense some items can be bigger than min row height in the same row,
        // we want all the items on that row to be of the same height
        f32 next_item_height;
        // the relative cursor used to position the placement of the items
        Vec2 cursor;

        /* layouting */
        i32 row_width;
        i32 row_height;

        /* positioning of panel */
        // this size is calculated based on the widgets that populate the panel.
        // this is the minimum size that it takes to fit everything inside of the
        // panel without clipping or scissoring.
        Vec2 fit_size;
        Vec2 fit_cursor;
        Rect cached_rect; // the rect of the panel from the previous frame
        bool is_embedded; // is inside of another panel
} SEUI_Panel;

void seui_panel_setup(SEUI_Panel *panel, Rect initial_rect, Vec2 min_size, bool minimised, f32 min_item_height, i32 docked_dir /* = 0*/);

/// Reset the panel configurations to their default values.
/// These are the values that the user can manually set before each widget call
/// to customise their appearance.
SEINLINE void seui_configure_panel_reset(SEUI_Panel *panel) {
    panel->config_row_left_margin = 0;
    panel->config_row_right_margin = 0;
}

/// Start a panel at the given position. Aligns the items inside of the panel
/// based on the given number of columns.
/// Returns true if the panel is not minimised.
bool seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel_data);
bool seui_panel(SE_UI *ctx, const char *title, SEUI_Panel *panel_data);

void seui_panel_row(SEUI_Panel *panel);
Rect panel_put(SEUI_Panel *panel, f32 min_width, f32 min_height, bool expand);

#endif // SEUI_PANEL_H