#define SDL_MAIN_HANDLED // gets rid of linking errors

#include "sedefines.h"
#include "GL/glew.h"
#include "application.h"

int main() {
    SDL_Window *window;
    // i32 window_w = 1600;
    // i32 window_h = 1024;
    i32 window_w = 1024;
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
    glDepthFunc(GL_LEQUAL);

    // -- Init GLEW
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        printf("ERROR init GLEW! %s\n", glewGetErrorString(glew_error));
    }

    // -- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

    Application *app = new(Application);
    app_init(app, window);

    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // -- main loop
    while (!app->should_quit) {
        // -- events
        SDL_Event event;
        bool keyboard_down = false;
        bool keyboard_pressed = false;
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
                    // f32 zoom_factor = 1.2f;
                    // if(event.wheel.y > 0) { // scroll up
                    //     segl_camera_zoom(game->camera, zoom_factor);
                    // }
                    // else if(event.wheel.y < 0) { // scroll down
                    //     segl_camera_zoom(game->camera, 1 / zoom_factor);
                    // }

                    // f32 angle = 0.0f;
                    // if (event.wheel.y > 0) { // scroll up
                    //     angle = 0.01f;
                    // } else if (event.wheel.y < 0) {
                    //     angle = -0.01f;
                    // }
                    // camera.view = mat4_mul(camera.view, mat4_euler_y(angle));
                } break;
            }
        }

        app_update(app);
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
