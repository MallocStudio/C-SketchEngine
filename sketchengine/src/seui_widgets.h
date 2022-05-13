#ifndef SEUI_WIDGETS_H
#define SEUI_WIDGETS_H

#include "seui_components.h"

void seui_label(SE_UI *ctx, const char *text);
void seui_label_vec3(SE_UI *ctx, const char *title, Vec3 *value, bool editable);
/// Draws a button but figures out the position and the rect based on the current
/// context and panel.
bool seui_button(SE_UI *ctx, const char *text);
bool seui_button_textured(SE_UI *ctx, Vec2 texture_index);
void seui_slider(SE_UI *ctx, f32 *value);
void seui_colour_picker(SE_UI *ctx, RGBA *value);
RGBA seui_colour_picker_hsv(SE_UI *ctx, i32 *h, i32 *s, i32 *v);
void seui_slider2d(SE_UI *ctx, Vec2 *value);
void seui_input_text(SE_UI *ctx, SE_String *text);
bool seui_selector(SE_UI *ctx, i32 *value, i32 min, i32 max);

#endif // SEUI_WIDGETS_H