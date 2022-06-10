#include "application.h"
#include "stdio.h" // @remove
#include "seui.h"
#include "panels.h"
#include "serenderer2D.h"

/* @temp */
RGBA color;

/* ui */
SE_UI *ctx;
u32 entity_panel = -1;
u32 light_panel = -1;
bool show_hsv                      = false; // toggles
bool show_texture_viewer_light_map = false; // toggles
bool show_texture_viewer_soulspear = false; // toggles
HSV hsv; // @temp
u32 test_texture; // @temp
u32 light_map_texture = 0;
Panel_Entity panel_entity;

static void panel_entity_init(Application *app, Panel_Entity *p, u32 entity_index) {
    Entity *entity  = &app->entities[entity_index];
    p->entity_id    = entity_index;
    p->entity_mesh  = &entity->mesh_index;
    p->entity_pos   = &entity->position;
    p->entity_rot   = &entity->oriantation;
    p->entity_scale = &entity->scale;
}

Application_Panel app_panel;

/* to render any texture on the screen */
// u32 cheat_vbo;
// Mat4 cheat_transform;

/* entities */
u32 player  = -1;
u32 player2 = -1;
u32 player3 = -1;
u32 skeleton_mesh = -1;
u32 plane = -1;
u32 bulb = -1; // bulb entity
Vec3 point_light_pos;
/* meshes */
u32 line_mesh = -1;
u32 proj_lines = -1;
u32 proj_box   = -1;
u32 current_obj_aabb = -1;
AABB3D world_aabb;

static void app_render_directional_shadow_map(Application *app);
static void app_render_omnidirectional_shadow_map(Application *app);

void app_init(Application *app, SDL_Window *window) {
    hsv.h = 45;
    hsv.s = 0.5f;
    hsv.v = 1.0f;
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
        serender3d_init(&app->renderer, &app->camera);
        app->renderer.light_directional.direction = (Vec3) {0, -1, 0};
        app->renderer.light_directional.ambient   = (RGB)  {50, 50, 50};
        app->renderer.light_directional.diffuse   = (RGB)  {255, 255, 255};
        // light_pos = (Vec3) {0.5f, 1, 0.5f};
        // light_pos_normalised = vec3_normalised(light_pos);
        light_map_texture = app->renderer.shadow_render_target.texture;
    }

    { // -- init entities
        player  = app_add_entity(app);
        player2 = app_add_entity(app);
        player3 = app_add_entity(app);
        plane = app_add_entity(app);
        bulb = app_add_entity(app);

        app->entities[player].position = vec3_zero();
        app->entities[plane].position = (Vec3) {0, -1.2f, 0};
        app->entities[bulb].position = vec3_zero();

        app->entities[player].scale = vec3_one();
        app->entities[plane].scale = vec3_one();
        app->entities[bulb].scale = vec3_one();

        app->entities[player].oriantation = vec3_zero();
        app->entities[plane].oriantation = vec3_zero();
        app->entities[bulb].oriantation = vec3_zero();

        app->entities[player2].position = vec3_create(-5, 2, -1);
        app->entities[player2].oriantation = vec3_zero();
        app->entities[player2].scale = v3f(0.1f, 0.1f, 0.1f);

        app->entities[player3].position = vec3_create(+5, 2, -1);
        app->entities[player3].oriantation = vec3_zero();
        app->entities[player3].scale = v3f(0.1f, 0.1f, 0.1f);

        line_mesh = serender3d_add_mesh_empty(&app->renderer);
        proj_lines = serender3d_add_mesh_empty(&app->renderer);
        proj_box = serender3d_add_mesh_empty(&app->renderer);
        current_obj_aabb = serender3d_add_mesh_empty(&app->renderer);

        /* meshes */
        app->entities[player].mesh_index = serender3d_load_mesh(&app->renderer, "assets/soulspear/soulspear.obj", false);
        // app->entities[player].mesh_index = serender3d_load_mesh(&app->renderer, "assets/models/fisherboy/source/all_posed.obj", false);
        app->entities[player].has_mesh = true;

        app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/animations/1/Booty Hip Hop Dance.fbx", false);
        // app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/animations/2/Sitting Laughing.dae", true);
        // app->entities[player2].mesh_index = serender3d_load_mesh(&app->renderer, "assets/animations/2/Sitting Laughing.fbx", true);
        app->entities[player2].has_mesh = true;

        app->entities[player3].mesh_index = serender3d_load_mesh(&app->renderer, "assets/animations/1/Booty Hip Hop Dance.fbx", true);
        app->entities[player3].has_mesh = true;

        app->entities[plane].mesh_index = serender3d_add_plane(&app->renderer, (Vec3) {20.0f, 20.0f, 20.0f});
        app->entities[plane].has_mesh = true;

        skeleton_mesh = serender3d_add_mesh_empty(&app->renderer);
        app->renderer.meshes[skeleton_mesh]->material_index = app->renderer.material_lines;
        semesh_generate_skinned_skeleton(app->renderer.meshes[skeleton_mesh], app->renderer.meshes[app->entities[player3].mesh_index]->skeleton, true);

        app->entities[bulb].mesh_index = serender3d_add_sprite_mesh(&app->renderer, v2f(1, 1));
        app->entities[bulb].has_mesh = true;
        u32 bulb_material = serender3d_add_material(&app->renderer);
        sesprite_load(&app->renderer.materials[bulb_material]->sprite, "assets/textures/light_bulb.png");
        app->renderer.meshes[app->entities[bulb].mesh_index]->material_index = bulb_material;
    }

    { // -- init UI
        ctx = new (SE_UI);
        // seui_init(ctx, &app->input, (Rect) {0,0, window_w, window_h}, 0.1f, 100);
        seui_init(ctx, &app->input, (Rect) {0,0, window_w, window_h}, -1000, 1000);

        // panel = seui_add_panel(ctx);
        // panel_entity_info = seui_add_panel(ctx);
        // seui_panel_setup(panel,             (Rect) {0, 0, 300, 400}, v2f(128 * 2, 128 * 2), false, 32, 1);
        // seui_panel_setup(panel_entity_info, (Rect) {0, 500, 64, 64}, v2f(128 * 2, 128 * 2), false, 32, 2);
        // test_colour_picker = seui_add_panel(ctx);

        panel_init(&app_panel);
        panel_entity_init(app, &panel_entity, player2);

        SE_Texture soulspear_texture = app->renderer.materials[app->renderer.meshes[app->entities[player].mesh_index]->material_index]->texture_diffuse;
        test_texture = soulspear_texture.id;
    }
    // { // -- constructed UI
    //     SE_Constructed_Panel root = {
    //         .config_centered = true,
    //         .config_pos = v2f(300, 300),

    //         .children = {
    //             (SE_Constructed) (SE_Constructed_Button) {
    //                 .text = "test button"
    //             }
    //         }
    //     };
    // }
}

void app_deinit(Application *app) {
    serender3d_deinit(&app->renderer);
    seui_deinit(ctx);
}

void app_update(Application *app) {
    // -- input
    u32 window_w, window_h;
    SDL_GetWindowSize(app->window, &window_w, &window_h);
    secamera3d_update_projection(&app->camera, window_w, window_h);
    seinput_update(&app->input, app->camera.projection, app->window);
    seui_resize(ctx, window_w, window_h);

    secamera3d_input(&app->camera, &app->input);

    {   // -- animation
        // seskeleton_calculate_pose(app->renderer.meshes[app->entities[player2].mesh_index]->skeleton, 0.0167f);
    }

/// ---------------------------------------------------------
///                    UI ARRANGEMENT
/// ---------------------------------------------------------
    { // -- ui
        seui_reset(ctx);

        if (seui_panel(ctx, "light")) {
            seui_panel_row(ctx, 32, 2);
            seui_label(ctx, "light direction:");
            seui_slider2d(ctx, &app_panel.light_direction);
            seui_panel_row(ctx, 32, 2);
            seui_label(ctx, "light intensity:");
            seui_slider(ctx, &app_panel.light_intensity);

            // seui_panel_row(ctx, 32, 2);
            // seui_label(ctx, "colour:");
            // color = seui_colour_picker_hsv(ctx, &hue, &saturation, &value);

            seui_panel_row(ctx, 32, 4);
            seui_label(ctx, "test input1:");
            seui_input_text(ctx, &app_panel.input_text);
            seui_label(ctx, "test input2:");
            seui_input_text(ctx, &app_panel.input_text2);
        }
        light_panel = ctx->current_panel->index;

        // if (seui_panel(ctx, "test bruh")) {
        //     seui_panel_row(ctx, 32, 2);
        //     seui_label(ctx, "text number 1");
        //     seui_label(ctx, "text # 2");
        //     seui_panel_container(ctx);
        // }

        if (seui_panel(ctx, "entity")) {
            char label_buffer[255];
            seui_panel_row(ctx, 32, 4);

            /* id */
            seui_label(ctx, "entity:");
            if (seui_selector(ctx, &panel_entity.entity_id, 0, app->entity_count-1)) {
                panel_entity_init(app, &panel_entity, panel_entity.entity_id);
            }
            /* mesh */
            sprintf(label_buffer, "%i", *panel_entity.entity_mesh);
            seui_label(ctx, "mesh:");
            seui_label(ctx, label_buffer);

            // SE_String entity_info;
            // sestring_init(&entity_info,
            /* pos, rot, scale */
            Vec3 rot_in_degrees = vec3_mul_scalar(*panel_entity.entity_rot, SEMATH_RAD2DEG_MULTIPLIER);
            seui_label_vec3(ctx, "position", panel_entity.entity_pos, true);
            seui_label_vec3(ctx, "rotation", &rot_in_degrees, true);
            seui_label_vec3(ctx, "scale", panel_entity.entity_scale, false);
            *panel_entity.entity_rot = vec3_mul_scalar(rot_in_degrees, SEMATH_DEG2RAD_MULTIPLIER);
        }
        entity_panel = ctx->current_panel->index;

        if (show_hsv) {
            seui_hsv_picker(ctx, &hsv);
            ctx->current_panel->is_closed = false;

            hsv_to_rgb(hsv.h, hsv.s, hsv.v, &app->renderer.light_directional.ambient);
        }

        if (show_texture_viewer_light_map && light_map_texture != 0) {
            seui_texture_viewer(ctx, light_map_texture);
            ctx->current_panel->is_closed = false;
        }

        if (show_texture_viewer_soulspear) {
            seui_texture_viewer(ctx, test_texture);
            ctx->current_panel->is_closed = false;
        }

        {   //-- PANELLESS WIDGETS
            ctx->current_panel = NULL; // unbind these widgets from a panel
            f32 cursor_x = 0;
            f32 button_size = 200;
            if (seui_button_at(ctx, "entity", (Rect) {cursor_x,0, button_size, 64})) {
                ctx->panels[entity_panel].is_closed = false;
            }
            cursor_x += button_size;

            if (seui_button_at(ctx, "light", (Rect) {cursor_x,0, button_size, 64})) {
                ctx->panels[light_panel].is_closed = false;
            }
            cursor_x += button_size;

            if (seui_button_at(ctx, "skeleton", (Rect) {cursor_x, 0, button_size, 64})) {
                app->entities[player2].should_render_mesh = !app->entities[player2].should_render_mesh;
            }
            cursor_x += button_size;

            if (seui_button_at(ctx, "hsv", (Rect) {cursor_x, 0, button_size, 64})) {
                show_hsv = !show_hsv;
            }
            cursor_x += button_size;

            if (seui_button_at(ctx, "texture viewer 1", (Rect) {cursor_x, 0, button_size, 64})) {
                show_texture_viewer_light_map = !show_texture_viewer_light_map;
            }
            cursor_x += button_size;

            if (seui_button_at(ctx, "texture viewer 2", (Rect) {cursor_x, 0, button_size, 64})) {
                show_texture_viewer_soulspear = !show_texture_viewer_soulspear;
            }
            cursor_x += button_size;
        }

    }

    { // -- calculate world aabb
        AABB3D *aabb = malloc(sizeof(AABB3D) * app->entity_count);
        Mat4 *transforms = malloc(sizeof(Mat4) * app->entity_count);

        for (u32 i = 0; i < app->entity_count; ++i) {
            AABB3D mesh_aabb = app->renderer.meshes[app->entities[i].mesh_index]->aabb;
            Mat4 entity_transform = entity_get_transform(&app->entities[i]);

            aabb[i] = aabb3d_from_points(mesh_aabb.min, mesh_aabb.max, entity_transform);
        }
        world_aabb = aabb3d_calc(aabb, app->entity_count);

        free(aabb);
        free(transforms);
    }

    // serender3d_update_gizmos_aabb(&app->renderer, world_aabb.min, world_aabb.max, 3, app->entities[line_entity].mesh_index);
    semesh_generate_gizmos_aabb(app->renderer.meshes[line_mesh], world_aabb.min, world_aabb.max, 3);
}

void app_render(Application *app) {
    { // -- application level render
        i32 window_w, window_h;
        SDL_GetWindowSize(app->window, &window_w, &window_h);
        secamera3d_update_projection(&app->camera, window_w, window_h);

        // Vec3 cam_forward = mat4_forward(app->camera.view);
        // app->renderer.light_directional.direction = cam_forward;
        Vec3 light_direction = {
            app_panel.light_direction.x,
            app_panel.light_direction.y,
            0
        };
        vec3_normalise(&light_direction);
        app->renderer.light_directional.direction = light_direction; //vec3_right();
        // 1. render directioanl shadow map
        app_render_directional_shadow_map(app);
        // 2. render point light shadow maps
        app->renderer.point_lights[0].position = app->entities[bulb].position;//point_light_pos;
        app_render_omnidirectional_shadow_map(app);
        // 3. render normally with the shadow map
        app->renderer.light_directional.intensity = app_panel.light_intensity;
        RGB ambient = app->renderer.light_directional.ambient;
        // RGB ambient = {
        //     color.r, color.g, color.b
        // };
        glClearColor(ambient.r / 255.0f, ambient.g / 255.0f, ambient.b / 255.0f, 1.0f);
        glClearDepth(1);
        // glClearColor(1, 1, 1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window_w, window_h);

        u32 selected = panel_entity.entity_id;
        serender3d_render_mesh_outline(&app->renderer, app->entities[selected].mesh_index, entity_get_transform(&app->entities[selected]));

        for (u32 i = 0; i < app->entity_count; ++i) {
            entity_render(&app->entities[i], &app->renderer);
        }

        // serender3d_render_mesh(&app->renderer, line_mesh, mat4_identity());
        serender_mesh_index(&app->renderer, proj_lines,       mat4_identity());
        serender_mesh_index(&app->renderer, proj_box,         mat4_identity());
        serender_mesh_index(&app->renderer, current_obj_aabb, mat4_identity());
        if (skeleton_mesh != -1) serender_mesh_index(&app->renderer, skeleton_mesh, entity_get_transform(&app->entities[player2]));

        // serender3d_render_mesh(&app->renderer, bulb_mesh,        mat4_translation(app->renderer.point_lights[0].position));
    }
    { // -- ui
        glClear(GL_DEPTH_BUFFER_BIT);
        seui_render(ctx);
    }
}

u32 app_add_entity(Application *app) {
    u32 result = app->entity_count;
    app->entity_count++;
    entity_default(&app->entities[result]);
    return result;
}

/// ---------------------------------------------------
///            DIRECTIONAL SHADOW MAPPING
/// ---------------------------------------------------

static void app_render_directional_shadow_map(Application *app) {
    // -- shadow mapping
    /* calculate the matrices */
#if 1
    // manually
    f32 left   =-10 + 20 * app_panel.left;   //world_aabb.min.x;
    f32 right  =-10 + 20 * app_panel.right;  //world_aabb.max.x;
    f32 bottom =-10 + 20 * app_panel.bottom; //world_aabb.min.y;
    f32 top    =-10 + 20 * app_panel.top;    //world_aabb.max.y;
    f32 near   =-10 + 20 * app_panel.near;   //world_aabb.min.z;
    f32 far    =-10 + 20 * app_panel.far;    //world_aabb.max.z;
    Vec3 light_pos = v3f(0, 0, 0);

    // ! the following is a bit messed up. the problem is that we calculate world aabb fine, but when light
    // ! rotation changes, we rotate that aabb and it does not cover everything
#else
    // automatically
    f32 left   = world_aabb.min.x;
    f32 right  = world_aabb.max.x;
    f32 bottom = world_aabb.min.y;
    f32 top    = world_aabb.max.y;
    f32 near   = world_aabb.min.z;
    f32 far    = world_aabb.max.z;
    Vec3 light_pos = (Vec3) {
        -light_direction.x,
        -light_direction.y,
        0,
    };
    light_pos = vec3_mul_scalar(light_pos, (far + near) * 0.5f);
#endif
    Mat4 light_proj = mat4_ortho(left, right, bottom, top, near, far);
    Vec3 light_target = vec3_add(app->renderer.light_directional.direction, light_pos);
    Mat4 light_view = mat4_lookat(light_pos, light_target, vec3_up());
    Mat4 light_space_mat = mat4_mul(light_view, light_proj);

    { // -- visualise the orhto projection
        f32 left    = -1;
        f32 right   = +1;
        f32 bottom  = -1;
        f32 top     = +1;
        f32 near    = -1;
        f32 far     = +1;

        Vec4 poss_4d[8] = {
            {.x = left,  .y = bottom, .z = near, 1.0}, // 0
            {.x = right, .y = bottom, .z = near, 1.0}, // 1
            {.x = right, .y = top,    .z = near, 1.0}, // 2
            {.x = left,  .y = top,    .z = near, 1.0}, // 3
            {.x = left,  .y = bottom, .z = far , 1.0}, // 4
            {.x = right, .y = bottom, .z = far , 1.0}, // 5
            {.x = right, .y = top,    .z = far , 1.0}, // 6
            {.x = left,  .y = top,    .z = far , 1.0}  // 7
        };

        Mat4 inv_light_space_mat = mat4_inverse(light_space_mat);
        poss_4d[0] = mat4_mul_vec4(inv_light_space_mat, poss_4d[0]);
        poss_4d[1] = mat4_mul_vec4(inv_light_space_mat, poss_4d[1]);
        poss_4d[2] = mat4_mul_vec4(inv_light_space_mat, poss_4d[2]);
        poss_4d[3] = mat4_mul_vec4(inv_light_space_mat, poss_4d[3]);
        poss_4d[4] = mat4_mul_vec4(inv_light_space_mat, poss_4d[4]);
        poss_4d[5] = mat4_mul_vec4(inv_light_space_mat, poss_4d[5]);
        poss_4d[6] = mat4_mul_vec4(inv_light_space_mat, poss_4d[6]);
        poss_4d[7] = mat4_mul_vec4(inv_light_space_mat, poss_4d[7]);

        Vec3 poss[8] = {
            {poss_4d[0].x, poss_4d[0].y, poss_4d[0].z},
            {poss_4d[1].x, poss_4d[1].y, poss_4d[1].z},
            {poss_4d[2].x, poss_4d[2].y, poss_4d[2].z},
            {poss_4d[3].x, poss_4d[3].y, poss_4d[3].z},
            {poss_4d[4].x, poss_4d[4].y, poss_4d[4].z},
            {poss_4d[5].x, poss_4d[5].y, poss_4d[5].z},
            {poss_4d[6].x, poss_4d[6].y, poss_4d[6].z},
            {poss_4d[7].x, poss_4d[7].y, poss_4d[7].z}
        };

        SE_Vertex3D verts[8] = {
            {.position = poss[0]},
            {.position = poss[1]},
            {.position = poss[2]},
            {.position = poss[3]},
            {.position = poss[4]},
            {.position = poss[5]},
            {.position = poss[6]},
            {.position = poss[7]}
        };

        u32 indices[24] = {
            0, 1,
            0, 3,
            0, 4,
            5, 1,
            5, 6,
            5, 4,
            2, 6,
            2, 3,
            2, 1,
            7, 3,
            7, 6,
            7, 4
        };

        app->renderer.meshes[proj_lines]->type       = SE_MESH_TYPE_LINE;
        app->renderer.meshes[proj_lines]->line_width = 4;
        semesh_generate_line_fan(app->renderer.meshes[proj_lines], light_pos, poss, 8, 3);

        app->renderer.meshes[proj_box]->type       = SE_MESH_TYPE_LINE;
        app->renderer.meshes[proj_box]->line_width = 4;
        semesh_generate(app->renderer.meshes[proj_box], 8, verts, 24, indices);
    }

    glDisable(GL_CULL_FACE); // @TODO what the f investigate
    { /* render the scene from the light's point of view */
        glCullFace(GL_FRONT);
        /* configure shadow shader */
        serender_target_use(&app->renderer.shadow_render_target);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        seshader_use(app->renderer.shaders[app->renderer.shader_shadow_calc]);

        seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_calc], "light_space_matrix", light_space_mat);

        for (u32 i = 0; i < app->entity_count; ++i) {
            Entity *entity = &app->entities[i];
            SE_Mesh *mesh = app->renderer.meshes[entity->mesh_index];
            Mat4 model_mat = entity_get_transform(entity);

            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_calc], "model", model_mat);

            glBindVertexArray(mesh->vao);
            if (mesh->indexed) {
                glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
            }
        }
        glBindVertexArray(0);

        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE); // @remove after fixing whatever this is
    }

    serender_target_use(NULL);

    app->renderer.light_space_matrix = light_space_mat;
}


/// ---------------------------------------------------
///         OMNIDIRECTIONAL SHADOW MAPPING
/// ---------------------------------------------------
static void app_render_omnidirectional_shadow_map(Application *app) {
    SE_Light_Point *point_light = &app->renderer.point_lights[0];
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, point_light->depth_map_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        { // configure shader and matrices
            // projection
            f32 aspect = 1024 / (f32) 1024;
            f32 near = 1.0f;
            f32 far  = 25.0f;
            Mat4 shadow_proj = mat4_perspective(SEMATH_DEG2RAD_MULTIPLIER * 90.0f, aspect, near, far);
            // views for each face
            Mat4 shadow_transforms[6];
            shadow_transforms[0] = mat4_mul(
                mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(1, 0, 0)), vec3_down()),
                shadow_proj);
            shadow_transforms[1] = mat4_mul(
                mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(-1, 0, 0)), vec3_down()),
                shadow_proj);
            shadow_transforms[2] = mat4_mul(
                mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, 1, 0)), vec3_forward()),
                shadow_proj);
            shadow_transforms[3] = mat4_mul(
                mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, -1, 0)), vec3_forward()), // ! this may need to be vec3_forward()
                shadow_proj);
            shadow_transforms[4] = mat4_mul(
                mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, 0, 1)), vec3_down()),
                shadow_proj);
            shadow_transforms[5] = mat4_mul(
                mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, 0, -1)), vec3_down()),
                shadow_proj);

            // configure shader
            seshader_use(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, point_light->depth_cube_map);
            seshader_set_uniform_f32(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "far_plane", far);
            seshader_set_uniform_vec3(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "light_pos", point_light->position);
            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "shadow_matrices[0]", shadow_transforms[0]);
            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "shadow_matrices[1]", shadow_transforms[1]);
            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "shadow_matrices[2]", shadow_transforms[2]);
            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "shadow_matrices[3]", shadow_transforms[3]);
            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "shadow_matrices[4]", shadow_transforms[4]);
            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "shadow_matrices[5]", shadow_transforms[5]);
        }
        // render scene
        for (u32 i = 0; i < app->entity_count; ++i) {
            Entity *entity = &app->entities[i];
            SE_Mesh *mesh = app->renderer.meshes[entity->mesh_index];
            Mat4 model_mat = entity_get_transform(entity);

            seshader_set_uniform_mat4(app->renderer.shaders[app->renderer.shader_shadow_omnidir_calc], "model", model_mat);

            glBindVertexArray(mesh->vao);
            if (mesh->indexed) {
                glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
            }
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}