/// SDL Inlcudes
#define SDL_MAIN_HANDLED // gets rid of linking errors
#include "core.h"
#include "renderer.h"
#include "ui.h"
#include "input.h"

// -- for delta time calculation
Uint64 NOW = 0;
Uint64 LAST = 0;

int main (int argc, char *argv[]) {
    // -- initialise app
    App *app = new(App);
    init_app(app);    

    UI_Context context = {0};
    context.active = -1;
    context.hot    = -1;
    context.renderer = app->renderer;
    context.theme = app->ui_theme; // @incomplete move ui_theme to ctx struct

    Rect rect = (Rect) {100, 100, 60, 48};
    // -- loop
    bool should_close = false;
    while (should_close == false) {
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
                SDL_GetWindowSize(app->window, &app->window_width, &app->window_height);
                SDL_RenderSetLogicalSize(app->renderer->sdl_renderer, app->window_width, app->window_height);
            }
        }

        SDL_RenderClear(app->renderer->sdl_renderer);
        // -- draw
        ui_update_context(&context);

        // -- ctx ui test
        // if (ui_begin(&context, (Rect) {20, 20, 300, 400}, 3, UI_LAYOUT_VERTICAL)) {
        //     ui_label(&context, "test_label");
        //     if (ui_button(&context, 1, "button 1")) {
        //         printf("button 1 pressed\n");
        //     }
        //     if (ui_button(&context, 2, "button 2")) {
        //         printf("button 2 pressed\n");
        //     }
        // }
        if (ui_begin(&context, (Rect) {20, 20, 300, 300})) {
            ui_row(&context, 3, 48);
            if (ui_button(&context, 1, "button 1")) printf("button 1 pressed\n");
            if (ui_button(&context, 2, "button 2")) printf("button 2 pressed\n");
            if (ui_button(&context, 3, "button 3")) printf("button 3 pressed\n");
            ui_row(&context, 1, 32);
            if (ui_button(&context, 4, "button 4")) printf("button 4 pressed\n");
            ui_row(&context, 1, 64);
            if (ui_button(&context, 5, "button 5")) printf("button 5 pressed\n");
        }
        // -- swap buffers
        SDL_RenderPresent(app->renderer->sdl_renderer);
    }

    // -- uninit app
    deinit_app(app);    
    free(app);
    
    printf("prgram closed successfully!\n");
    return 0;
}

#pragma region // after -- events in main loop 
        // -- keyboard state
        // SDL_PumpEvents();
        // SDL_GetKeyboardState(app.keyboard);
        // if (app.keyboard[SDL_SCANCODE_ESCAPE] == 1) should_close = true;

        // -- update window surface // ! (matink dec 2021) we cannot use window surface if we're using the sdl_renderer according to https://dev.to/noah11012/using-sdl2-2d-accelerated-renderering-1kcb
        // app.surface = sdl.GetWindowSurface(app.window)
        // if app.surface == nil do sdl_print_error("GetWindowSurface:", sdl.GetError())
        // if sdl.UpdateWindowSurface(app.window) < 0 do sdl_print_error("UpdateWindowSurface:", sdl.GetError())
#pragma endregion