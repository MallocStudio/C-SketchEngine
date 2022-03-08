#define SDL_MAIN_HANDLED // gets rid of linking errors

#define DEFINES_IMPL
#include "defines.h"
#include "finn_game.h"

int main () {
    { // -- unit tests
        printf("------------MATH TESTS------------\n");
        bool result = true;
        result = mat2_test();
        if (!result) return 1; // error. our unit tests failed
        printf("--------END OF MATH TESTS---------\n");
    }
    // SDL_DisplayMode dm;
    // SDL_GetCurrentDisplayMode(0, &dm);
    SDL_Window *window;
    i32 window_w = 1600;
    i32 window_h = 1024;
    
    // -- init SDL
    ERROR_ON_NOTZERO_SDL(SDL_Init(SDL_INIT_EVERYTHING), "init_sdl");

    // -- Use OpenGL 3.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // -- Create window
    window = SDL_CreateWindow("SketchEngine_OpenGL window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ERROR_ON_NULL_SDL(window, "window was null");

    // -- Create context
    SDL_GLContext g_context = SDL_GL_CreateContext(window);
    ERROR_ON_NULL_SDL(g_context, "OpenGL context could not be created!");

    // -- Init GLEW
    // bool glew_experimental = GL_TRUE;
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        printf("ERROR init GLEW! %s\n", glewGetErrorString(glew_error));
    }

    // -- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

    // -- Init phsycis renderer
    global_physics_debug = new(SE_Physics_Global);
    se_physics_global_init();
    global_physics_debug->active = true;

    // -- Init Finn's example
    Finn_Game *game = new(Finn_Game);
    finn_game_init(game, window);

    // -- main loop
    bool quit = false;
    while (!quit) {
        // -- events
        SDL_Event event;
        bool keyboard_down = false;
        bool keyboard_pressed = false;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: { // -- wanting to quit
                    quit = true;
                } break;
                case SDL_WINDOWEVENT: { // -- resized window
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        // printf("window resized\n");
                        // SDL_GetWindowSize(window, &window_w, &window_h);
                        // glViewport(0, 0, window_w, window_h);
                    }
                } break;
                case SDL_KEYDOWN: {
                    keyboard_pressed = true;
                    keyboard_down = true;
                } break;
                case SDL_MOUSEWHEEL: {
                    f32 zoom_factor = 1.2f;
                    if(event.wheel.y > 0) { // scroll up
                        segl_camera_zoom(game->camera, zoom_factor);
                    }
                    else if(event.wheel.y < 0) { // scroll down 
                        segl_camera_zoom(game->camera, 1 / zoom_factor);
                    }
                } break;
            }
        }
        // -- update
        finn_game_update(game, 0.0166667f);

        // -- physics update
        // ! if paused, update one frame when space is pressed
        if (!game->is_paused) {
            finn_game_physics_update(game, 0.0166667f);
        }
        else if (game->is_physics_update_queued) {
            finn_game_physics_update(game, 0.0166667f);
            game->is_physics_update_queued = false;
        }

        // -- render
        finn_game_render(game);

        SDL_GL_SwapWindow(window);
        if (game->input.keyboard[SDL_SCANCODE_ESCAPE]) {
            printf("quit\n");
            quit = true;
        }
    }

    // -- exit
    finn_game_deinit(game);
    se_physics_global_deinit();
    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("exited successfully!\n");
    return 0;
}