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
    text->data = (char*) malloc (sizeof(char) * (strlen(data) + 1));
    text->data = strcpy(text->data, data);
    text->color = color;
    text->renderer = renderer;
    text->surface = TTF_RenderText_Solid(font, data, rgba_to_sdl_color(&color)); // @leak @incomplete need to free the surface afterwards
    text->texture = SDL_CreateTextureFromSurface(renderer, text->surface);
}

/// frees text itself
/// frees the memory allocated for text->data
/// frees the generated surface
/// frees the generated texture
void uninit_text(Text *text) {
    SDL_FreeSurface(text->surface);
    SDL_DestroyTexture(text->texture);
    free(text->data);
    free(text);
}

///
void set_text_color(Text *text, RGBA color) {
    assert(text != NULL && "init_text: text was null");
    text->color = color;
    text->surface = TTF_RenderText_Solid(text->font, text->data, rgba_to_sdl_color(&color));
    text->texture = SDL_CreateTextureFromSurface(text->renderer, text->surface);
}