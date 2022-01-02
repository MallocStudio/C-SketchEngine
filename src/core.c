#include "core.h"

/// debugging for SDL2
void print_sdl_error() {
    printf("ERROR: %s\n", SDL_GetError());
}
/// debugging for ttf SDL2
void print_ttf_error() {
    printf("ERROR: %s\n", TTF_GetError());
}