/// SDL Inlcudes
#define SDL_MAIN_HANDLED // gets rid of linking errors
#include "core.h"
#include "test.h"
#include "renderer.h"
#include "ui.h"

void   init_sdl(App *app);
void uninit_sdl(App *app);
void init_globals(App *app);
void uninit_globals();

// -- for delta time calculation
Uint64 NOW = 0;
Uint64 LAST = 0;

int main (int argc, char *argv[]) {
    App app; // @temp put app on the heap
    app.window_width  = 1000;
    app.window_height = 800;

    // -- Init SDL
    init_sdl(&app);
    reset_render_draw_color(app.renderer);

    // -- init globals declared in core.h
    init_globals(&app);
    UI_Button button_1;
    ui_init_button(&button_1, global_ui_theme);
    button_1.rect = (Rect) {100, 100, 128, 48};

    // -- loop
    bool should_close = false;
    while (!should_close) {
        // -- delta time
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        delta_time = (f32)( (NOW - LAST)*1000 / (f32)SDL_GetPerformanceFrequency() );

        // -- events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { // -- wanting to quit
                should_close = true;
            } else
            if (event.type == SDL_WINDOWEVENT_SIZE_CHANGED) { // -- resized window
                printf("LOL\n");
                SDL_GetWindowSize(global_app->window, &global_app->window_width, &global_app->window_height);
                SDL_RenderSetLogicalSize(global_app->renderer, global_app->window_width, global_app->window_height);
            }
        }

        // -- keyboard state
        // SDL_PumpEvents();
        SDL_GetKeyboardState(app.keyboard);
        if (app.keyboard[SDL_SCANCODE_ESCAPE] == 1) should_close = true;

        // -- update window surface // ! (matink dec 2021) we cannot use window surface if we're using the renderer according to https://dev.to/noah11012/using-sdl2-2d-accelerated-renderering-1kcb
        // app.surface = sdl.GetWindowSurface(app.window)
        // if app.surface == nil do sdl_print_error("GetWindowSurface:", sdl.GetError())
        // if sdl.UpdateWindowSurface(app.window) < 0 do sdl_print_error("UpdateWindowSurface:", sdl.GetError())

        SDL_RenderClear(app.renderer);
        // -- draw
        ui_draw_button(&button_1, global_ui_theme);
        // -- swap buffers
        SDL_RenderPresent(app.renderer);
    }

    // -- uninit SDL
    uninit_sdl(&app);
    uninit_globals();    
    return 0;
}
/// init SDL stuff
void init_sdl(App *app) {
    // -- SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        print_sdl_error();
    }

    // -- Init window
    app->window = SDL_CreateWindow("My C Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, app->window_width, app->window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if (app->window == NULL) {
        print_sdl_error();
    }

    // -- renderer
    app->renderer = SDL_CreateRenderer(app->window, 0, SDL_RENDERER_ACCELERATED);
    if (app->renderer == NULL) {
        print_sdl_error();
    }
    SDL_RenderSetLogicalSize(app->renderer, app->window_width, app->window_height);
}

/// uninit SDL
void uninit_sdl(App *app) {
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    SDL_Quit();
}

/// init globals defined in core.h
void init_globals(App *app) {
    global_app = app;
    delta_time = 0.f;
    global_ui_theme = (UI_Theme*) malloc (sizeof(UI_Theme));
    ui_init_theme(global_ui_theme);
}

/// init globals defined in core.h
void uninit_globals() {
    free(global_ui_theme);
}