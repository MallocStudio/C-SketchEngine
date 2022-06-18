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
        b8 minimised;
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
        b8 config_item_centered;  // should the current "item" be aligned centered
        b8 config_item_minimised; // should the current "item" take the minimum amount of space

        const char *title;

    /* auto calculated ------------------------------------------------------- */
        i32 index; // the index identifier of the panel tracked by SE_UI, if index is -1 then it's uninitialised?
        b8 is_closed;
            // depth testing
        f32 depth_bg; // depth used for background    items
        f32 depth_fg; // depth used for foreground    items
        f32 depth_mg; // depth used for middle ground items

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
        b8 is_embedded; // is inside of another panel
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

void seui_theme_default(SE_Theme *theme);

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
    i32 panel_containers[SEUI_PANEL_CONTAINER_CAPACITY]; // if the result is negative it means that the container is empty

    u32 panel_capacity;
    u32 panel_count;
    SEUI_Panel *panels;
    f32 current_max_depth; // the highest depth currently assigned (not the maximum available)
    f32 min_depth_available;
    f32 max_depth_available;

    SEUI_Panel *current_panel; // the panel we put the widgets on
    SEUI_Panel *current_non_embedded_panel; // same as current panel but the latest non embedded one
    SEUI_Panel *current_dragging_panel; // the panel that the user is dragging
    SEUI_Panel *latest_activated_panel; // used to determine which panel should be on top of the rest

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
    b8 text_input_only_numerical;
} SE_UI;

/// Set text input configurations to default values. It's good practice to
/// do call this procedure after modifying the configuration for a usecase.
void seui_configure_text_input_reset(SE_UI *ctx);
void seui_panel_setup(SEUI_Panel *panel, Rect initial_rect, b8 minimised, f32 min_item_height, i32 docked_dir /* = 0*/);
void seui_configure_panel_reset(SEUI_Panel *panel);

/// Start a panel at the given position. Aligns the items inside of the panel
/// based on the given number of columns.
/// Returns true if the panel is not closed.
b8 seui_panel_at(SE_UI *ctx, const char *title, SEUI_Panel *panel);
b8 seui_panel(SE_UI *ctx, const char *title);

void seui_panel_row(SE_UI *ctx, f32 height, u32 columns);
Rect seui_panel_put(SE_UI *ctx, f32 min_width);

/// call this at the beginning of every frame before creating other widgets
void seui_reset(SE_UI *ctx);
void seui_resize(SE_UI *ctx, u32 window_w, u32 window_h);
void seui_init(SE_UI *ctx, SE_Input *input, Rect viewport, f32 min_depth, f32 max_depth);
void seui_deinit(SE_UI *ctx);
void seui_close_panel(SE_UI *ctx, u32 panel_index);
void seui_render(SE_UI *ctx);
SEUI_Panel* seui_ctx_get_panel(SE_UI *ctx);
SEUI_Panel* seui_ctx_get_panel_container(SE_UI *ctx);

//// COMPONENTS ////

u32 generate_ui_id(SE_UI *ctx);
UI_STATES get_ui_state (SE_UI *ctx, u32 id, Rect rect, b8 stay_active_on_mouse_leave /* = false */);

void seui_label_at(SE_UI *ctx, const char *text, Rect rect);


/// Draws a button at the given rectangle.
b8 seui_button_at(SE_UI *ctx, const char *text, Rect rect);
b8 seui_button_textured_at(SE_UI *ctx, Vec2 texture_index, Rect rect);

/// Draws a button that returns the drag if the mouse is trying to drag it.
/// That means is the mouse is hovering over the button and pressing down.
Vec2 seui_drag_button_at(SE_UI *ctx, Rect rect, UI_STATES *state);

/// Draws a button that returns the drag if the mouse is trying to drag it.
/// That means is the mouse is hovering over the button and pressing down.
/// If the texture_index is (Vec2) {0} it will draw a simple rectangle.
Vec2 seui_drag_button_textured_at(SE_UI *ctx, Rect rect, Vec2 texture_index, UI_STATES *state);

/// value is clamped between 0 - 1
void seui_slider_at(SE_UI *ctx, Vec2 pos1, Vec2 pos2, f32 *value);

// void seui_colour_picker_at(SE_UI *ctx, Rect rect, RGBA *value);

// RGBA seui_colour_picker_at_hsv(SE_UI *ctx, Rect rect, i32 *h, i32 *s, i32 *v);
// meant to replace seui_colour_picker_at_hsv
// note that panel is added by the user or another button that spawns the colour picker's panel
void seui_hsv_picker(SE_UI *ctx, HSV *hsv);

/// a 2d slider that returns a normalised vec2
void seui_slider2d_at(SE_UI *ctx, Vec2 center, f32 radius, Vec2 *value);

void seui_input_text_at(SE_UI *ctx, SE_String *text, Rect rect);

/// A horizontal slider with buttons on its sides.
/// Within the range of [min, max]
/// If min AND max are zero, the limits will be ignored
b8 seui_selector_at(SE_UI *ctx, Rect rect, i32 *value, i32 min, i32 max);

void seui_texture_viewer(SE_UI *ctx, u32 texture_id); // texture_index refers to the texture added to the renderer
// void seui_image_viewer(SE_UI *ctx, Rect rect, const SE_Texture *texture);

/// ctx current panel must not be null
/// creates a place holder for a panel to be dropped on
void seui_panel_container(SE_UI *ctx);

///
/// WIDGETS INSIDE OF PANELS
///
/// DEFINED IN seui_widgets.c

void seui_label(SE_UI *ctx, const char *text);
void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, b8 editable);
void seui_label_hsv(SE_UI *ctx, const char *title, HSV *value, b8 editable);
/// Draws a button but figures out the position and the rect based on the current
/// context and panel.
b8 seui_button(SE_UI *ctx, const char *text);
b8 seui_button_textured(SE_UI *ctx, Vec2 texture_index);
void seui_slider(SE_UI *ctx, f32 *value);
void seui_slider2d(SE_UI *ctx, Vec2 *value);
void seui_input_text(SE_UI *ctx, SE_String *text);
b8 seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max);

#endif // SEUI_H_CTX