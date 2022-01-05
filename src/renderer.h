#ifndef RENDERER_H
#define RENDERER_H

#include "core.h"

/// set renderer draw color and print out any errors. NOTE that the VALUES must be from 0 - 255
void set_render_draw_color_raw(SDL_Renderer *renderer, u8 r, u8 g, u8 b, u8 a);
/// set renderer draw color and print out any errors. Uses RGBA so the values are from 0 - 1
void set_render_draw_color_rgba(SDL_Renderer *renderer, RGBA *rgba);
/// reset renderer draw color to the default
void reset_render_draw_color(SDL_Renderer *renderer);
/// renders a filled rectangle using global_app->renderer and using the current render color
void render_rect_filled(SDL_Renderer *renderer, Rect *rect);
/// renders a filled rectangle using global_app->renderer using the given color.
/// the render color is set back to whatever it was before this procedure
void render_rect_filled_color(SDL_Renderer *renderer, Rect *rect, RGBA *color);


#endif // RENDERER_H