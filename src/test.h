#ifndef TEST_H
#define TEST_H

#include "core.h"
#include "renderer.h"

void draw_test_1(SDL_Renderer *renderer) {
    SDL_Rect rect = {0, 0, 400, 200};
    set_render_draw_color_raw(renderer, 255, 100, 100, 255);
    if (SDL_RenderDrawRect(renderer, &rect) != 0) {
        printf("ERROR: %s\n", SDL_GetError());
    }
    reset_render_draw_color(renderer);
}

#endif //TEST_H