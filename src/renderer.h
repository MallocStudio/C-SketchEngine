// sdl_renderer.h defines primitive rendering functionality, such as rendering text, rectangles,
// circles, etc. It also allows the change of global rendering state, such as current render
// blend mode and current render color. - matink 19 Jan 2022

#ifndef RENDERER_H
#define RENDERER_H

#include "core.h"

/// render text
void render_text(SDL_Renderer *sdl_renderer, Text *text, int x, int y);

/// render text within the given rect
void render_text_at_rect_clipped(SDL_Renderer *sdl_renderer, Text *text, Rect rect);

/// render string within the given rect. if rect's width and height are zero, this function will not confine the text within the rect
void render_string(Renderer *renderer, const char *string, Rect rect, bool wrapped);

/// set sdl_renderer draw color and print out any errors. NOTE that the VALUES must be from 0 - 255
void set_render_draw_color_raw(SDL_Renderer *sdl_renderer, u8 r, u8 g, u8 b, u8 a);

/// set sdl_renderer draw color and print out any errors. Uses RGBA so the values are from 0 - 1
void set_render_draw_color_rgba(SDL_Renderer *sdl_renderer, RGBA rgba);

/// reset sdl_renderer draw color to the default
void reset_render_draw_color(SDL_Renderer *sdl_renderer);

/// renders a filled rectangle using global_app->sdl_renderer and using the current render color
void render_rect_filled(SDL_Renderer *sdl_renderer, Rect rect);

/// renders a filled rectangle using global_app->sdl_renderer using the given color.
/// the render color is set back to whatever it was before this procedure
void render_rect_filled_color(SDL_Renderer *sdl_renderer, Rect rect, RGBA color);

/// render a rectangle border
void render_rect(SDL_Renderer *sdl_renderer, Rect rect);

/// render a rectangle border based on the given color.
void render_rect_color(SDL_Renderer *sdl_renderer, Rect rect, RGBA color);

/// render a filled circle
void render_circle(SDL_Renderer *sdl_renderer, i32 x, i32 y, i32 radius);
#endif // RENDERER_H