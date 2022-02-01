#ifndef TEST_H
#define TEST_H

#include "core.h"
#include "renderer.h"

void draw_test_1(SDL_Renderer *sdl_renderer) {
    SDL_Rect rect = {0, 0, 400, 200};
    render_set_draw_color_raw(sdl_renderer, 255, 100, 100, 255);
    if (SDL_RenderDrawRect(sdl_renderer, &rect) != 0) {
        printf("ERROR: %s\n", SDL_GetError());
    }
    render_reset_draw_color(sdl_renderer);
}

#endif //TEST_H