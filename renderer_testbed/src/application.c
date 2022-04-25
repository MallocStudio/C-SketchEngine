#include "application.h"
#include "stdio.h" // @remove
#include "seui.h"

#if 0
SEUI_Context *ctx;
#endif

/* ui */
SE_UI *ctx;
SEUI_Panel panel;

/* shadow mapping */
#define shadow_w  1024
#define shadow_h  1024
u32 shadow_depth_map_fbo;
u32 shadow_depth_map;
SE_Shader shadow_shader;
SE_Shader shadow_depth_map_shader; // to debug depth map

// @remove
u32 shadow_vbo;

static void render_scene(SE_Shader *shader, Application *app) { // @temp
    seshader_use(shader);
    for (u32 i = 0; i < app->entity_count; ++i) {
        Entity *entity = &app->entities[i];
        SE_Mesh *mesh = app->renderer.meshes[entity->mesh_index];
        Mat4 model_mat = entity->transform;

        glBindVertexArray(mesh->vao);
        if (mesh->indexed) {
            glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
        }
    }
    glBindVertexArray(0);
}

void app_init(Application *app, SDL_Window *window) {
    u32 player = -1; // @remove
    u32 player2 = -1;
    memset(app, 0, sizeof(Application));

    app->window = window;
    app->should_quit = false;
    u32 window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    // -- input input
    seinput_init(&app->input);

    { // -- init camera
        secamera3d_init(&app->camera);
    }

    { // -- init renderer
        serender3d_init(&app->renderer, &app->camera, "shaders/Simple.vsd", "shaders/Simple.fsd");
        app->renderer.light_directional.direction = (Vec3) {0, -1, 0};
        app->renderer.light_directional.ambient   = (RGB)  {50, 50, 50};
        app->renderer.light_directional.diffuse   = (RGB)  {255, 255, 255};
    }

    { // -- init entities
        player  = app_add_entity(app);
        player2 = app_add_entity(app);
        app->entities[player].transform = mat4_translation(vec3_zero());
        app->entities[player2].transform = mat4_translation((Vec3) {0, -1.2f, 0}); // mat4_translation(vec3_create(10, 0, 5));
    }

    { // -- init UI
        ctx = new (SE_UI);
        seui_init(ctx, &app->input, window_w, window_h);
        panel.initial_rect = (Rect) {250, 300, 300, 400};
        panel.minimised = false;
    }

    { // -- load mesh
        app->entities[player].mesh_index = serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj");
        // app->entities[player2].mesh_index = serender3d_add_cube(&app->renderer);
        app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/models/plane/plane.fbx");
        app->entities[player2].transform = mat4_mul(mat4_euler_x(SEMATH_HALF_PI), app->entities[player2].transform);
    }

    { // -- shadow mapping
        // https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
        glGenFramebuffers(1, &shadow_depth_map_fbo);

        glGenTextures(1, &shadow_depth_map);
        glBindTexture(GL_TEXTURE_2D, shadow_depth_map);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_w, shadow_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_map_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        seshader_init_from(&shadow_shader, "shaders/SimpleShadow.vsd", "shaders/SimpleShadow.fsd");
        seshader_init_from(&shadow_depth_map_shader, "shaders/ShadowDepthMap.vsd", "shaders/ShadowDepthMap.fsd");

        glGenBuffers(1, &shadow_vbo);
        Vec2 quad[4] = {
                (Vec2) {0, 0},
                (Vec2) {1, 0},
                (Vec2) {1, 1},
                (Vec2) {0, 1}
        };
        glBindBuffer(GL_ARRAY_BUFFER, shadow_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 4, quad, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
    seui_deinit(ctx);
    seshader_deinit(&shadow_shader);
    seshader_deinit(&shadow_depth_map_shader);
}

f32 slider_value = 0.5f;
Vec2 slider2d_value = {0};
void app_update(Application *app) {
    // -- input
    u32 window_w, window_h;
    SDL_GetWindowSize(app->window, &window_w, &window_h);
    secamera3d_update_projection(&app->camera, window_w, window_h);
    seinput_update(&app->input, app->camera.projection, app->window);
    seui_resize(ctx, window_w, window_h);

    const u8 *keyboard = app->input.keyboard;
    if (keyboard[SDL_SCANCODE_ESCAPE]) app->should_quit = true;

    secamera3d_input(&app->camera, &app->input);

    { // -- ui
        seui_reset(ctx);

        if (seui_panel_at(ctx, "panel", 2, 100, &panel)) {
            seui_label(ctx, "light direction:");
            seui_slider2d(ctx, &slider2d_value);
        }
    }
}

void app_render(Application *app) {
    { // -- application level render
        i32 window_w, window_h;
        SDL_GetWindowSize(app->window, &window_w, &window_h);
        secamera3d_update_projection(&app->camera, window_w, window_h);

        // Vec3 cam_forward = mat4_forward(app->camera.view);
        // app->renderer.light_directional.direction = cam_forward;
        Vec3 light_direction = {
            -slider2d_value.x,
            -slider2d_value.y,
            0
        };
        vec3_normalise(&light_direction);
        app->renderer.light_directional.direction = light_direction; //vec3_right();

        // 1. render to depth map
        { // -- shadow mapping
            glViewport(0, 0, shadow_w, shadow_h);
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_map_fbo);
                glClear(GL_DEPTH_BUFFER_BIT);
                // configure shader and matrices
                seshader_use(&shadow_shader);
                f32 near_plane = 1.0f, far_plane = 70.5f;
                Mat4 light_projection = mat4_ortho(-200.0f, 200.0f, -200.0f, 200.0f, near_plane, far_plane); // what is visible to the light
                // Mat4 light_view = mat4_lookat(vec3_zero(), app->renderer.light_directional.direction, vec3_up());
                Vec3 light_pos = vec3_create(-2, 4, -1);
                Vec3 light_target = vec3_add(app->renderer.light_directional.direction, light_pos);
                Mat4 light_view = mat4_lookat(light_pos, light_target, vec3_up());

                // Mat4 light_space_mat = mat4_mul(light_projection, light_view);
                Mat4 light_space_mat = mat4_mul(app->renderer.current_camera->projection, app->renderer.current_camera->view);

                seshader_set_uniform_mat4(&shadow_shader, "light_space_matrix", light_space_mat);
                render_scene(&shadow_shader, app);

                // render scene
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // // 2. render normally with the shadow map
        RGB ambient = app->renderer.light_directional.ambient;
        rgb_normalise(&ambient);
        glClearColor(ambient.r, ambient.g, ambient.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window_w, window_h);
        { // @cleanup
        glActiveTexture(GL_TEXTURE0 + 4); // shadow map
        glBindTexture(GL_TEXTURE_2D, shadow_depth_map);
        }

        for (u32 i = 0; i < app->entity_count; ++i) {
            entity_render(&app->entities[i], &app->renderer);
        }

        // { // @temp an attempt to draw our shadow map to the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            seshader_use(&shadow_depth_map_shader);
            glBindBuffer(GL_ARRAY_BUFFER, shadow_vbo);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, shadow_depth_map);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        // }
    }
    { // -- ui
        seui_render(ctx);
    }
}

u32 app_add_entity(Application *app) {
    u32 result = app->entity_count;
    app->entity_count++;
    return result;
}