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

/// scissor
void render_text_at_rect_clipped(SDL_Renderer *sdl_renderer, Text *text, Rect rect) {
    Rect srcrect;
    Rect destrect;
    Rect text_rect = get_text_rect(text, rect.x, rect.y);

    srcrect = (Rect) {0, 0, rect.w, text_rect.h};//get_text_rect(text, 0, 0);
    destrect = (Rect) {rect.x, rect.y, srcrect.w, srcrect.h}; // get rid of stretching

    if (SDL_RenderCopy(sdl_renderer, text->texture, &srcrect, &destrect) != 0) {
        print_sdl_error();
    }
}

/// generates the text based on the renderer's glyphs. It wraps the text within the given rect
void generate_text_from_glyphs_rect (Text *result, SDL_Renderer *renderer, Glyphs *glyphs, const char *string, Rect rect) {
    int string_width;
    int string_height;
    Uint32 rmask, gmask, bmask, amask;
    // SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    #endif
    // -- get the size of the text
    TTF_SizeText(glyphs->font, string, &string_width, &string_height);
    // -- create the surface we will generate the text on
    SDL_Surface *result_surface = SDL_CreateRGBSurface(0, rect.w, rect.h, 32, rmask, gmask, bmask, amask);
    int offset_from_prev_glyph_h = 0; // horizontal
    int offset_from_prev_glyph_v = 0; // vertical
    for (int i = 0; i < strlen(string); ++i) { // loop through each letter
        
        // -- calculate where to put this glyph on the destination surface (result_surface)
        Rect dest_rect = {0};
        int minx, maxx, miny, maxy, advance;
        TTF_GlyphMetrics(glyphs->font, string[i], &minx, &maxx, &miny, &maxy, &advance);
        dest_rect.x = offset_from_prev_glyph_h;
        dest_rect.y = offset_from_prev_glyph_v;
        dest_rect.w = advance;
        dest_rect.h = maxy - miny;

        offset_from_prev_glyph_h += dest_rect.w; // advance offset
        if (offset_from_prev_glyph_h >= rect.w) {
            offset_from_prev_glyph_h = 0; // wrap around
            offset_from_prev_glyph_v += string_height; 
        }

        int glyph_index = UNICODE_TO_GLYPH_INDEX((int)string[i]); // get the surface's index
        SDL_BlitSurface(glyphs->glyph[glyph_index], NULL, result_surface, &dest_rect);
    }
    // -- init the text
    init_text_dynamic(result, renderer, string, glyphs->font, glyphs->rgba, result_surface);
}

/// render string within the given rect. if rect's width and height are zero, this function will not confine the text within the rect
void render_string(Renderer *renderer, const char *string, Rect rect, bool wrapped) {
    // @incomplete not implemented yet
    Text text;
    if (wrapped) {
        generate_text_from_glyphs_rect(&text, renderer->sdl_renderer, renderer->glyphs, string, rect);
        render_text(renderer->sdl_renderer, &text, rect.x, rect.y);
    } else {
        generate_text_from_glyphs(&text, renderer->sdl_renderer, renderer->glyphs, string);
        render_text_at_rect_clipped(renderer->sdl_renderer, &text, rect);
    }
    deinit_text(&text);
}