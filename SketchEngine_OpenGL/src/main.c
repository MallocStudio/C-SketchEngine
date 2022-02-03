// %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
#define SDL_MAIN_HANDLED // gets rid of linking errors
#include "SDL2/SDL.h"
#include "SketchEngine_OpenGL.h"
#include <stdio.h>

#define RAYMATH_IMPLEMENTATION
#include "raymath.h"

int main () {
    SDL_Window *window;
    i32 window_w = 800;
    i32 window_h = 400;
    
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
        printf("Error init GLEW! %s\n", glewGetErrorString(glew_error));
    }

    // -- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

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
                        printf("window resized\n");
                        // i32 prev_w = window_w;
                        // i32 prev_h = window_h;

                        SDL_GetWindowSize(window, &window_w, &window_h);
                        // f32 new_aspect_ration = window_w / (f32)window_h;
                        // i32 xspan = 1;
                        // i32 ypan = 1;
                        // if (new_aspect_ration > 1) {
                        //     // width > height
                        //     xspan *= new_aspect_ration;
                        // } else {
                        //     if (new_aspect_ration == 0) new_aspect_ration = 0.1f;
                        //     yspan = xspan / new_aspect_ration;
                        // }

                        // i32 desired_aspect_ration = game->camera->aspect_ratio;
                        // i32 desired_w = prev_w;
                        // i32 desired_h = prev_h;
                        glViewport(0, 0, window_w, window_h);
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

        // -- render
        finn_game_render(game);
        SDL_GL_SwapWindow(window);

        if (game->keyboard[SDL_SCANCODE_ESCAPE]) {
            printf("quit\n");
            quit = true;
        }
    }

    // -- exit
    finn_game_deinit(game);
    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("exited successfully!\n");
    return 0;
}