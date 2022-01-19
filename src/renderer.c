#include "renderer.h"

/// set sdl_renderer draw color and print out any errors
void set_render_draw_color_raw(SDL_Renderer *sdl_renderer, u8 r, u8 g, u8 b, u8 a) {
    if (SDL_SetRenderDrawColor(sdl_renderer, r, g, b, a) !=0) {
        print_sdl_error();
    }
}

///
void set_render_draw_color_rgba(SDL_Renderer *sdl_renderer, RGBA rgba) {
    set_render_draw_color_raw(sdl_renderer, rgba.r * 255, rgba.g * 255, rgba.b * 255, rgba.a * 255);
}

/// reset sdl_renderer draw color to the default
void reset_render_draw_color(SDL_Renderer *sdl_renderer) {
    if (SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255) !=0) {
        print_sdl_error();
    }
}

/// renders a filled rectangle using global_app->sdl_renderer and using the current render color
void render_rect_filled(SDL_Renderer *sdl_renderer, Rect rect) {
    if (SDL_RenderFillRect(sdl_renderer, &rect) != 0) {
        print_sdl_error();
    }
}

/// renders a filled rectangle using global_app->sdl_renderer using the given color.
/// the render color is set back to whatever it was before this procedure
void render_rect_filled_color(SDL_Renderer *sdl_renderer, Rect rect, RGBA color) {
    u8 prev_color[4];
    if (SDL_GetRenderDrawColor(sdl_renderer, &prev_color[0], &prev_color[1], &prev_color[2], &prev_color[3]) != 0) {
        print_sdl_error();
    }

    set_render_draw_color_rgba(sdl_renderer, color);
    render_rect_filled(sdl_renderer, rect);

    if (SDL_SetRenderDrawColor(sdl_renderer, prev_color[0], prev_color[1], prev_color[2], prev_color[3]) != 0) {
        print_sdl_error();
    }
}

///
void render_text(SDL_Renderer *sdl_renderer, Text *text, int x, int y) {
    Rect text_rect = get_text_rect(text, x, y);
    if (SDL_RenderCopy(sdl_renderer, text->texture, NULL, &text_rect) != 0) {
        print_sdl_error();
    }
}

///
void render_text_rect(SDL_Renderer *sdl_renderer, Text *text, Rect rect) {
    Rect text_rect = get_text_rect(text, rect.x, rect.y);
    // // -- scale down text rect to fit the text inside of the given rect with proper aspect ratio
    if (text_rect.w > rect.w) {
        int diff = text_rect.w - rect.w;
        text_rect.w -= diff;
    }

    if (SDL_RenderCopy(sdl_renderer, text->texture, NULL, &text_rect) != 0) {
        print_sdl_error();
    }
}

/// render string within the given rect. if rect's width and height are zero, this function will not confine the text within the rect
void render_string(SDL_Renderer *sdl_renderer, const char *string, Rect rect) {
    // Text text;
    // @incomplete not implemented yet
}