#define SDL_MAIN_HANDLED // gets rid of linking errors

#include "sedefines.h"
#include "GL/glew.h"
#include "application.h"
#include "sestring.h"
#include "stdio.h"

int main() {
    SDL_Window *window;
    i32 window_w = 1600;
    i32 window_h = 1024;
    // -- init SDL
    ERROR_ON_NOTZERO_SDL(SDL_Init(SDL_INIT_EVERYTHING), "init_sdl");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // -- Create a window
    window = SDL_CreateWindow("SketchEngine_OpenGL Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ERROR_ON_NULL_SDL(window, "window was null");

    // -- Create context
    SDL_GLContext g_context = SDL_GL_CreateContext(window);
    ERROR_ON_NULL_SDL(g_context, "OpenGL context could not be created!");

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE); // Face Culling is enabled by default.

    // -- Init GLEW
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        printf("ERROR init GLEW! %s\n", glewGetErrorString(glew_error));
    }

    // -- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

    SDL_StopTextInput(); // don't take text input by default
    Application *app = new(Application);
    app_init(app, window);

    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        // delta time
    Uint64 now  = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    f64 delta_time = 0;
    // -- main loop
    while (!app->should_quit) {
        // -- events
        SDL_Event event;
        bool keyboard_down = false;
        bool keyboard_pressed = false;
        app->input.mouse_wheel = 0; // reset this to zero
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: { // -- wanting to quit
                    app->should_quit = true;
                } break;
                case SDL_WINDOWEVENT: { // -- resized window
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        // printf("window resized\n");
                        SDL_GetWindowSize(window, &window_w, &window_h);
                        glViewport(0, 0, window_w, window_h);
                    }
                } break;
                case SDL_KEYDOWN: {
                    // keyboard_pressed = true;
                    // keyboard_down = true;
                } break;
                case SDL_MOUSEWHEEL: {
                    app->input.mouse_wheel = event.wheel.preciseY;
                } break;
                case SDL_TEXTINPUT: { // this event happens after SDL_StartTextInput() is called
                    if (app->input.text_input_stream != NULL) {
                        if (app->input.is_text_input_only_numeric) {
                            if ((event.text.text[0] >= (i32)'0' && event.text.text[0] <= (i32)'9')
                                || event.text.text[0] == (i32)'-' || event.text.text[0] == (i32)'.') {
                                sestring_append(app->input.text_input_stream, event.text.text);
                            }
                        } else {
                            sestring_append(app->input.text_input_stream, event.text.text);
                            // printf("WHAT!!! %s\n", event.text.text);
                        }
                    } else {
                        printf("Warning: tried to append to input text stream but it was null\n");
                    }
                } break;
                case SDL_TEXTEDITING: {// this event happens after SDL_StartTextInput() is called
                } break;
            }
        }

        last = now;
        now = SDL_GetPerformanceCounter();
        delta_time = (f64)((now - last) / (f64)SDL_GetPerformanceFrequency());
        printf("delta time: %f\n", (f32)delta_time);
        app_update(app, (f32)delta_time);
        app_render(app);

        SDL_GL_SwapWindow(window);
    }

    // -- exit
    app_deinit(app);
    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("exited successfully!\n");
    return 0;
}
