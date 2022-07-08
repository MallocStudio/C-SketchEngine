#pragma once
#include "game.hpp"
//! This is meant to be included in game.cpp ONLY. Nowhere else. So this has to be included once.

///
///     General variables used in game.cpp
///


SE_UI *ctx;

    //@temp a temporary way of loading required meshes once at init_application() time
u32 mesh_soulspear = -1;
u32 mesh_plane = -1;
u32 mesh_guy = -1;
u32 mesh_skeleton = -1;
u32 mesh_gizmos_translate = -1;
u32 mesh_light_pos_gizmos = -1;
u32 current_obj_aabb = -1;
u32 world_aabb_mesh = -1;
u32 point_light_1 = -1;

SE_Animation animation;

RGBA dim = {100, 100, 100, 255};
RGBA lit = {255, 255, 255, 255};

    //@debug
u32 debug_raycast_visual = -1;

u32 main_camera = -1;

///
///     UTIL FUNCTIONALITIES
///

void App::util_load_meshes_from_disk() {
        // @temp TODO: MOVE TO LOADER
    mesh_soulspear = se_render3d_load_mesh(&m_renderer, "game/meshes/soulspear/soulspear.obj", false);

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

    mesh_plane = se_render3d_add_plane(&m_renderer, v3f(10, 10, 10));

    mesh_gizmos_translate = se_gizmo_add_coordniates(&m_gizmo_renderer);
    m_gizmo_renderer.shapes[mesh_gizmos_translate].base_colour = dim;

    mesh_light_pos_gizmos = se_gizmo_add_coordniates(&m_gizmo_renderer);
    m_gizmo_renderer.shapes[mesh_light_pos_gizmos].base_colour = lit;

    point_light_1 = se_render3d_add_point_light(&m_renderer);
    current_obj_aabb = se_render3d_add_mesh_empty(&m_renderer);
}

void App::util_create_default_scene() {
    u32 soulspear = m_level.add_entity();
    m_level.entities.mesh_index[soulspear] = mesh_soulspear;
    m_level.entities.has_mesh[soulspear] = true;
    m_level.entities.should_render_mesh[soulspear] = true;
    m_level.entities.has_name[soulspear] = true;
    se_string_init(&m_level.entities.name[soulspear], "soulspear_entity");
    m_level.entities.position[soulspear] = v3f(3, 1, 0);

    u32 guy = m_level.add_entity();
    m_level.entities.mesh_index[guy] = mesh_guy;
    m_level.entities.has_mesh[guy] = true;
    m_level.entities.should_render_mesh[guy] = true;
    m_level.entities.has_name[guy] = true;
    se_string_init(&m_level.entities.name[guy], "guy");
    m_level.entities.position[guy] = v3f(0, 0, 0);
    m_level.entities.scale[guy]    = v3f(0.1f, 0.1f, 0.1f);

    u32 plane = m_level.add_entity();
    m_level.entities.mesh_index[plane] = mesh_plane;
    m_level.entities.has_mesh[plane] = true;
    m_level.entities.should_render_mesh[plane] = true;
    m_level.entities.has_name[plane] = true;
    se_string_init(&m_level.entities.name[plane], "plane_entity");

    u32 point_light_1_entity = m_level.add_entity();
    m_level.entities.has_name           [point_light_1_entity] = true;
    se_string_init(&m_level.entities.name[point_light_1_entity], "light1");
    m_level.entities.position           [point_light_1_entity] = v3f(0, 1, 0);
    m_level.entities.has_light          [point_light_1_entity] = true;
    m_level.entities.light_index        [point_light_1_entity] = point_light_1;
}