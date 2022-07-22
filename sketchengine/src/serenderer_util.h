#ifndef SE_RENDERER_3D_UTIL
#define SE_RENDERER_3D_UTIL
//! ONLY MEANT TO BE INCLUDED IN serenderer.c AND NO WHERE ELSE.

///
///     INCLUDES
///

#include "serenderer.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "sestring.h"

#include "seinput.h" // for camera
#include "stdio.h" // for file management

///
///     DEFINES
///

#define default_normal_filepath "core/textures/default_normal.png"
#define default_diffuse_filepath "core/textures/default_diffuse.png"
#define default_specular_filepath "core/textures/default_specular.png"

#define shader_filename_lit_header_vsd "core/shaders/3D/lit_header.vsd"
#define shader_filename_lit_vsd "core/shaders/3D/lit.vsd"
#define shader_filename_lit_header_fsd "core/shaders/3D/lit_header.fsd"
#define shader_filename_lit_fsd "core/shaders/3D/lit.fsd"

#define shader_filename_shadow_calc_directional_vsd "core/shaders/3D/shadow_calc/shadow_calc.vsd"
#define shader_filename_shadow_calc_directional_fsd "core/shaders/3D/shadow_calc/shadow_calc.fsd"
#define shader_filename_shadow_calc_directional_skinned_mesh_vsd "core/shaders/3D/shadow_calc/shadow_calc_skinned_mesh.vsd"
#define shader_filename_shadow_calc_omnidir_vsd "core/shaders/3D/shadow_calc/shadow_omni_calc.vsd"
#define shader_filename_shadow_calc_omnidir_fsd "core/shaders/3D/shadow_calc/shadow_omni_calc.fsd"
#define shader_filename_shadow_calc_omnidir_gsd "core/shaders/3D/shadow_calc/shadow_omni_calc.gsd"
#define shader_filename_shadow_calc_omnidir_skinned_mesh_vsd "core/shaders/3D/shadow_calc/shadow_omni_calc_skinned_mesh.vsd"

#define shader_filename_lines_vsd "core/shaders/3D/lines.vsd"
#define shader_filename_lines_fsd "core/shaders/3D/lines.fsd"
#define shader_filename_outline_vsd "core/shaders/3D/outline.vsd"
#define shader_filename_outline_fsd "core/shaders/3D/outline.fsd"
#define shader_filename_sprite_vsd "core/shaders/3D/sprite.vsd"
#define shader_filename_sprite_fsd "core/shaders/3D/sprite.fsd"
#define shader_filename_lit_skinned_vsd "core/shaders/3D/skinned_vertex.vsd"
#define shader_filename_skeleton_vsd "core/shaders/3D/skinned_skeleton_lines.vsd"

#define shader_filename_post_process_header_vsd "core/shaders/post_process/post_process_header.vsd"
#define shader_filename_post_process_header_fsd "core/shaders/post_process/post_process_header.fsd"
#define shader_filename_post_process_tonemap "core/shaders/post_process/post_process_tonemap.fsd"
#define shader_filename_post_process_blur "core/shaders/post_process/post_process_blur.fsd"
#define shader_filename_post_process_downsample "core/shaders/post_process/post_process_downsample.fsd"
#define shader_filename_post_process_upsample "core/shaders/post_process/post_process_upsample.fsd"
#define shader_filename_post_process_bloom "core/shaders/post_process/post_process_bloom.fsd"
#define shader_filename_post_process_gaussian "core/shaders/post_process/post_process_gaussian_blur.fsd"

static void set_material_uniforms_lit(SE_Renderer3D *renderer, SE_Shader *shader, const SE_Material *material, Mat4 transform) {
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    // the good old days when debugging:
    // material->texture_diffuse.width = 100;
    /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
    se_shader_set_uniform_mat4(shader, "model_matrix", transform);
    se_shader_set_uniform_vec3(shader, "camera_pos", renderer->current_camera->position);
    se_shader_set_uniform_mat4(shader, "light_space_matrix", renderer->light_space_matrix);

    /* material uniforms */
    se_shader_set_uniform_f32 (shader, "material.shininess", 0.1f);
    se_shader_set_uniform_i32 (shader, "material.diffuse", 0);
    se_shader_set_uniform_i32 (shader, "material.specular", 1);
    se_shader_set_uniform_i32 (shader, "material.normal", 2);

    Vec4 base_diffuse_linear_space = {
        se_math_power(material->base_diffuse.x, renderer->gamma),
        se_math_power(material->base_diffuse.y, renderer->gamma),
        se_math_power(material->base_diffuse.z, renderer->gamma),
        se_math_power(material->base_diffuse.w, renderer->gamma)
    };
    se_shader_set_uniform_vec4(shader, "material.base_diffuse", base_diffuse_linear_space);

    // directional light uniforms
    se_shader_set_uniform_vec3(shader, "dir_light.direction", renderer->light_directional.direction);
    se_shader_set_uniform_rgb (shader, "dir_light.ambient", renderer->light_directional.ambient);
    se_shader_set_uniform_rgb (shader, "dir_light.diffuse", renderer->light_directional.diffuse);
    se_shader_set_uniform_rgb (shader, "dir_light.specular", (RGB) {0, 0, 0});
    se_shader_set_uniform_f32 (shader, "dir_light.intensity", renderer->light_directional.intensity);
    se_shader_set_uniform_i32 (shader, "shadow_map", 3);

    // point light uniforms
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].position", i);
        se_shader_set_uniform_vec3(shader, buf, renderer->point_lights[i].position);
        SDL_snprintf(buf, 100, "point_lights[%i].ambient", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].ambient);
        SDL_snprintf(buf, 100, "point_lights[%i].diffuse", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].diffuse);
        SDL_snprintf(buf, 100, "point_lights[%i].specular", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].specular);
        SDL_snprintf(buf, 100, "point_lights[%i].constant", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].constant);
        SDL_snprintf(buf, 100, "point_lights[%i].linear", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].linear);
        SDL_snprintf(buf, 100, "point_lights[%i].quadratic", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].quadratic);
        SDL_snprintf(buf, 100, "point_lights[%i].far_plane", i);
        se_shader_set_uniform_f32 (shader, buf, 25.0f); // @temp magic value set to the projection far plane when calculating the shadow maps (cube texture)
    }
    se_shader_set_uniform_i32 (shader, "num_of_point_lights", renderer->point_lights_count);

    /* textures */
    // Note that by defaut meshes point to SE_DEFAULT_MATERIAL_INDEX, so by default it'll have
    // the default textures.
    if (material->texture_diffuse.loaded) {
        se_texture_bind(&material->texture_diffuse, 0);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_diffuse, 0);
    }

    if (material->texture_specular.loaded) {
        se_texture_bind(&material->texture_specular, 1);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_specular, 1);
    }

    if (material->texture_normal.loaded) {
        se_texture_bind(&material->texture_normal, 2);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_normal, 2);
    }

        // - Directional Shadow Map
    glActiveTexture(GL_TEXTURE0 + 3); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.colour_buffers[0]); // @TODO maybe change to depth buffer

        //- Omnidirectional Shadow Map
    for (u32 i = 0; i < SERENDERER3D_MAX_POINT_LIGHTS; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].shadow_map", i);
        se_shader_set_uniform_i32 (shader, buf, 4+i);
    }
        // ! NOTE: Might want to consider merging the below for loop with the above. I'm not sure which one
        // ! has what kind of a performance impact.
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        glActiveTexture(GL_TEXTURE0 + 4+i); // shadow map
        glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->point_lights[i].depth_cube_map);
    }
}

static void set_material_uniforms_skeleton
(SE_Renderer3D *renderer, SE_Shader *shader, const SE_Material *material, Mat4 transform, Mat4 *final_pose) {
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

     /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
    se_shader_set_uniform_mat4(shader, "model_matrix", transform);

    /* material uniforms */
    se_shader_set_uniform_vec3 (shader, "base_diffuse", v3f(1, 0, 0));
    se_shader_set_uniform_mat4_array(shader, "bones", final_pose, SE_SKELETON_BONES_CAPACITY);
}

static void set_material_uniforms_lines
(SE_Renderer3D *renderer, SE_Shader *shader, const SE_Material *material, Mat4 transform) {
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
}

static void set_material_uniforms_sprite
(SE_Renderer3D *renderer, SE_Shader *shader, const SE_Material *material, Mat4 transform) {
    se_shader_use(shader);

    /* always look at the camera */
    // Mat4 look_at_camera = mat4_lookat(mat4_get_translation(transform), renderer->current_camera->position, renderer->current_camera->up);
    // // transform = mat4_mul(transform, look_at_camera);
    // transform = mat4_mul(mat4_translation(mat4_get_translation(transform)), look_at_camera);
    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);

    /* material */
    Vec4 base_diffuse_linear_space = {
        se_math_power(material->base_diffuse.x, renderer->gamma),
        se_math_power(material->base_diffuse.y, renderer->gamma),
        se_math_power(material->base_diffuse.z, renderer->gamma),
        se_math_power(material->base_diffuse.w, renderer->gamma)
    };

    se_shader_set_uniform_vec4(shader, "base_diffuse", base_diffuse_linear_space);
    se_shader_set_uniform_i32(shader, "sprite_texture", 0);

    /* textures */
    if (material->sprite.texture.loaded) {
        se_texture_bind(&material->sprite.texture, 0);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_diffuse, 0);
    }
}

static void load_animation_from_file(SE_Skeleton *skeleton, const char *model_filepath) {
        // load scene from file
    const struct aiScene *scene = aiImportFile(model_filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (scene == NULL) {
        printf("ERROR: could not mesh from %s (%s)\n", model_filepath, aiGetErrorString());
        return;
    }

    load_animation(skeleton, scene);

    aiReleaseImport(scene);
}

static void
set_material_uniforms_skinned
(SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform, Mat4 *final_pose) {
    SE_Shader *shader = renderer->user_shaders[renderer->shader_skinned_mesh];
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

     /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
    se_shader_set_uniform_mat4(shader, "model_matrix", transform);
    se_shader_set_uniform_vec3(shader, "camera_pos", renderer->current_camera->position);
    se_shader_set_uniform_mat4(shader, "light_space_matrix", renderer->light_space_matrix);

    /* material uniforms */
    se_shader_set_uniform_f32 (shader, "material.shininess", 0.1f);
    se_shader_set_uniform_i32 (shader, "material.diffuse", 0);
    se_shader_set_uniform_i32 (shader, "material.specular", 1);
    se_shader_set_uniform_i32 (shader, "material.normal", 2);

    Vec4 base_diffuse_linear_space = {
        se_math_power(material->base_diffuse.x, renderer->gamma),
        se_math_power(material->base_diffuse.y, renderer->gamma),
        se_math_power(material->base_diffuse.z, renderer->gamma),
        se_math_power(material->base_diffuse.w, renderer->gamma)
    };
    se_shader_set_uniform_vec4(shader, "material.base_diffuse", base_diffuse_linear_space);

    // directional light uniforms
    se_shader_set_uniform_vec3(shader, "dir_light.direction", renderer->light_directional.direction);
    se_shader_set_uniform_rgb (shader, "dir_light.ambient", renderer->light_directional.ambient);
    se_shader_set_uniform_rgb (shader, "dir_light.diffuse", renderer->light_directional.diffuse);
    se_shader_set_uniform_rgb (shader, "dir_light.specular", (RGB) {0, 0, 0});
    se_shader_set_uniform_f32 (shader, "dir_light.intensity", renderer->light_directional.intensity);
    se_shader_set_uniform_i32 (shader, "shadow_map", 3);

    // point light uniforms
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].position", i);
        se_shader_set_uniform_vec3(shader, buf, renderer->point_lights[i].position);
        SDL_snprintf(buf, 100, "point_lights[%i].ambient", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].ambient);
        SDL_snprintf(buf, 100, "point_lights[%i].diffuse", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].diffuse);
        SDL_snprintf(buf, 100, "point_lights[%i].specular", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].specular);
        SDL_snprintf(buf, 100, "point_lights[%i].constant", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].constant);
        SDL_snprintf(buf, 100, "point_lights[%i].linear", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].linear);
        SDL_snprintf(buf, 100, "point_lights[%i].quadratic", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].quadratic);
        SDL_snprintf(buf, 100, "point_lights[%i].far_plane", i);
        se_shader_set_uniform_f32 (shader, buf, 25.0f); // @temp magic value set to the projection far plane when calculating the shadow maps (cube texture)
    }
    se_shader_set_uniform_i32 (shader, "num_of_point_lights", renderer->point_lights_count);

    /* textures */
    // Note that by defaut meshes point to SE_DEFAULT_MATERIAL_INDEX, so by default it'll have
    // the default textures.
    if (material->texture_diffuse.loaded) {
        se_texture_bind(&material->texture_diffuse, 0);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_diffuse, 0);
    }

    if (material->texture_specular.loaded) {
        se_texture_bind(&material->texture_specular, 1);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_specular, 1);
    }

    if (material->texture_normal.loaded) {
        se_texture_bind(&material->texture_normal, 2);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_normal, 2);
    }

    glActiveTexture(GL_TEXTURE0 + 3); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.colour_buffers[0]); // @TODO maybe change to depth buffer

       //- Omnidirectional Shadow Map
    for (u32 i = 0; i < SERENDERER3D_MAX_POINT_LIGHTS; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].shadow_map", i);
        se_shader_set_uniform_i32 (shader, buf, 4+i);
    }
        // ! NOTE: Might want to consider merging the below for loop with the above. I'm not sure which one
        // ! has what kind of a performance impact.
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        glActiveTexture(GL_TEXTURE0 + 4+i); // shadow map
        glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->point_lights[i].depth_cube_map);
    }

    se_shader_set_uniform_mat4_array(shader, "bones", final_pose, SE_SKELETON_BONES_CAPACITY);
}

static void recursive_render_directional_shadow_map_for_mesh
(SE_Renderer3D *renderer, u32 mesh_index, Mat4 model_mat, Mat4 light_space_mat) {
    SE_Mesh *mesh = renderer->user_meshes[mesh_index];

    if (mesh->should_cast_shadow) {
        if (mesh->type == SE_MESH_TYPE_NORMAL) {
            se_shader_use(renderer->user_shaders[renderer->shader_shadow_calc]);
            se_shader_set_uniform_mat4(renderer->user_shaders[renderer->shader_shadow_calc], "light_space_matrix", light_space_mat);
            se_shader_set_uniform_mat4(renderer->user_shaders[renderer->shader_shadow_calc], "model", model_mat);
        } else
        if (mesh->type == SE_MESH_TYPE_SKINNED) {
            se_shader_use(renderer->user_shaders[renderer->shader_shadow_calc_skinned_mesh]);
            se_shader_set_uniform_mat4(renderer->user_shaders[renderer->shader_shadow_calc_skinned_mesh], "light_space_matrix", light_space_mat);
            se_shader_set_uniform_mat4(renderer->user_shaders[renderer->shader_shadow_calc_skinned_mesh], "model", model_mat);
            se_shader_set_uniform_mat4_array(renderer->user_shaders[renderer->shader_shadow_calc_skinned_mesh], "bones", mesh->skeleton->final_pose, SE_SKELETON_BONES_CAPACITY);
        }

        glBindVertexArray(mesh->vao);
        if (mesh->indexed) {
            glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh->element_count);
        }
    }

        // continue for children meshes if they exist
    if (mesh->next_mesh_index >= 0) {
        recursive_render_directional_shadow_map_for_mesh(renderer, mesh->next_mesh_index, model_mat, light_space_mat);
    }
}

static void recursive_render_omnidir_shadow_map_for_mesh
(SE_Renderer3D *renderer, u32 mesh_index, Mat4 model_mat, SE_Light_Point *point_light, Mat4 shadow_transforms[6], f32 far) {
    SE_Mesh *mesh = renderer->user_meshes[mesh_index];

    if (mesh->should_cast_shadow) {
        // configure shader
        SE_Shader *shader = renderer->user_shaders[renderer->shader_shadow_omnidir_calc];
        if (mesh->type == SE_MESH_TYPE_SKINNED) {
            shader = renderer->user_shaders[renderer->shader_shadow_omnidir_calc_skinned_mesh];
        }

        se_shader_use(shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, point_light->depth_cube_map);
        se_shader_set_uniform_f32 (shader, "far_plane", far);
        se_shader_set_uniform_vec3(shader, "light_pos", point_light->position);
        se_shader_set_uniform_mat4(shader, "shadow_matrices[0]", shadow_transforms[0]);
        se_shader_set_uniform_mat4(shader, "shadow_matrices[1]", shadow_transforms[1]);
        se_shader_set_uniform_mat4(shader, "shadow_matrices[2]", shadow_transforms[2]);
        se_shader_set_uniform_mat4(shader, "shadow_matrices[3]", shadow_transforms[3]);
        se_shader_set_uniform_mat4(shader, "shadow_matrices[4]", shadow_transforms[4]);
        se_shader_set_uniform_mat4(shader, "shadow_matrices[5]", shadow_transforms[5]);
        se_shader_set_uniform_mat4(shader, "model", model_mat);

        if (mesh->type == SE_MESH_TYPE_SKINNED) {
            se_shader_set_uniform_mat4(renderer->user_shaders[renderer->shader_shadow_omnidir_calc_skinned_mesh], "model", model_mat);
            se_shader_set_uniform_mat4_array(renderer->user_shaders[renderer->shader_shadow_omnidir_calc_skinned_mesh], "bones", mesh->skeleton->final_pose, SE_SKELETON_BONES_CAPACITY);
        }

        glBindVertexArray(mesh->vao);
        if (mesh->indexed) {
            glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh->element_count);
        }
    }

        // continue for children meshes if they exist
    if (mesh->next_mesh_index >= 0) {
        recursive_render_omnidir_shadow_map_for_mesh(renderer, mesh->next_mesh_index, model_mat, point_light, shadow_transforms, far);
    }
}

#endif // SE_RENDERER_3D_UTIL