#ifndef SEUI_H
#define SEUI_H

#include "defines.h"
#include "serenderer.h"
#include "setext.h"
#include "semath.h"
#include "seinput.h"

typedef struct UI_Theme {
    RGB color_primary;
    RGB color_secondary;
    // -- the reset is generated from the primary and secondary values during init
    RGB color_interactive_normal;
    RGB color_interactive_hot;
    RGB color_interactive_active;
    RGB color_panel_base;
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
    SEGL_Renderer2D renderer;
    // the text renderer
    SE_Text_Renderer txt_renderer;
    
    // -- input
    SE_Input *input; // ! not owned
    Vec2 mouse_grab_offset;

    UI_Theme *theme;
    UI_ID current_max_id; // UI_ID_NULL means none. At begin time, this value should be UI_ID_NULL
    UI_CONTEXT_STATES context_state;

    // -- related to rendering other widgets (positioning)
    Rect window_rect;
    Rect view_rect;

    // defines the minimum rect used
    // also the x-y coord define the width and height but going to the left
    // so by default they are set to zero
    Rect min_rect;
    // the previous frame's min_rect.
    Rect min_rect_prev_frame;
    
    Rect prev_item_rect; // the rect of the previously 'put' item
    i32 at_y;
    i32 at_x;
    i32 at_w;
    i32 at_h;

    // -- describes the layout. this is used to define the place of the next item
    i32 x_advance_by;
    i32 y_advance_by;
} UI_Context;
/// inits context. creates a default theme
void ui_init_context(UI_Context *ctx, SE_Input *input);
/// deinitialises context and frees relevant memory
void ui_deinit_context(UI_Context *ctx);
/// frees current theme and replaces it with the provided one
void ui_context_set_theme(UI_Context *ctx, UI_Theme *theme);
/// @TODO // @document
void ui_update_context(UI_Context *ctx, Rect viewport);
/// render everything and clear frame
void ui_render(UI_Context *ctx);

/// advances the at_* coordinates based on current layout
/// can be used to leave an item's space empty
/// this is also used internally when items are called
void ui_put(UI_Context *ctx);

/// render a panel for other widgets. returns true if it's been successfully
/// drawn. if the number_of_items is greater than zero, the items will be expanded
void ui_begin(UI_Context *ctx, Rect *rect);

/// start a new row
void ui_row(UI_Context *ctx, i32 number_of_items, i32 height, i32 min_width);

/// puts a margin between the previous element and the next, based on
/// the direction of the layout (might put a margin within a row, or between rows)
void ui_margin(UI_Context *ctx, i32 amount); // @incomplete yet to be implemented

/// render a button using context. returns true if the button is pressed
bool ui_button(UI_Context *ctx, const char *string);

/// returns true while the button is pressed down
bool ui_button_grab(UI_Context *ctx, Rect rect);

// /// render a label using context.
void ui_label(UI_Context *ctx, const char *title);

#endif // SEUI_H