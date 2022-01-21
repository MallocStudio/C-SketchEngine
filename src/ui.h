#ifndef UI_H
#define UI_H

#include "core.h"

typedef struct UI_Theme {
    RGBA color_primary;
    RGBA color_secondary;
    // -- the reset is generated from the primary and secondary values during init
    RGBA color_interactive_normal;
    RGBA color_interactive_hot;
    RGBA color_interactive_active;
    RGBA color_panel_base;
    f32  color_transition_amount;
    Rect padding; // inside padding
} UI_Theme; void ui_init_theme (UI_Theme *theme);

typedef enum UI_CONTEXT_STATES {
    UI_CS_NORMAL, UI_CS_MAXIMISED, UI_CS_MINIMISED,
} UI_CONTEXT_STATES;

typedef u32 UI_ID;
#define UI_ID_NULL 0

typedef struct UI_Context {
    // -- to be compare against id (internally)
    UI_ID hot;    // the item is about to be interacted with (eg mouse over)
    UI_ID active; // the currently active item

    // the renderer to use for rendering
    Renderer *renderer;
    // the mouse position
    Vec2i mouse_pos;
    // the position the mouse was initially pressed. updated in ui_update_context
    Vec2i mouse_pressed_pos;
    //
    Vec2i mouse_grab_offset;
    // information from the current frame
    bool  is_mouse_left_pressed;
    bool  is_mouse_right_pressed;
    // information from the previous frame
    bool  was_mouse_left_pressed;
    bool  was_mouse_right_pressed;
    UI_Theme *theme;
    UI_ID current_max_id; // UI_ID_NULL means none. At begin time, this value should be UI_ID_NULL
    UI_CONTEXT_STATES context_state;

    // -- related to rendering other widgets (positioning)
    Rect window_rect;
    Rect view_rect;
    Rect used_rect;
    Rect prev_item_rect; // the rect of the previously 'put' item
    i32 at_y;
    i32 at_x;
    i32 at_w;
    i32 at_h;

    // -- describes the layout. this is used to define the place of the next item
    i32 x_advance_by;
    i32 y_advance_by;
} UI_Context;
void ui_init_context(UI_Context *ctx, Renderer *renderer);
void ui_deinit_context(UI_Context *ctx);
void ui_context_set_theme(UI_Context *ctx, UI_Theme *theme);
void ui_update_context(UI_Context *context);

/// advances the at_* coordinates based on current layout
/// can be used to leave an item's space empty
/// this is also used internally when items are called
void ui_put(UI_Context *ctx);

/// render a panel for other widgets. returns true if it's been successfully
/// drawn. if the number_of_items is greater than zero, the items will be expanded
void ui_begin(UI_Context *ctx, Rect *rect);

/// start a new row
void ui_row(UI_Context *ctx, i32 number_of_items, i32 height);

/// puts a margin between the previous element and the next, based on
/// the direction of the layout (might put a margin within a row, or between rows)
void ui_margin(UI_Context *ctx, i32 amount); // @incomplete yet to be implemented

/// render a button using context. returns true if the button is pressed
bool ui_button(UI_Context *ctx, const char *string);

/// returns true while the button is pressed down
bool ui_button_grab(UI_Context *ctx, Rect rect);

// /// render a label using context.
void ui_label(UI_Context *ctx, const char *title);

#endif // UI_H