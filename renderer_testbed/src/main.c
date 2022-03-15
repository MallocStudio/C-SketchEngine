#define SDL_MAIN_HANDLED // gets rid of linking errors

#include "defines.h"
#include "GL/glew.h"
#include "sketchengine.h"
#include "serenderer_opengl.h"

int main() {
    SDL_Window *window;
    i32 window_w = 1600;
    i32 window_h = 1024;

    // -- init SDL
    ERROR_ON_NOTZERO_SDL(SDL_Init(SDL_INIT_EVERYTHING), "init_sdl");

    // -- Use OpenGL 4.5 core // @TODO move these to sketchengine_init()
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // -- Create a window
    window = SDL_CreateWindow("SketchEngine_OpenGL Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ERROR_ON_NULL_SDL(window, "window was null");

    // -- Create context
    SDL_GLContext g_context = SDL_GL_CreateContext(window);
    ERROR_ON_NULL_SDL(g_context, "OpenGL context could not be created!");

    // -- Init GLEW
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        printf("ERROR init GLEW! %s\n", glewGetErrorString(glew_error));
    }

    // -- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

    SE_Camera3D camera;

    SE_Mesh mesh = {0};
    SE_Shader shader;
    Mat4 quad_transform;

    { // -- application level testing
        camera.view = mat4_lookat(vec3_create(5, 5, 5), vec3_create(0, 0, 0), vec3_up());
        camera.projection = mat4_perspective(SEMATH_PI * 0.25f,
                                            window_w / (f32) window_h,
                                            0.1f, 1000.0f);

        seshader_init_from(&shader, "Simple.vsd", "Simple.fsd");
        semesh_generate_quad(&mesh);
        quad_transform = (Mat4) {
            10, 0, 0, 0,
            0, 10, 0, 0,
            0, 0, 10, 0,
            0, 0, 0,  1,
        };
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
                        // printf("window resized\n");
                        // SDL_GetWindowSize(window, &window_w, &window_h);
                        // glViewport(0, 0, window_w, window_h);
                    }
                } break;
                case SDL_KEYDOWN: {
                    // keyboard_pressed = true;
                    // keyboard_down = true;
                    quit = true; // @temp
                } break;
                case SDL_MOUSEWHEEL: {
                    // f32 zoom_factor = 1.2f;
                    // if(event.wheel.y > 0) { // scroll up
                    //     segl_camera_zoom(game->camera, zoom_factor);
                    // }
                    // else if(event.wheel.y < 0) { // scroll down
                    //     segl_camera_zoom(game->camera, 1 / zoom_factor);
                    // }

                    static f32 angle = 0.0f;
                    if (event.wheel.y > 0) { // scroll up
                        angle += 0.01f;
                    } else if (event.wheel.y < 0) {
                        angle -= 0.01f;
                    }
                    camera.view = mat4_mul(camera.view, mat4_euler_z(angle));
                } break;
            }
        }

        { // -- application level update

        }

        { // -- application level render
            glClear(GL_COLOR_BUFFER_BIT);

            // in case of a window resize
            camera.projection = mat4_perspective(SEMATH_PI * 0.25f,
                                            window_w / (f32) window_h,
                                            0.1f, 1000.0f);

            // Mat4 projection_view_model = mat4_mul(camera.projection, camera.view);
            // projection_view_model = mat4_mul(projection_view_model, quad_transform);
            Mat4 projection_view_model = mat4_mul(camera.view, quad_transform);
            projection_view_model = mat4_mul(projection_view_model, camera.projection);

            seshader_use(&shader);
            seshader_set_uniform_mat4(&shader, "projection_view_model", projection_view_model);

            semesh_draw(&mesh);
        }

        SDL_GL_SwapWindow(window);
    }

    // -- exit
    { // -- application level deinit
        semesh_deinit(&mesh);
        seshader_deinit(&shader);
    }
    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("exited successfully!\n");
    return 0;
}
