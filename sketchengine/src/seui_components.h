#ifndef SEUI_COMPONENETS_H
#define SEUI_COMPONENETS_H

#include "seui_panel.h"

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

void seui_colour_picker_at(SE_UI *ctx, Rect rect, RGBA *value);

RGBA seui_colour_picker_at_hsv(SE_UI *ctx, Rect rect, i32 *h, i32 *s, i32 *v);

/// a 2d slider that returns a normalised vec2
void seui_slider2d_at(SE_UI *ctx, Vec2 center, f32 radius, Vec2 *value);

void seui_input_text_at(SE_UI *ctx, SE_String *text, Rect rect);

/// A horizontal slider with buttons on its sides.
/// Within the range of [min, max]
/// If min AND max are zero, the limits will be ignored
bool seui_selector_at(SE_UI *ctx, Rect rect, i32 *value, i32 min, i32 max);

#endif SEUI_COMPONENETS_H