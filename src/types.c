#include "types.h"
#include "assert.h"
#include <memory.h>

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
    text->buffer = (char*) malloc (sizeof(char) * (strlen(data) + 1));
    text->buffer = strcpy(text->buffer, data);
    text->color = color;
    text->renderer = renderer;
    // text->surface = TTF_RenderText_Solid(font, data, rgba_to_sdl_color(&color));
    text->surface = TTF_RenderText_Blended(font, data, rgba_to_sdl_color(&color));
    text->texture = SDL_CreateTextureFromSurface(renderer, text->surface);
}

/// frees text itself
/// frees the memory allocated for text->data
/// frees the generated surface
/// frees the generated texture
void uninit_text(Text *text) {
    SDL_FreeSurface(text->surface);
    SDL_DestroyTexture(text->texture);
    free(text->buffer);
    free(text);
}

///
void set_text_color(Text *text, RGBA color) {
    assert(text != NULL && "init_text: text was null");
    text->color = color;
    // @incomplete @leak check if we get errors if we free surface and destroy texture before doing the following calls
    text->surface = TTF_RenderText_Solid(text->font, text->buffer, rgba_to_sdl_color(&color));
    text->texture = SDL_CreateTextureFromSurface(text->renderer, text->surface);
}

/// returns the rect of the texture based on font.
Rect get_text_rect(Text *text, int pos_x, int pos_y) {
    Rect result;
    TTF_SizeText(text->font, text->buffer, &result.w, &result.h);
    result.x = pos_x;
    result.y = pos_y;
    return result;
}