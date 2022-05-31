#ifndef SEUI_COMPONENETS_H
#define SEUI_COMPONENETS_H

#include "seui_ctx.h"

u32 generate_ui_id(SE_UI *ctx);
UI_STATES get_ui_state (SE_UI *ctx, u32 id, Rect rect, bool stay_active_on_mouse_leave /* = false */);

void seui_label_at(SE_UI *ctx, const char *text, Rect rect);


/// Draws a button at the given rectangle.
bool seui_button_at(SE_UI *ctx, const char *text, Rect rect);

bool seui_button_textured_at(SE_UI *ctx, Vec2 texture_index, Rect rect);

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
bool seui_selector_at(SE_UI *ctx, Rect rect, i32 *value, i32 min, i32 max);

void seui_texture_viewer(SE_UI *ctx, u32 texture_index); // texture_index refers to the texture added to the renderer
// void seui_image_viewer(SE_UI *ctx, Rect rect, const SE_Texture *texture);

/// ctx current panel must not be null
/// creates a place holder for a panel to be dropped on
void seui_panel_container(SE_UI *ctx);

///
/// WIDGETS INSIDE OF PANELS
///
/// DEFINED IN seui_widgets.c

void seui_label(SE_UI *ctx, const char *text);
void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, bool editable);
void seui_label_hsv(SE_UI *ctx, const char *title, HSV *value, bool editable);
/// Draws a button but figures out the position and the rect based on the current
/// context and panel.
bool seui_button(SE_UI *ctx, const char *text);
bool seui_button_textured(SE_UI *ctx, Vec2 texture_index);
void seui_slider(SE_UI *ctx, f32 *value);
void seui_slider2d(SE_UI *ctx, Vec2 *value);
void seui_input_text(SE_UI *ctx, SE_String *text);
bool seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max);

#endif SEUI_COMPONENETS_H