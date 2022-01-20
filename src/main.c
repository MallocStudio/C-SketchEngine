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

    // // -- text test with button
    // Text* text = new(Text); // @temp
    // // init_text(text, app->sdl_renderer, "yo shit it worked again!", font, (RGBA){1, 1, 1, 1});
    // generate_text_from_glyphs(text, app->renderer->sdl_renderer, app->renderer->glyphs, "yo shit it wooorked!");

    // UI_Button button_1;
    // ui_init_button(&button_1, text, app->ui_theme);
    // button_1.rect = (Rect) {100, 100, 128, 48};
    // f32 i = 0;
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
#pragma region // notes 
        // -- keyboard state
        // SDL_PumpEvents();
        // SDL_GetKeyboardState(app.keyboard);
        // if (app.keyboard[SDL_SCANCODE_ESCAPE] == 1) should_close = true;

        // -- update window surface // ! (matink dec 2021) we cannot use window surface if we're using the sdl_renderer according to https://dev.to/noah11012/using-sdl2-2d-accelerated-renderering-1kcb
        // app.surface = sdl.GetWindowSurface(app.window)
        // if app.surface == nil do sdl_print_error("GetWindowSurface:", sdl.GetError())
        // if sdl.UpdateWindowSurface(app.window) < 0 do sdl_print_error("UpdateWindowSurface:", sdl.GetError())
#pragma endregion
        SDL_RenderClear(app->renderer->sdl_renderer);
        // -- draw
        // if (ui_render_button(app->renderer->sdl_renderer, &button_1, app->ui_theme)) {
        //     printf("haleloya: %f\n", i);
        //     ++i;
        // }
        
        ui_render_floating_rect(app->renderer, &rect);
        render_string(app->renderer, "render_string test", rect, true);

        // -- swap buffers
        SDL_RenderPresent(app->renderer->sdl_renderer);
    }

    // deinit_text(text);     // @temp with text with button test

    // -- uninit app
    deinit_app(app);    
    free(app);
    
    printf("prgram closed successfully!\n");
    return 0;
}