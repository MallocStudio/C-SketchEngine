/// SDL Inlcudes
// copy and paste the following into a normal cmd on windows to setup the dev environment:
// %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
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

    // -- double check versions of the dependencies
    SDL_version compiled;
    SDL_version linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    SDL_Log("We compiled against SDL version %u.%u.%u ...\n", compiled.major, compiled.minor, compiled.patch);
    SDL_Log("But we are linking against SDL version %u.%u.%u.\n", linked.major, linked.minor, linked.patch);

    SDL_version compile_version, *link_version;
    TTF_VERSION(&compile_version);
    printf("compiled with SDL_ttf version: %d.%d.%d\n", 
            compile_version.major,
            compile_version.minor,
            compile_version.patch);
    link_version=TTF_Linked_Version();
    printf("running with SDL_ttf version: %d.%d.%d\n", 
            link_version->major,
            link_version->minor,
            link_version->patch);
    
    UI_Context *ctx = new(UI_Context);
    ui_init_context(ctx, app->renderer);

    Rect rect = (Rect) {100, 100, 400, 600};
    // -- loop
    bool should_close = false;
    while (should_close == false) {
        // -- delta time
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        delta_time = (f32)( (NOW - LAST)*1000 / (f32)SDL_GetPerformanceFrequency() );

        // -- events
        SDL_Event event;
        app->keyboard_down = false;
        app->keyboard_pressed = false;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: { // -- wanting to quit
                    should_close = true;
                } break;
                case SDL_WINDOWEVENT: { // -- resized window
                    // if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    //     printf("LOL\n");
                    //     SDL_GetWindowSize(app->window, &app->window_width, &app->window_height);
                    //     SDL_RenderSetLogicalSize(app->renderer->sdl_renderer, app->window_width, app->window_height);
                    // }
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        printf("LOL\n");
                        SDL_GetWindowSize(app->window, &app->window_width, &app->window_height);
                        SDL_RenderSetLogicalSize(app->renderer->sdl_renderer, app->window_width, app->window_height);
                    }
                } break;
                case SDL_KEYDOWN: {
                    app->keyboard_pressed = true;
                    app->keyboard_down = true;
                } break;
            }
        }
        
        // -- update input
        // SDL_PumpEvents();
        app->keyboard = SDL_GetKeyboardState(NULL);
        if (app->keyboard[SDL_SCANCODE_ESCAPE]) {
            printf("escape\n");
            should_close = true;
        }
        if (input_is_key_pressed(app, SDL_SCANCODE_R)) {
            printf("resize render logic\n");
            SDL_GetWindowSize(app->window, &app->window_width, &app->window_height);
            SDL_RenderSetLogicalSize(app->renderer->sdl_renderer, app->window_width, app->window_height);
        }

        SDL_RenderClear(app->renderer->sdl_renderer);
        // -- draw
        render_set_draw_color_raw(app->renderer->sdl_renderer, 255, 0, 0, 1);
        render_grid(app->renderer->sdl_renderer, 64, 64, 300, 300, 32);
        render_reset_draw_color(app->renderer->sdl_renderer);
        
        ui_update_context(ctx);

        // -- ctx ui test
        ui_begin(ctx, &rect);
        ui_row(ctx, 3, 48, 100);
        if (ui_button(ctx, "button 1")) printf("button 1 pressed\n");
        if (ui_button(ctx, "button 2")) printf("button 2 pressed\n");
        if (ui_button(ctx, "button 3")) printf("button 3 pressed\n");
        ui_row(ctx, 1, 32, 0);
        if (ui_button(ctx, "button 4")) printf("button 4 pressed\n");
        ui_row(ctx, 1, 64, 0);
        if (ui_button(ctx, "button 5")) printf("button 5 pressed\n");
        ui_row(ctx, 1, 200, 0);
        ui_label(ctx, "ma danny long text lalbal blbll balaha labal increasing text to take even more space to test the wrapping functionality");

        render_set_draw_color_raw(app->renderer->sdl_renderer, 255, 0, 0, 1);
        render_circle(app->renderer->sdl_renderer, 300, 300, 64);
        Vec2i mouse_pos = get_mouse_pos(NULL, NULL);
        render_cross(app->renderer->sdl_renderer, mouse_pos.x, mouse_pos.y, 16);
        render_reset_draw_color(app->renderer->sdl_renderer);
        
        // -- swap buffers
        SDL_RenderPresent(app->renderer->sdl_renderer);
    }

    // -- uninit app
    deinit_app(app);    
    ui_deinit_context(ctx);
    free(app);
    free(ctx);
    
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