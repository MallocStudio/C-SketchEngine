#include "types.h"
#include "assert.h"
#include <memory.h>
#include "core.h"
#include "renderer.h"

///
SDL_Color rgba_to_sdl_color(const RGBA *rgba) {
    SDL_Color color;
    color.r = rgba->r * 255;
    color.g = rgba->g * 255;
    color.b = rgba->b * 255;
    color.a = rgba->a * 255;
    return color;
}

/// text should not be null
/// stores a pointer to the renderer and font for future use
/// allocates memory based on data
/// copies RGBA color to text->color
void init_text(Text *text, SDL_Renderer *renderer, const char *data, TTF_Font *font, RGBA color) {
    assert(text != NULL && "init_text: text was null");
    text->font = font;
    text->dynamic = false;
    text->buffer = (char*) malloc (sizeof(char) * (SDL_strlen(data) + 1));
    // text->buffer = 
    SDL_strlcpy(text->buffer, data, SDL_strlen(data));//strcpy(text->buffer, data);
    text->color = color;
    text->renderer = renderer;
    text->surface = TTF_RenderText_Blended(font, data, rgba_to_sdl_color(&color));
    text->texture = SDL_CreateTextureFromSurface(renderer, text->surface);
}
/// init a dynamic text
void init_text_dynamic(Text *text, SDL_Renderer *renderer, const char *data, TTF_Font *font, RGBA color, SDL_Surface *surface) {
    assert(text != NULL && "init_text: text was null");
    text->font = font;
    text->dynamic = true;
    text->buffer = (char*) malloc (sizeof(char) * (strlen(data) + 1));
    SDL_strlcpy(text->buffer, data, SDL_strlen(data));
    text->color = color;
    text->renderer = renderer;
    text->surface = surface;
    text->texture = SDL_CreateTextureFromSurface(renderer, text->surface);
    ERROR_ON_NULL_SDL(text->texture, "init_text_dynamic");
}

/// frees text itself
/// frees the memory allocated for text->data
/// frees the generated surface
/// frees the generated texture
void deinit_text(Text *text) {
    SDL_FreeSurface(text->surface);
    SDL_DestroyTexture(text->texture);
    free(text->buffer);
}

/// @slow this is an extremely slow operation
void set_text_color(Text *text, RGBA color) {
    assert(text != NULL && "init_text: text was null");
    text->color = color;
    SDL_FreeSurface(text->surface);
    SDL_DestroyTexture(text->texture);
    text->surface = TTF_RenderText_Solid(text->font, text->buffer, rgba_to_sdl_color(&color));
    ERROR_ON_NULL_SDL(text->surface, "set_text_color");
    text->texture = SDL_CreateTextureFromSurface(text->renderer, text->surface);
    ERROR_ON_NULL_SDL(text->texture, "set_text_color");
}

/// returns the rect of the texture based on font.
Rect get_text_rect(Text *text, int pos_x, int pos_y) {
    Rect result;
    TTF_SizeText(text->font, text->buffer, &result.w, &result.h);
    result.x = pos_x;
    result.y = pos_y;
    return result;
}

/// generate all the glyph textures based on the font
void init_glyphs (SDL_Renderer *renderer, Glyphs *glyphs, TTF_Font* font, RGBA rgba) {
    for (int i = 0; i < GLYPHS_AMOUNT; ++i) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(font, (char)(GLYPHS_UNCODE_MIN + i), (SDL_Color) {rgba.r * 255, rgba.g * 255, rgba.b * 255, rgba.a * 255});
        glyphs->glyph[i] = surface;
        // glyphs->glyph[i] = SDL_CreateTextureFromSurface(renderer, surface);
        // SDL_FreeSurface(surface);
    }
    glyphs->font = font;
}

/// free all the textures generated by the glyphs
void deinit_glyphs (Glyphs *glyphs) {
    for (int i = 0; i < GLYPHS_AMOUNT; ++i) {
        // SDL_DestroyTexture(glyphs->glyph[i]);
        SDL_FreeSurface(glyphs->glyph[i]);
    }
    glyphs->font = NULL;
}

/// inits the given text based on the given glyphs
void generate_text_from_glyphs (Text *result, SDL_Renderer *renderer, Glyphs *glyphs, const char *string, Rect rect, u32 style_flags) { // @incomplete we need to optimise text regenration in general. It may not apply to this procedure directly, but we should only regenrate a given text's texture if it needs regenerating. Otherwise we should use the old texture
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
    // -- create the surface we will generate the text on
    SDL_Surface *result_surface = SDL_CreateRGBSurface(0, rect.w, rect.h, 32, rmask, gmask, bmask, amask);
    ERROR_ON_NULL_SDL(result_surface, "generate_text_from_glyphs");

    if (style_flags & STRING_STYLE_ALIGN_CENTER) {
        // i32 string_height;
        // i32 string_width;
        // TTF_SizeText(glyphs->font, string, &string_width, &string_height);

        // // -- find the number of lines
        // i32 number_of_lines = 0;
        // i32 x_offset = 0;
        // i32 y_offset = 0;
        // i32 string_index_begin = 0;
        // i32 string_index_end   = 0;
        // for (i32 i = 0; i < strlen(string); ++i) {
        //     Rect letter_size = {0}; // used to determine the size of each letter
        //     i32 minx, maxx, miny, maxy, advance;
        //     TTF_GlyphMetrics(glyphs->font, string[i], &minx, &maxx, &miny, &maxy, &advance);

        //     letter_size.x = x_offset;
        //     letter_size.y = y_offset;
        //     letter_size.w = advance;
        //     letter_size.h = maxy - miny;
            
        //     x_offset += letter_size.w; // advance offset
        //     if (x_offset + letter_size.w  >= rect.w) { // we've reached the end of this line
        //         // create a sub text and copy from string to line_text
        //         u32 line_text_len = string_index_end - string_index_begin;
        //         char line_text[line_text_len + 1];
        //         for (i32 letter_index = 0; letter_index < line_text_len; ++letter_index) {
        //             line_text[letter_index] = string[string_index_begin + letter_index];
        //         }
        //         line_text[line_text_len] = '\0'; // end the sub text with a null terminator

        //         // get the size of the line
        //         i32 line_text_width, line_text_height;
        //         TTF_SizeText(glyphs->font, line_text, &line_text_width, &line_text_height);
        //         if (x_offset < rect.w) {
        //             x_offset = (rect.w - line_text_width) * 0.5f; // center the line if line_width is less than rect.w
        //         }
        //         // generate the glyphs for this line
        //         render_glyphs_onto_surface(result_surface, glyphs, line_text, (Rect) {
        //             x_offset, y_offset, rect.w, rect.h
        //         });

        //         // reset offsets and go to the next line
        //         string_index_begin = string_index_end;
        //         x_offset = 0;
        //         y_offset += string_height;

        //         // after rendering the line, go to the next line
        //         number_of_lines++;
        //     }
        //     // if (letter_size.y + letter_size.h > rect.h) break; // break out early for efficiency
        //     string_index_end++; // increase index end
        // }

        render_glyphs_onto_surface(result_surface, glyphs, string, (Rect) {0, 0, result_surface->w, result_surface->h});
    } else {
        // render_glyphs_onto_surface(result_surface, glyphs, string, rect);
        render_glyphs_onto_surface(result_surface, glyphs, string, (Rect) {0, 0, result_surface->w, result_surface->h});
    }
    // int offset_from_prev_glyph = 0;
    // for (int i = 0; i < strlen(string); ++i) {
    //     // -- calculate where to put this glyph on the destination surface (result_surface)
    //     Rect dest_rect = {0};
    //     int minx, maxx, miny, maxy, advance;
    //     TTF_GlyphMetrics(glyphs->font, string[i], &minx, &maxx, &miny, &maxy, &advance);

    //     dest_rect.x = offset_from_prev_glyph;
    //     dest_rect.y = 0;
    //     dest_rect.w = advance;
    //     dest_rect.h = maxy - miny;
        
    //     offset_from_prev_glyph += dest_rect.w; // advance offset

    //     int glyph_index = UNICODE_TO_GLYPH_INDEX((int)string[i]);
    //     SDL_BlitSurface(glyphs->glyph[glyph_index], NULL, result_surface, &dest_rect);
    // }
    
    // -- init the text
    init_text_dynamic(result, renderer, string, glyphs->font, glyphs->rgba, result_surface);
}

///
void init_renderer (Renderer *renderer) {
    renderer->glyphs = new(Glyphs);

    // -- @temp
    TTF_Font *font = TTF_OpenFont(DEFAULT_FONT_PATH, 24);
    if (font == NULL) {
        printf("Error: could not load font at %s\n", DEFAULT_FONT_PATH);
        print_ttf_error();
    }

    // -- glyphs test
    init_glyphs(renderer->sdl_renderer, renderer->glyphs, font, (RGBA){1, 1, 1, 1});
}

void deinit_renderer (Renderer *renderer) {
    deinit_glyphs(renderer->glyphs);
    free(renderer->glyphs);
}