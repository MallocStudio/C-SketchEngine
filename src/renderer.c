#include "renderer.h"

/// set sdl_renderer draw color and print out any errors
void set_render_draw_color_raw(SDL_Renderer *sdl_renderer, u8 r, u8 g, u8 b, u8 a) {
    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, r, g, b, a), "set_render_draw_color_raw");
}

///
void set_render_draw_color_rgba(SDL_Renderer *sdl_renderer, RGBA rgba) {
    set_render_draw_color_raw(sdl_renderer, rgba.r * 255, rgba.g * 255, rgba.b * 255, rgba.a * 255);
}

/// reset sdl_renderer draw color to the default
void reset_render_draw_color(SDL_Renderer *sdl_renderer) {
    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255), "reset_render_draw_color");
}

/// renders a filled rectangle using global_app->sdl_renderer and using the current render color
void render_rect_filled(SDL_Renderer *sdl_renderer, Rect rect) {
    ERROR_ON_NOTZERO_SDL(SDL_RenderFillRect(sdl_renderer, &rect), "render_rect_filled");
}

/// renders a filled rectangle using global_app->sdl_renderer using the given color.
/// the render color is set back to whatever it was before this procedure
void render_rect_filled_color(SDL_Renderer *sdl_renderer, Rect rect, RGBA color) {
    u8 prev_color[4];
    ERROR_ON_NOTZERO_SDL(SDL_GetRenderDrawColor(sdl_renderer, &prev_color[0], &prev_color[1], &prev_color[2], &prev_color[3]), "render_rect_filled_color");

    set_render_draw_color_rgba(sdl_renderer, color);
    render_rect_filled(sdl_renderer, rect);

    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, prev_color[0], prev_color[1], prev_color[2], prev_color[3]), "render_rect_filled_color");
}

/// render a filled circle
/// https://stackoverflow.com/questions/38334081/howto-draw-circles-arcs-and-vector-graphics-in-sdl
void render_circle_filled(SDL_Renderer *sdl_renderer, i32 centreX, i32 centreY, i32 radius) {
    const i32 diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(sdl_renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(sdl_renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(sdl_renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(sdl_renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(sdl_renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(sdl_renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(sdl_renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(sdl_renderer, centreX - y, centreY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

///
void render_text(SDL_Renderer *sdl_renderer, Text *text, int x, int y) {
    Rect text_rect = get_text_rect(text, x, y);
    ERROR_ON_NOTZERO_SDL(SDL_RenderCopy(sdl_renderer, text->texture, NULL, &text_rect), "render_text");
}

/// scissor
void render_text_at_rect_clipped(SDL_Renderer *sdl_renderer, Text *text, Rect rect) {
    Rect srcrect;
    Rect destrect;
    Rect text_rect = get_text_rect(text, rect.x, rect.y);

    srcrect = (Rect) {0, 0, rect.w, text_rect.h};//get_text_rect(text, 0, 0);
    destrect = (Rect) {rect.x, rect.y, srcrect.w, srcrect.h}; // get rid of stretching

    ERROR_ON_NOTZERO_SDL(SDL_RenderCopy(sdl_renderer, text->texture, &srcrect, &destrect), "render_text_at_rect_clipped");
}

/// generates the text based on the renderer's glyphs. It wraps the text within the given rect
void generate_text_from_glyphs_rect (Text *result, SDL_Renderer *renderer, Glyphs *glyphs, const char *string, Rect rect) {
    // SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
    Uint32 rmask, gmask, bmask, amask;
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
    int string_width;
    int string_height;
    TTF_SizeText(glyphs->font, string, &string_width, &string_height);
    // -- create the surface we will generate the text on
    SDL_Surface *result_surface = SDL_CreateRGBSurface(0, rect.w, rect.h, 32, rmask, gmask, bmask, amask);
    ERROR_ON_NULL_SDL(result_surface, "generate_text_from_glyphs_rect");
    
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
        if (offset_from_prev_glyph_h + dest_rect.w  >= rect.w) {
            offset_from_prev_glyph_h = 0; // wrap around
            offset_from_prev_glyph_v += string_height; 
        }
        if (dest_rect.y + dest_rect.h > rect.h) break; // don't render past rect.h

        int glyph_index = UNICODE_TO_GLYPH_INDEX((int)string[i]); // get the surface's index
        ERROR_ON_NOTZERO_SDL(SDL_BlitSurface(
            glyphs->glyph[glyph_index], 
            NULL, // NULL so that the whole surface of the glyph is copied
            result_surface, 
            &dest_rect), "generate_text_from_glyphs_rect");
    }
    // -- init the text
    init_text_dynamic(result, renderer, string, glyphs->font, glyphs->rgba, result_surface);
}

/// render string within the given rect. if rect's width and height are zero, this function will not confine the text within the rect
void render_string(Renderer *renderer, const char *string, Rect rect, bool wrapped) {
    // @incomplete if rect dimension's zero, sdl will fail. Come up with something to deal with that universally
    if (rect.h <= 0) return;
    if (rect.w <= 0) return;
    Text text;
    if (wrapped) {
        // generate_text_from_glyphs(&text, renderer->sdl_renderer, renderer->glyphs, string);
        generate_text_from_glyphs_rect(&text, renderer->sdl_renderer, renderer->glyphs, string, rect);
        ERROR_ON_NOTZERO_SDL(SDL_RenderCopy(renderer->sdl_renderer, text.texture, NULL, &rect), "render_text");
    } else {
        generate_text_from_glyphs(&text, renderer->sdl_renderer, renderer->glyphs, string);
        render_text_at_rect_clipped(renderer->sdl_renderer, &text, rect);
    }
    deinit_text(&text);
}

/// render a rectangle border
void render_rect(SDL_Renderer *sdl_renderer, Rect rect) {
    ERROR_ON_NOTZERO_SDL(
        SDL_RenderDrawRect(sdl_renderer, &rect),
        "render_rect");
}

/// render a rectangle border
void render_rect_color(SDL_Renderer *sdl_renderer, Rect rect, RGBA color) {
    u8 prev_color[4];
    ERROR_ON_NOTZERO_SDL(SDL_GetRenderDrawColor(sdl_renderer, &prev_color[0], &prev_color[1], &prev_color[2], &prev_color[3]), "render_rect_filled_color");

    set_render_draw_color_rgba(sdl_renderer, color);
    render_rect(sdl_renderer, rect);

    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, prev_color[0], prev_color[1], prev_color[2], prev_color[3]), "render_rect_filled_color");
}