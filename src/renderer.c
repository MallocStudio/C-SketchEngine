#include "renderer.h"

/// set renderer draw color and print out any errors
void set_render_draw_color_raw(SDL_Renderer *renderer, u8 r, u8 g, u8 b, u8 a) {
    if (SDL_SetRenderDrawColor(renderer, r, g, b, a) !=0) {
        print_sdl_error();
    }
}

///
void set_render_draw_color_rgba(SDL_Renderer *renderer, RGBA *rgba) {
    set_render_draw_color_raw(renderer, rgba->r * 255, rgba->g * 255, rgba->b * 255, rgba->a * 255);
}

/// reset renderer draw color to the default
void reset_render_draw_color(SDL_Renderer *renderer) {
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) !=0) {
        print_sdl_error();
    }
}

/// renders a filled rectangle using global_app->renderer and using the current render color
void render_rect_filled(SDL_Renderer *renderer, Rect *rect) {
    if (SDL_RenderFillRect(renderer, rect) != 0) {
        print_sdl_error();
    }
}

/// renders a filled rectangle using global_app->renderer using the given color.
/// the render color is set back to whatever it was before this procedure
void render_rect_filled_color(SDL_Renderer *renderer, Rect *rect, RGBA *color) {
    u8 prev_color[4];
    if (SDL_GetRenderDrawColor(renderer, &prev_color[0], &prev_color[1], &prev_color[2], &prev_color[3]) != 0) {
        print_sdl_error();
    }

    set_render_draw_color_rgba(renderer, color);
    render_rect_filled(renderer, rect);

    if (SDL_SetRenderDrawColor(renderer, prev_color[0], prev_color[1], prev_color[2], prev_color[3]) != 0) {
        print_sdl_error();
    }
}