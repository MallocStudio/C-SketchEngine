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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

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
        // camera.pos = vec3_create(5, 5, 5);
        camera.pos = vec3_create(10, 10, 10);
        camera.phi = 0;
        camera.theta = 0;

        // camera.look_at = vec3_zero();
        // camera.look_at = vec3_add(camera.pos, vec3_forward());

        // camera.oriantation = quat_identity();
        // Quat oriantation = quat_from_axis_angle(vec3_create(0, 1, 0), 0, true);
        // camera.oriantation = quat_mul(oriantation, camera.oriantation);

        // camera.oriantation = quat_from_axis_angle(vec3_create(1, 0, 0), SEMATH_DEG2RAD_MULTIPLIER * -45, false);

        // camera.view = mat4_lookat(camera.pos, vec3_zero(), vec3_up());


        seshader_init_from(&shader, "Simple.vsd", "Simple.fsd");

        // semesh_generate_quad(&mesh);
        SE_Vertex3D verts[5];
        verts[0].position = (Vec4) {-0.5f, 0, +0.5f, 1};
        verts[1].position = (Vec4) {+0.5f, 0, +0.5f, 1};
        verts[2].position = (Vec4) {-0.5f, 0, -0.5f, 1};
        verts[3].position = (Vec4) {+0.5f, 0, -0.5f, 1};
        verts[4].position = (Vec4) {+1.0f, -0.5f, -0.5f, 1};

        verts[0].rgba = RGBA_RED;
        verts[1].rgba = RGBA_RED;
        verts[2].rgba = RGBA_RED;
        verts[3].rgba = RGBA_BLUE;
        verts[4].rgba = RGBA_GREEN;

        u32 indices[9] = {
            0, 1, 2,
            2, 1, 3,
            3, 1, 4,
        };

        semesh_generate(&mesh, 5, verts, 9, indices);

        quad_transform = (Mat4) {
            10, 0, 0, 0,
            0, 10, 0, 0,
            0, 0, 10, 0,
            0, 0, 0,  1,
        };
    }

    Vec2i mouse_pos;
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    Vec2i mouse_pos_pre = mouse_pos;

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

        { // -- application level update
            const u8 *keyboard = SDL_GetKeyboardState(NULL);
            if (keyboard[SDL_SCANCODE_ESCAPE]) quit = true;

            i32 r = keyboard[SDL_SCANCODE_D] == true ? 1 : 0;
            i32 l = keyboard[SDL_SCANCODE_A] == true ? 1 : 0;
            i32 d = keyboard[SDL_SCANCODE_S] == true ? 1 : 0;
            i32 u = keyboard[SDL_SCANCODE_W] == true ? 1 : 0;
            i32 elevate = keyboard[SDL_SCANCODE_E] == true ? 1 : 0;
            i32 dive = keyboard[SDL_SCANCODE_Q] == true ? 1 : 0;

            Vec3 input = vec3_create(r - l, d - u, elevate - dive);
            camera.pos.x += input.x;
            camera.pos.z += input.y;
            camera.pos.y += input.z;

            { // quaternion way
                // static f32 v_angle = 0.0f;
                // static f32 h_angle = 0.0f;
                // v_angle = input.y * 0.01f;
                // h_angle = input.x * 0.01f;
                // Quat quat_v = quat_from_axis_angle(vec3_create(1, 0, 0), v_angle, true);
                // Quat quat_h = quat_from_axis_angle(vec3_create(0, 1, 0), h_angle, true);

                // // Quat quat_v = quat_from_axis_angle(mat4_up(quat_to_mat4(camera.oriantation)), v_angle, true);
                // // Quat quat_h = quat_from_axis_angle(mat4_right(quat_to_mat4(camera.oriantation)), h_angle, true);

                // Quat quat = quat_mul(quat_v, quat_h);

                // camera.oriantation = quat_mul(quat, camera.oriantation);
            }
            { // AIE tutorial way
                u8 mouse_state = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
                if (mouse_state & SDL_BUTTON_RMASK) {
                    f32 turn_speed = 0.1f;
                    camera.theta -= turn_speed * (f32)(mouse_pos.x - mouse_pos_pre.x);
                    camera.phi   += turn_speed * (f32)(mouse_pos.y - mouse_pos_pre.y);
                }
                mouse_pos_pre = mouse_pos;
            }

        }

        { // -- application level render
            glClear(GL_COLOR_BUFFER_BIT);
            glClear(GL_DEPTH_BUFFER_BIT);

            // secamera3d_update_view(&camera);
            // camera.view = mat4_lookat(camera.pos, vec3_zero(), vec3_up());
            camera.view = secamera3d_get_view(&camera);
            camera.projection = mat4_perspective(SEMATH_PI * 0.25f,
                                            window_w / (f32) window_h,
                                            0.1f, 1000.0f);

            // quad_transform = mat4_mul(quad_transform, mat4_euler_y(0.01f));

            seshader_use(&shader);

            // take the quad (world space) and project it to view space
            Mat4 pvm = mat4_mul(quad_transform, camera.view);
            // then take that and project it to the clip space
            pvm = mat4_mul(pvm, camera.projection);
            // then pass that final projection matrix and give it to the shader
            seshader_set_uniform_mat4(&shader, "projection_view_model", pvm);

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
