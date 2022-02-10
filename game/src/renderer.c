#include "renderer.h"
#include <assert.h>

/// -------- ///
/// INTERNAL ///
/// -------- ///


/// ------ ///
/// PUBLIC ///
/// ------ ///

/// set sdl_renderer draw color and print out any errors
void render_set_draw_color_raw(SDL_Renderer *sdl_renderer, u8 r, u8 g, u8 b, u8 a) {
    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, r, g, b, a), "render_set_draw_color_raw");
}

///
void render_set_draw_color_rgba(SDL_Renderer *sdl_renderer, RGBA rgba) {
    render_set_draw_color_raw(sdl_renderer, rgba.r * 255, rgba.g * 255, rgba.b * 255, rgba.a * 255);
}

/// reset sdl_renderer draw color to the default
void render_reset_draw_color(SDL_Renderer *sdl_renderer) {
    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255), "render_reset_draw_color");
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

    render_set_draw_color_rgba(sdl_renderer, color);
    render_rect_filled(sdl_renderer, rect);

    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, prev_color[0], prev_color[1], prev_color[2], prev_color[3]), "render_rect_filled_color");
}

/// render a filled circle
/// https://stackoverflow.com/questions/38334081/howto-draw-circles-arcs-and-vector-graphics-in-sdl
void render_circle(SDL_Renderer *sdl_renderer, i32 centreX, i32 centreY, i32 radius) {
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
    
void render_string(Renderer *renderer, const char *string, Rect rect, u32 style_flags) {
    if (rect.h <= 0) return;
    if (rect.w <= 0) return;
    Text text;

    generate_text_from_glyphs(&text, renderer->sdl_renderer, renderer->glyphs, string, rect, style_flags);
    ERROR_ON_NOTZERO_SDL(SDL_RenderCopy(renderer->sdl_renderer, text.texture, NULL, &rect), "render_text");
    // if (style_flags & STRING_STYLE_ALIGN_CENTER) {
    //     // generate_text_from_glyphs(&text, renderer->sdl_renderer, renderer->glyphs, string);
    // } else {
    //     // generate_text_from_glyphs(&text, renderer->sdl_renderer, renderer->glyphs, string);
    //     // render_text_at_rect_clipped(renderer->sdl_renderer, &text, rect);
    // }
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

    render_set_draw_color_rgba(sdl_renderer, color);
    render_rect(sdl_renderer, rect);

    ERROR_ON_NOTZERO_SDL(SDL_SetRenderDrawColor(sdl_renderer, prev_color[0], prev_color[1], prev_color[2], prev_color[3]), "render_rect_filled_color");
}

void render_glyphs_onto_surface (SDL_Surface *result_surface, Glyphs *glyphs, const char *string, Rect rect) {
    i32 string_height;
    i32 string_width;
    TTF_SizeText(glyphs->font, string, &string_width, &string_height);
    f32 pad_amount = 0.125f; // to center
    Rect text_box_padded = {                        // rect but padded
        rect.x,// rect.x + rect.w * pad_amount,
        rect.y,
        rect.w,//rect.w - rect.w * pad_amount,
        rect.h
    };
    i32 x_offset = text_box_padded.x;
    i32 y_offset = text_box_padded.y;

    // -- arrange the letters within the text box
    i32 string_len = SDL_strlen(string);
    for (int i = 0; i < string_len; ++i) { // loop through each letter
        // calculate where to put this glyph on the destination surface (result_surface)
        Rect dest_rect = {0};
        int minx, maxx, miny, maxy, advance;
        TTF_GlyphMetrics(glyphs->font, string[i], &minx, &maxx, &miny, &maxy, &advance);

        dest_rect.x = x_offset;
        dest_rect.y = y_offset;
        dest_rect.w = advance;
        dest_rect.h = maxy - miny;

        x_offset += dest_rect.w; // advance offset

        if (x_offset + dest_rect.w  >= text_box_padded.w) {
            x_offset = text_box_padded.x; // wrap around
            y_offset += string_height;
        }
        // if (dest_rect.y + dest_rect.h > text_box_padded.h) break; // break out early for efficiency

        int glyph_index = UNICODE_TO_GLYPH_INDEX((int)string[i]); // get the surface's index
        ERROR_ON_NOTZERO_SDL(SDL_BlitSurface(
            glyphs->glyph[glyph_index], 
            NULL, // NULL so that the whole surface of the glyph is copied
            result_surface, 
            &dest_rect), "render_glyphs_onto_surface");
    }
}

void render_line(SDL_Renderer *sdl_renderer, i32 x1, i32 y1, i32 x2, i32 y2) {
    SDL_RenderDrawLine(sdl_renderer, x1, y1, x2, y2);
}

void render_cross(SDL_Renderer *sdl_renderer, i32 x, i32 y, i32 size) {
    size *= 0.5f;
    i32 x1, y1, x2, y2;
    x1 = x - size;
    y1 = y - size;
    x2 = x + size;
    y2 = y + size;
    render_line(sdl_renderer, x1, y1, x2, y2);
    x1 = x - size;
    y1 = y + size;
    x2 = x + size;
    y2 = y - size;
    render_line(sdl_renderer, x1, y1, x2, y2);
}

void render_grid(SDL_Renderer *sdl_renderer, i32 x, i32 y, i32 w, i32 h, i32 size_of_cell) {
    i32 num_of_cols = w / size_of_cell + 1;
    i32 num_of_rows = h / size_of_cell + 1;
    // -- columns
    for (i32 i = 0; i < num_of_cols; ++i) {
        i32 x1, y1, x2, y2;
        x1 = x + i * size_of_cell;
        x2 = x1;
        y1 = y;
        y2 = y + num_of_cols * size_of_cell;
        render_line(sdl_renderer, x1, y1, x2, y2);
    }
    // -- rows
    for (i32 i = 0; i < num_of_rows; ++i) {
        i32 x1, y1, x2, y2;
        x1 = x;
        x2 = x + num_of_rows * size_of_cell;
        y1 = y + i * size_of_cell;
        y2 = y1;
        render_line(sdl_renderer, x1, y1, x2, y2);
    }
}

void render_infinite_grid(SDL_Renderer *sdl_renderer, i32 x, i32 y, i32 w, i32 h) {
    // @incomplete
}