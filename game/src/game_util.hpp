#pragma once
#include "game.hpp"
//! This is meant to be included in game.cpp ONLY. Nowhere else. So this has to be included once.

///
///     General variables used in game.cpp
///

char fps_text[24];
SE_UI *ctx;

    //@temp a temporary way of loading required meshes once at init_application() time
u32 mesh_soulspear = -1;
u32 mesh_plane = -1;
u32 mesh_guy = -1;
u32 mesh_skeleton = -1;
u32 mesh_gizmos_translate = -1;
u32 mesh_light_pos_gizmos = -1;
u32 mesh_cube = -1;
u32 mesh_cube_tiny = -1;
u32 mesh_demo_crate = -1;
u32 mesh_demo_diamond = -1;
u32 diamond_shader = -1;

u32 current_obj_aabb = -1;
u32 world_aabb_mesh = -1;
u32 point_light_1 = -1;
u32 point_light_2 = -1;
u32 point_light_3 = -1;
u32 point_light_4 = -1;

    //@temp
SE_Grid grid;
RGBA value_mappings[SE_GRID_MAX_VALUE];

SE_Animation animation;

RGBA dim = {100, 100, 100, 255};
RGBA lit = {255, 255, 255, 255};

    //@debug
u32 debug_raycast_visual = -1;
SE_Texture debug_screen_quad_texture;

u32 main_camera = -1;

///
///     UTIL FUNCTIONALITIES
///

void App::util_load_meshes_from_disk() {
        // @temp TODO: MOVE TO LOADER
    mesh_soulspear = se_render3d_load_mesh(&m_renderer, "game/meshes/soulspear/soulspear.obj", false);
#if 1
    // mesh_guy = se_render3d_load_mesh(&m_renderer, "game/meshes/Booty Hip Hop Dance.fbx", true);
    mesh_guy = se_render3d_load_mesh(&m_renderer, "game/meshes/Sitting Laughing.fbx", true);
    // mesh_guy = se_render3d_load_mesh(&m_renderer, "game/meshes/changedPrizm2.fbx", true);
    // mesh_guy = se_render3d_load_mesh(&m_renderer, "core/meshes/TriangularPrism.fbx", true);
    mesh_skeleton = se_render3d_add_mesh_empty(&m_renderer);
    se_mesh_generate_skinned_skeleton(m_renderer.user_meshes[mesh_skeleton], m_renderer.user_meshes[mesh_guy]->skeleton, true, true);
    // se_mesh_generate_static_skeleton(m_renderer.meshes[mesh_skeleton], m_renderer.meshes[mesh_guy]->skeleton);

    animation.current_frame = 0;
    animation.duration = m_renderer.user_meshes[mesh_guy]->skeleton->animations[0]->duration;
    animation.speed = m_renderer.user_meshes[mesh_guy]->skeleton->animations[0]->ticks_per_second;
#endif

    mesh_plane = se_render3d_add_plane(&m_renderer, v3f(1, 1, 1));

    mesh_gizmos_translate = se_gizmo_add_coordniates(&m_gizmo_renderer);
    m_gizmo_renderer.shapes[mesh_gizmos_translate].base_colour = dim;

    mesh_light_pos_gizmos = se_gizmo_add_coordniates(&m_gizmo_renderer);
    m_gizmo_renderer.shapes[mesh_light_pos_gizmos].base_colour = lit;

    point_light_1 = se_render3d_add_point_light_ext(&m_renderer, 1.0f, 0.14f, 0.07f);
    point_light_2 = se_render3d_add_point_light_ext(&m_renderer, 1.0f, 0.14f, 0.07f);
    point_light_3 = se_render3d_add_point_light_ext(&m_renderer, 1.0f, 0.14f, 0.07f);
    point_light_4 = se_render3d_add_point_light_ext(&m_renderer, 1.0f, 0.14f, 0.07f);
    current_obj_aabb = se_render3d_add_mesh_empty(&m_renderer);

    mesh_cube = se_render3d_load_mesh(&m_renderer, "core/meshes/cube.fbx", false);
    mesh_cube_tiny = se_render3d_load_mesh(&m_renderer, "core/meshes/cube_tiny.obj", false);
    m_renderer.user_meshes[mesh_cube_tiny]->should_cast_shadow = false;

    mesh_demo_crate = se_render3d_load_mesh(&m_renderer, "game/meshes/demo/Crate/Wooden Crate.obj", false);

    mesh_demo_diamond = se_render3d_load_mesh(&m_renderer, "game/meshes/demo/Diamond/diamond.obj", false);
    {
        const char *vsd[2] = {
            "core/shaders/3D/lit_header.vsd",
            "core/shaders/3D/lit.vsd"
        };

        const char *fsd[2] = {
            "core/shaders/3D/lit_header.fsd",
            "game/shaders/diamond.fsd"
        };
        diamond_shader = se_render3d_add_shader(&m_renderer, vsd, 2, fsd, 2, NULL, 0);
    }
}

void App::util_create_default_scene() {

        //- SOULSPEAR
    u32 soulspear = m_level.add_entity();
    m_level.entities.mesh_index[soulspear] = mesh_soulspear;
    m_level.entities.has_mesh[soulspear] = true;
    m_level.entities.should_render_mesh[soulspear] = true;
    m_level.entities.has_name[soulspear] = true;
    se_string_init(&m_level.entities.name[soulspear], "soulspear_entity");
    m_level.entities.position[soulspear] = v3f(3, 1, 0);

#if 1        //- PLAYER
    u32 guy = m_level.add_entity();
    m_level.entities.mesh_index[guy] = mesh_guy;
    m_level.entities.has_mesh[guy] = true;
    m_level.entities.should_render_mesh[guy] = true;
    m_level.entities.has_name[guy] = true;
    se_string_init(&m_level.entities.name[guy], "guy");
    m_level.entities.position[guy] = v3f(0, 0, 0);
    m_level.entities.scale[guy]    = v3f(0.1f, 0.1f, 0.1f);
#endif
        //- FLOOR
    u32 plane = m_level.add_entity();
    m_level.entities.mesh_index[plane] = mesh_plane;
    m_level.entities.has_mesh[plane] = true;
    m_level.entities.should_render_mesh[plane] = true;
    m_level.entities.has_name[plane] = true;
    se_string_init(&m_level.entities.name[plane], "plane_entity");

        //- POINT LIGHT ENTITY
    u32 point_light_1_entity = m_level.add_entity();
    m_level.entities.has_name           [point_light_1_entity] = true;
    se_string_init(&m_level.entities.name[point_light_1_entity], "light1");
    m_level.entities.position           [point_light_1_entity] = v3f(0, 1, 0);
    m_level.entities.has_light          [point_light_1_entity] = true;
    m_level.entities.light_index        [point_light_1_entity] = point_light_1;

        //- CUBE
    u32 cube_entity = m_level.add_entity();
    m_level.entities.has_name[cube_entity] = false;
    m_level.entities.position[cube_entity] = v3f(0, 0.5f, 1);
    m_level.entities.has_mesh[cube_entity] = true;
    m_level.entities.should_render_mesh[cube_entity] = true;
    m_level.entities.mesh_index[cube_entity] = mesh_cube;
}

void App::util_create_scene_from_image(const char *filepath) {
    {   //- Ground
        u32 entity = m_level.add_entity();
        m_level.entities.mesh_index[entity] = mesh_plane;
        m_level.entities.has_mesh[entity] = true;
        m_level.entities.should_render_mesh[entity] = true;
        m_level.entities.position[entity] = v3f(5, -1, 5);
        m_level.entities.scale[entity] = v3f(11, 1, 11);
    }

    {   //- Player
        u32 player = m_level.add_player();
        m_level.entities.mesh_index[player] = mesh_guy;
        m_level.entities.has_mesh[player] = true;
        m_level.entities.should_render_mesh[player] = true;
        m_level.entities.position[player] = v3f(5, 0, 5);
        // @temp
        m_level.entities.scale[player] = v3f(0.05f,0.05f,0.05f);
    }

    {   //- Lights
        u32 light_1 = m_level.add_entity();
        u32 light_2 = m_level.add_entity();
        u32 light_3 = m_level.add_entity();
        u32 light_4 = m_level.add_entity();
        m_level.entities.has_light[light_1] = true;
        m_level.entities.has_light[light_2] = true;
        m_level.entities.has_light[light_3] = true;
        m_level.entities.has_light[light_4] = true;

        m_level.entities.light_index[light_1] = point_light_1;
        m_level.entities.light_index[light_2] = point_light_2;
        m_level.entities.light_index[light_3] = point_light_3;
        m_level.entities.light_index[light_4] = point_light_4;

        m_level.entities.has_name[light_1] = true;
        m_level.entities.has_name[light_2] = true;
        m_level.entities.has_name[light_3] = true;
        m_level.entities.has_name[light_4] = true;
        se_string_init(&m_level.entities.name[light_1], "light_1");
        se_string_init(&m_level.entities.name[light_2], "light_2");
        se_string_init(&m_level.entities.name[light_3], "light_3");
        se_string_init(&m_level.entities.name[light_4], "light_4");

        m_level.entities.aabb[light_1] = {v3f(-0.5f, -0.5f, -0.5f), v3f(0.5f, 0.5f, 0.5f)};
        m_level.entities.aabb[light_2] = {v3f(-0.5f, -0.5f, -0.5f), v3f(0.5f, 0.5f, 0.5f)};
        m_level.entities.aabb[light_3] = {v3f(-0.5f, -0.5f, -0.5f), v3f(0.5f, 0.5f, 0.5f)};
        m_level.entities.aabb[light_4] = {v3f(-0.5f, -0.5f, -0.5f), v3f(0.5f, 0.5f, 0.5f)};

        m_level.entities.has_mesh[light_1] = true;
        m_level.entities.has_mesh[light_2] = true;
        m_level.entities.has_mesh[light_3] = true;
        m_level.entities.has_mesh[light_4] = true;

        m_level.entities.should_render_mesh[light_1] = true;
        m_level.entities.should_render_mesh[light_2] = true;
        m_level.entities.should_render_mesh[light_3] = true;
        m_level.entities.should_render_mesh[light_4] = true;

        m_level.entities.mesh_index[light_1] = mesh_cube_tiny;
        m_level.entities.mesh_index[light_2] = mesh_cube_tiny;
        m_level.entities.mesh_index[light_3] = mesh_cube_tiny;
        m_level.entities.mesh_index[light_4] = mesh_cube_tiny;
    }

    {   //- Diamond
        u32 diamond = m_level.add_entity();
        m_level.entities.mesh_index[diamond] = mesh_demo_diamond;
        m_level.entities.has_mesh[diamond] = true;
        m_level.entities.should_render_mesh[diamond] = true;
        m_level.entities.position[diamond] = v3f(5, 0, 5);
        // m_renderer.user_materials[m_renderer.user_meshes[mesh_demo_diamond]->material_index]->type = SE_MATERIAL_TYPE_CUSTOM;
        // m_renderer.user_materials[m_renderer.user_meshes[mesh_demo_diamond]->material_index]->shader_index = diamond_shader;
    }

    {   //- Entities from image
        SE_Image image;
        se_image_load_ext(&image, filepath, 1);

        SE_Grid grid;
        se_grid_init(&grid, image.width, image.height);

        for (u32 x = 0; x < grid.w; ++x) {
            for (u32 y = 0; y < grid.h; ++y) {
                u32 index = y * grid.w + x;
                se_grid_set(&grid, x, y, (u32)image.data[index]);

                if (se_grid_get(&grid, x, y) != 0) {
                    u32 entity_index = m_level.add_entity();
                    m_level.entities.mesh_index[entity_index] = mesh_demo_crate;
                    m_level.entities.has_mesh[entity_index] = true;
                    m_level.entities.should_render_mesh[entity_index] = true;
                    m_level.entities.position[entity_index] = v3f(
                        x * m_level.cell_size,
                        0,
                        y * m_level.cell_size
                    );
                }
            }
        }

        se_grid_deinit(&grid);
        se_image_unload(&image);
    }
}

///
///     GAME MODE
///

void App::util_update_game_mode(f32 delta_time) {
        //- Camera
    se_camera3d_input(&m_cameras[main_camera], &m_input, delta_time);

            //- Entities
    m_level.entities.update(&m_renderer, delta_time);
    se_animation_update(&animation, delta_time);
    se_skeleton_calculate_pose(m_renderer.user_meshes[mesh_guy]->skeleton, animation.current_frame);

        //- PLAYER MOVEMENT
    if (m_level.m_player) {
        if (se_input_is_key_pressed(&m_input, SDL_SCANCODE_RIGHT)) {
            m_level.m_player->move(vec3_right());
        }
        if (se_input_is_key_pressed(&m_input, SDL_SCANCODE_LEFT)) {
            m_level.m_player->move(vec3_left());
        }
        if (se_input_is_key_pressed(&m_input, SDL_SCANCODE_UP)) {
            m_level.m_player->move(vec3_forward());
        }
        if (se_input_is_key_pressed(&m_input, SDL_SCANCODE_DOWN)) {
            m_level.m_player->move(vec3_backward());
        }
    }

        //- UI
          // switch to engine mode
    if (seui_button_at(ctx, "engine mode", {0, 0, 200, 32})) {
        util_switch_mode(GAME_MODES::ENGINE);
    }
}

void App::util_render_game_mode() {

}

///
///     ENGINE MODE
///

void App::util_update_engine_mode(f32 delta_time) {
        //- CAMERA MOVEMENT
    se_camera3d_input(&m_cameras[main_camera], &m_input, delta_time);


        //- Entities
    m_level.entities.update(&m_renderer, delta_time);
#if 1
    se_animation_update(&animation, delta_time);
    se_skeleton_calculate_pose(m_renderer.user_meshes[mesh_guy]->skeleton, animation.current_frame);
#endif

        // select entities
    if (se_input_is_mouse_left_released(&m_input) && se_input_is_key_down(&m_input, SDL_SCANCODE_LCTRL)) {
        m_selected_entity = this->raycast_to_select_entity();
        m_widget_entity.entity = m_selected_entity;
    }

    if (m_selected_entity >= 0) {   // @temp
        Vec3 dir;
        Vec3 origin;
        se_camera3d_get_raycast(&m_cameras[main_camera], m_window, &dir, &origin);
        if (ray_overlaps_sphere(origin, dir, 100, m_level.entities.position[m_selected_entity], 0.5f, NULL)) {
            m_gizmo_renderer.shapes[mesh_gizmos_translate].base_colour = lit;
        } else {
            m_gizmo_renderer.shapes[mesh_gizmos_translate].base_colour = dim;
        }
    }

        //- UI
    m_widget_entity.construct_panel(ctx, &m_renderer);
    m_selected_entity = m_widget_entity.entity;

        // make entity widget pop up
    // if (se_input_is_key_pressed(&m_input, SDL_SCANCODE_SPACE)) {
    //     m_widget_entity.toggle_visibility(ctx);
    // }

        // switch to game mode
    if (seui_button_at(ctx, "game mode", {0, 0, 200, 32})) {
        util_switch_mode(GAME_MODES::GAME);
    }
        // save
    if (seui_button_at(ctx, "save", {200, 0, 128, 32})) {
        this->save();
    }
        // save camera settings
    if (seui_button_at(ctx, "save camera", {128+200, 0, 200, 32})) {
        Assets::update_level_camera_settings(&m_level, m_cameras[main_camera]);
    }

    // @debug
    seui_texture_viewer(ctx, m_renderer.shadow_render_target.colour_buffers[0]);
    seui_texture_viewer(ctx, m_render_target_scene.colour_buffers[0]);

    // seui_grid_editor(ctx, &grid, value_mappings);

    sprintf(fps_text, "%f", fps);
    seui_label_at(ctx, fps_text, {128+200+200, 0, 200, 32});
}

void App::util_render_engine_mode() {
        //- aabb of selected entity
    if (m_selected_entity >= 0) {
        se_mesh_generate_gizmos_aabb(m_renderer.user_meshes[current_obj_aabb],
            m_level.entities.aabb[m_selected_entity].min,
            m_level.entities.aabb[m_selected_entity].max,
            2);

        se_render_mesh_index(&m_renderer, current_obj_aabb, m_level.entities.transform[m_selected_entity]);
    }

        //- skeleton mesh
#if 1   // @debug
    se_render_mesh_index(&m_renderer, mesh_skeleton, m_level.entities.transform[mesh_guy]);
#endif
    se_render_mesh_index(&m_renderer, world_aabb_mesh, mat4_identity());

        //- Gizmos
    glClear(GL_DEPTH_BUFFER_BIT);

        //- selected entity
    if (m_selected_entity >= 0 && m_selected_entity < m_level.entities.count) {
            // calculate transfrom without scale
        Vec3 pos   = m_level.entities.position[m_selected_entity];
        Vec3 rot   = m_level.entities.oriantation[m_selected_entity];

        Mat4 transform = mat4_identity();
        transform = mat4_mul(transform, mat4_euler_xyz(rot.x, rot.y, rot.z));
        transform = mat4_mul(transform, mat4_translation(pos));

        se_gizmo_render_index(&m_gizmo_renderer, mesh_gizmos_translate, transform);
    }

#if 0      //- DEBUG RENDERING
    se_render_mesh_index(&m_renderer, debug_raycast_visual, mat4_identity());
    {
        Vec3 pos = m_renderer.light_directional.calculated_position;
        Mat4 transform = mat4_identity();
        transform = mat4_mul(transform, mat4_translation(pos));
        se_gizmo_render_index(&m_gizmo_renderer, mesh_light_pos_gizmos, transform);
    }
#endif
}

void App::util_switch_mode(GAME_MODES mode) {
    m_has_queued_for_change_of_mode = true;
    m_queued_mode = mode;
}