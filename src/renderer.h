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

/// render string within the given rect.
/// this procedure will return if the width or height of the rect is <= zero
/// Use the flags to style and position the text within the rect.
#define STRING_STYLE_NONE          (0)
#define STRING_STYLE_ALIGN_LEFT    (1 << 0)
#define STRING_STYLE_ALIGN_RIGHT   (1 << 1)
#define STRING_STYLE_ALIGN_CENTER  (1 << 2)
#define STRING_STYLE_VALIGN_CENTER (1 << 3)
void render_string(Renderer *renderer, const char *string, Rect rect, u32 style_flags);

/// set sdl_renderer draw color and print out any errors. NOTE that the VALUES must be from 0 - 255
void render_set_draw_color_raw(SDL_Renderer *sdl_renderer, u8 r, u8 g, u8 b, u8 a);

/// set sdl_renderer draw color and print out any errors. Uses RGBA so the values are from 0 - 1
void render_set_draw_color_rgba(SDL_Renderer *sdl_renderer, RGBA rgba);

/// reset sdl_renderer draw color to the default
void render_reset_draw_color(SDL_Renderer *sdl_renderer);

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

/// generates the text based on the renderer's glyphs. It wraps the text within the given rect
/// wraps the text if string_width is greater than rect.
/// but the text will only be visible within the boundaries of rect
/// "rect" refers to an area in the result_surface
void render_glyphs_onto_surface (SDL_Surface *result_surface, Glyphs *glyphs, const char *string, Rect rect);

/// render a line from {x1, y1} to {x2, y2}
void render_line(SDL_Renderer *sdl_renderer, i32 x1, i32 y1, i32 x2, i32 y2);

/// render a cross at {x, y} with the given size
void render_cross(SDL_Renderer *sdl_renderer, i32 x, i32 y, i32 size);

/// render a grid at {x, y} with {cols, rows}
void render_grid(SDL_Renderer *sdl_renderer, i32 x, i32 y, i32 cols, i32 rows);
#endif // RENDERER_H