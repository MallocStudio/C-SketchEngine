#define SDL_MAIN_HANDLED // gets rid of linking errors
#include "SDL2/SDL.h"
#include "SketchEngine_OpenGL.h"
#include <stdio.h>

/// debugging for SDL2
void print_sdl_error() {
    const char *error = SDL_GetError();
    if (strlen(error) <= 0) {
        int x = 0; // dummy assignment for breakpoints
    }
    printf("ERROR: %s\n", error);
}

#define new(type) ( type *) malloc (sizeof( type ))
#define ERROR_ON_NOTZERO_SDL(x, additional_message) if( x != 0) {printf("(%s)\n", additional_message); print_sdl_error();}
#define ERROR_ON_NULL_SDL(x, additional_message) if( x == NULL) {printf("(%s)\n", additional_message); print_sdl_error();}

int main () {
    SDL_Window *window;
    i32 window_w = 800;
    i32 window_h = 400;
    
    SE_GL_Context se_gl_context;
    se_gl_context.gProgramID = 0;
    se_gl_context.gVertexPos2DLocation = -1;
    se_gl_context.gIBO = 0;
    se_gl_context.gVBO = 0;

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
    bool glew_experimental = GL_TRUE;
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        printf("Error init GLEW! %s\n", glewGetErrorString(glew_error));
    }

    // -- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

    // -- Init OpenGL
    if (!initGL(&se_gl_context)) {
        printf("Unable to init OpenGL!\n");
    }

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
                        printf("LOL\n");
                        // SDL_GetWindowSize(window, &window_w, &window_h);
                        // SDL_RenderSetLogicalSize(app->renderer->sdl_renderer, app->window_width, app->window_height);
                    }
                } break;
                case SDL_KEYDOWN: {
                    keyboard_pressed = true;
                    keyboard_down = true;
                } break;
            }
        }
        // -- update input
        Uint8 *keyboard = SDL_GetKeyboardState(NULL);
        if (keyboard[SDL_SCANCODE_ESCAPE]) {
            printf("escape\n");
            quit = true;
        }

        // -- render
        render(&se_gl_context);
        SDL_GL_SwapWindow(window);
    }

    // -- exit
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("exited successfully!\n");
    return 0;
}