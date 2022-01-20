#include "core.h"
#include "renderer.h"
#include "ui.h"

/// init SDL stuff
void init_sdl(App *app) {
    app->renderer = new(Renderer);
    // -- SDL
    ERROR_ON_NOTZERO_SDL(SDL_Init(SDL_INIT_EVERYTHING), "init_sdl");

    // -- Init window
    app->window = SDL_CreateWindow("My C Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, app->window_width, app->window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ERROR_ON_NULL_SDL(app->window, "init_sdl");

    // -- sdl_renderer
    app->renderer->sdl_renderer = SDL_CreateRenderer(app->window, 0, SDL_RENDERER_ACCELERATED);
    ERROR_ON_NULL_SDL(app->renderer->sdl_renderer, "init_sdl");
    SDL_RenderSetLogicalSize(app->renderer->sdl_renderer, app->window_width, app->window_height);
}

/// uninit SDL
void deinit_sdl(App *app) {
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer->sdl_renderer);
    SDL_Quit();
}

/// init globals defined in core.h
void init_globals() {
    // global_app = app;
    delta_time = 0.f;
}

/// init globals defined in core.h
void deinit_globals() {
}

void init_app   (App *app) {
    // -- window size
    app->window_width  = 1000;
    app->window_height = 800;

    // -- Init SDL
    init_sdl(app);
    reset_render_draw_color(app->renderer->sdl_renderer);
    
    // -- Init TTF
    if (TTF_Init() != 0) {
        printf("TTF_Init failed:\n");
        print_ttf_error();
    }

    // -- UI THEME
    app->ui_theme = new(UI_Theme);
    ui_init_theme(app->ui_theme);

    // -- init globals declared in core.h
    init_globals(app);

    // -- Renderer, after the renderer->sdl_renderer was set, initialise (generate) the rest
    init_renderer (app->renderer);
}

void deinit_app (App *app) {
    // -- deinit
    deinit_renderer(app->renderer);
    deinit_sdl(app);
    TTF_Quit();
    deinit_globals();

    // -- free
    free(app->ui_theme);
    free(app->renderer);
}

/// debugging for SDL2
void print_sdl_error() {
    const char *error = SDL_GetError();
    if (strlen(error) <= 0) {
        int x = 0; // dummy assignment for breakpoints
    }
    printf("ERROR: %s\n", error);
}
/// debugging for ttf SDL2
void print_ttf_error() {
    const char *error = TTF_GetError();
    if (strlen(error) <= 0) {
        int x = 0; // dummy assignment for breakpoints
    }
    printf("ERROR: %s\n", error);
}

f32 point_distance(Vec2i p1, Vec2i p2) {
    int x = p2.x - p1.x;
    int y = p2.y - p1.y;
    return (f32)SDL_sqrt(x * x + y * y);
}

bool point_in_circle(Vec2i point, Vec2i circle_pos, f32 radius) {
    return point_distance(point, circle_pos) <= radius;
}