#include "serenderer_util.h"

u32 se_render3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath, b8 with_skeleton) {
    u32 result = -1;

    // check if a generated mesh file has already been generated for this mesh.
    // if not generate it and load it.
    {
        FILE *file;
        SE_String save_data_filepath;
        se_string_init(&save_data_filepath, model_filepath);
        se_string_append(&save_data_filepath, ".mesh");

        if (file = fopen(save_data_filepath.buffer, "rb")) {
            fclose(file);
            // file was found. So we don't need to regenrate it.
            printf("file: %s has already been generated.\n", model_filepath);
        } else {
            printf("file: %s has NOT been generated. So we're generating it.\n", model_filepath);
            // file was not found. So we need to generate a mesh file.
                // load scene from file
            const struct aiScene *scene = aiImportFile(model_filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

            if (scene == NULL) {
                printf("ERROR: could not mesh from %s (%s)\n", model_filepath, aiGetErrorString());
                return result;
            }

            {    //- Trun scene into a save file
                SE_Save_Data_Meshes save_data = {0};
                ai_scene_to_mesh_save_data(scene, &save_data, model_filepath);
                    //- Save to disk for later use
                se_save_data_write_mesh(&save_data, save_data_filepath.buffer);
                se_save_data_mesh_deinit(&save_data);
            }

            aiReleaseImport(scene);
        }

        se_string_deinit(&save_data_filepath);
    }

    SE_String save_data_filepath;
        se_string_init(&save_data_filepath, model_filepath);
        se_string_append(&save_data_filepath, ".mesh");

        SE_Save_Data_Meshes save_data = {0};
        se_save_data_read_mesh(&save_data, save_data_filepath.buffer);
    se_string_deinit(&save_data_filepath);

        //- Generate meshes from save data
    result = se_save_data_mesh_to_mesh(renderer, &save_data);

    se_save_data_mesh_deinit(&save_data);
    return result;
}

u32 se_save_data_mesh_to_mesh
(SE_Renderer3D *renderer, const SE_Save_Data_Meshes *save_data) {
        //- Should we add a skeleton?
    SE_Skeleton *skeleton;
    if (save_data->meshes_count > 0 && save_data->meshes[0].skeleton_data != NULL) {
        u32 skeleton_index = se_render3d_add_skeleton(renderer);
        skeleton = renderer->user_skeletons[skeleton_index];
    }

    u32 result = renderer->user_meshes_count;

    for (u32 i = 0; i < save_data->meshes_count; ++i) {
        renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
        memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
        SE_Mesh *mesh = renderer->user_meshes[renderer->user_meshes_count];

        {   //- generate the mesh
                //- settings
            SE_Mesh_Raw_Data *raw_data = &save_data->meshes[i];
            mesh->next_mesh_index = -1;
            mesh->type = raw_data->type;

                //- generate vao
            if (mesh->type == SE_MESH_TYPE_SKINNED) {
                se_mesh_generate_skinned(mesh, raw_data->vert_count, raw_data->skinned_verts, raw_data->index_count, raw_data->indices);
            } else {
                se_mesh_generate(mesh, raw_data->vert_count, raw_data->verts, raw_data->index_count, raw_data->indices);
            }

                //- materials
            u32 material_index = se_render3d_add_material(renderer);
            mesh->material_index = material_index;

            SE_Material *material = renderer->user_materials[material_index];
            material->type = raw_data->material_type;
            material->shader_index = raw_data->material_shader_index;

                // override shader index if the material uses a built in type
            if (material->type == SE_MATERIAL_TYPE_LIT) {
                material->shader_index = renderer->shader_lit;
            }

            material->base_diffuse = (Vec4) {1, 1, 1, 1};
            material->base_diffuse = raw_data->base_diffuse;

            if (raw_data->texture_diffuse_filepath.buffer != NULL) {
                se_texture_load(&material->texture_diffuse,
                                raw_data->texture_diffuse_filepath.buffer,
                                SE_TEXTURE_LOAD_CONFIG_CONVERT_TO_LINEAR_SPACE);
            }

            if (raw_data->texture_specular_filepath.buffer != NULL) {
                se_texture_load(&material->texture_specular, raw_data->texture_specular_filepath.buffer, SE_TEXTURE_LOAD_CONFIG_NULL);
            }

            if (raw_data->texture_normal_filepath.buffer != NULL) {
                se_texture_load(&material->texture_normal, raw_data->texture_normal_filepath.buffer, SE_TEXTURE_LOAD_CONFIG_NULL);
            }

            mesh->line_width   = raw_data->line_width;
            mesh->point_radius = raw_data->point_radius;
            mesh->should_cast_shadow = raw_data->should_cast_shadow;

                //- skeleton and animations
            if (raw_data->skeleton_data != NULL) {
                mesh->skeleton = skeleton; // @TODO change to index like material
                skeleton_deep_copy(mesh->skeleton, raw_data->skeleton_data);
            }
        }

            // connect the link
        mesh->next_mesh_index = -1;
        if (i > 0) {
            renderer->user_meshes[renderer->user_meshes_count-1]->next_mesh_index = result + i;
        }
        renderer->user_meshes_count++;
    }

    return result;
}

void se_render_mesh(SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform) {
    se_render3d_reset_render_config(); // Reset configs to their default values
    // take the mesh (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    i32 primitive = GL_TRIANGLES;
    SE_Material *material = renderer->user_materials[mesh->material_index];
        //@temp assigning the shader to various indices based on mesh type.
        // rewrite this so that when we generate other meshes (such as plane, lines, sprites)...
        // we automatically assign the correct shader index to the material
    SE_Shader *shader = renderer->user_shaders[material->shader_index]; // this is being ignored except for normal and skinned meshes

    /* configs for this mesh */
    if (mesh->type == SE_MESH_TYPE_LINE) { // LINE
        primitive = GL_LINES;
        glLineWidth(mesh->line_width);
        if (mesh->skeleton != NULL && mesh->skeleton->animations_count > 0) {
        //- ANIMATED LINES
            shader = renderer->user_shaders[renderer->shader_skinned_mesh_skeleton];
                // used for animated skeleton
            set_material_uniforms_skeleton(renderer, shader, material, transform, mesh->skeleton->final_pose);
        } else {
        //- LINE
            shader = renderer->user_shaders[renderer->shader_lines];
                // render the line without animation
            set_material_uniforms_lines(renderer, shader, material, transform);
        }
    } else
    if (mesh->type == SE_MESH_TYPE_NORMAL) { // NORMAL
        //- STATIC MESH
        shader = renderer->user_shaders[renderer->shader_lit];
        set_material_uniforms_lit(renderer, shader, material, transform);
    } else
    if (mesh->type == SE_MESH_TYPE_SPRITE) { // SPRITE
        shader = renderer->user_shaders[renderer->shader_sprite];
        //- SPRITE
        set_material_uniforms_sprite(renderer, shader, material, transform);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
    } else
        //- SKINNED MESH
    if (mesh->type == SE_MESH_TYPE_SKINNED) { // SKELETAL ANIMATION
        shader = renderer->user_shaders[renderer->shader_skinned_mesh];
        set_material_uniforms_skinned(renderer, material, transform, mesh->skeleton->final_pose);
    } else
    if (mesh->type == SE_MESH_TYPE_POINT) { // MESH MADE OUT OF POINTS
        //- POINT
        shader = renderer->user_shaders[renderer->shader_lines];
        primitive = GL_POINTS;
        glPointSize(mesh->point_radius);
            // Note: points use the same shader as lines
        set_material_uniforms_lines(renderer, shader, material, transform);
    }

        //- Draw Call
    glBindVertexArray(mesh->vao);
    if (mesh->indexed) {
        glDrawElements(primitive, mesh->element_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->element_count);
    }

    glBindVertexArray(0);
}

// @remove
// void se_render_mesh_with_shader
// (SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform, u32 user_shader_index) {
//     se_render3d_reset_render_config(); // Reset configs to their default values
//     SE_Mesh *mesh = renderer->user_meshes[mesh_index];
//     SE_Shader *shader = renderer->user_shaders[user_shader_index];
//     SE_Material *material = renderer->user_materials[mesh->material_index];
//     se_shader_use(shader);
//     {
//         // Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
//         // pvm = mat4_mul(pvm, renderer->current_camera->projection);

//         // // the good old days when debugging:
//         // // material->texture_diffuse.width = 100;
//         // /* vertex */
//         // se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
//         // se_shader_set_uniform_mat4(shader, "model_matrix", transform);
//         // se_shader_set_uniform_vec3(shader, "camera_pos", renderer->current_camera->position);
//         // se_shader_set_uniform_mat4(shader, "light_space_matrix", renderer->light_space_matrix);
//     }
//     set_material_uniforms_lit(renderer, shader, material, transform);

//     i32 primitive = GL_TRIANGLES;
//         //- LINE
//     if (mesh->type == SE_MESH_TYPE_LINE) { // LINE
//         primitive = GL_LINES;
//         glLineWidth(mesh->line_width);
//     } else
//         //- POINT
//     if (mesh->type == SE_MESH_TYPE_POINT) { // MESH MADE OUT OF POINTS
//         primitive = GL_POINTS;
//         glPointSize(mesh->point_radius);
//     }

//         //- Draw Call
//     glBindVertexArray(mesh->vao);
//     if (mesh->indexed) {
//         glDrawElements(primitive, mesh->element_count, GL_UNSIGNED_INT, 0);
//     } else {
//         glDrawArrays(primitive, 0, mesh->element_count);
//     }

//     glBindVertexArray(0);
// }

// make sure to call serender3d_render_mesh_setup before calling this procedure. Only needs to be done once.
void se_render_mesh_index(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->user_meshes[mesh_index];
    se_render_mesh(renderer, mesh, transform);

    // if (mesh->next_mesh_index > -1 && mesh->type != SE_MESH_TYPE_SKINNED) { // @temp checking if it's not skinned because for some reason the other mesh does not get a proper skeleton final pose
    if (mesh->next_mesh_index > -1) {
        se_render_mesh_index(renderer, mesh->next_mesh_index, transform);
    }
}

void se_render_post_process(SE_Renderer3D *renderer, SE_RENDER_POSTPROCESS post_process, const SE_Render_Target *previous_render_pass) {
    SE_Shader *shader;
    switch (post_process) {
        case SE_RENDER_POSTPROCESS_TONEMAP: {
            shader = renderer->user_shaders[renderer->shader_post_process_tonemap];
        } break;
        case SE_RENDER_POSTPROCESS_BLUR: {
            shader = renderer->user_shaders[renderer->shader_post_process_blur];
            se_shader_use(shader);
            se_shader_set_uniform_i32(shader, "texture_to_blur", 1); // get the bright colour channel from lit_footer.fsd
        } break;
        case SE_RENDER_POSTPROCESS_DOWNSAMPLE: {
            shader = renderer->user_shaders[renderer->shader_post_process_downsample];
            se_shader_use(shader);
            se_shader_set_uniform_vec2(shader, "src_resolution",
                v2f(renderer->viewport.w, renderer->viewport.h));

        } break;
        case SE_RENDER_POSTPROCESS_UPSAMPLE: {
            shader = renderer->user_shaders[renderer->shader_post_process_upsample];
            se_shader_use(shader);
            se_shader_set_uniform_f32(shader, "src_resolution", 3.0f);
        } break;
        case SE_RENDER_POSTPROCESS_BLOOM: {
            shader = renderer->user_shaders[renderer->shader_post_process_bloom];
            se_shader_use(shader);
            se_shader_set_uniform_i32(shader, "bloom_texture", 1);
        } break;
    }

    se_shader_use(shader);
    se_shader_set_uniform_i32(shader, "texture_id", 0);

    for (u32 i = 0; i < previous_render_pass->colour_buffers_count; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, previous_render_pass->colour_buffers[i]);
    }

    glBindVertexArray(renderer->screen_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void se_render_post_process_gaussian_blur(SE_Renderer3D *renderer, const SE_Render_Target *previous_render_pass, b8 horizontal) {
    SE_Shader *shader = renderer->user_shaders[renderer->shader_post_process_gaussian_blur];

    se_shader_use(shader);
    se_shader_set_uniform_i32(shader, "texture_id", 0); // the BrightColour channel
    se_shader_set_uniform_i32(shader, "bright_colour_texture", 1); // the BrightColour channel
    se_shader_set_uniform_i32(shader, "horizontal", (i32)horizontal);

    for (u32 i = 0; i < previous_render_pass->colour_buffers_count; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, previous_render_pass->colour_buffers[i]);
    }

    glBindVertexArray(renderer->screen_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void se_render_directional_shadow_map(SE_Renderer3D *renderer, u32 *mesh_indices, Mat4 *transforms, u32 count, AABB3D world_aabb) {
    SE_Light *light = &renderer->light_directional;
        // -- shadow mapping
    /* calculate the matrices */
    f32 world_aabb_width  = se_math_abs(world_aabb.min.x) + se_math_abs(world_aabb.max.x);
    f32 world_aabb_height = se_math_abs(world_aabb.min.z) + se_math_abs(world_aabb.max.z);
    f32 world_aabb_size = se_math_max(world_aabb_width, world_aabb_height);
    f32 left   = -world_aabb_size;
    f32 right  = +world_aabb_size;
    f32 bottom = -world_aabb_size;
    f32 top    = +world_aabb_size;
    f32 near   = -world_aabb_size;
    f32 far    = +world_aabb_size;

    light->calculated_position = v3f(
        -light->direction.x,
        -light->direction.y,
        -light->direction.z
        // 0
    );

    Mat4 light_proj = mat4_ortho(left, right, bottom, top, near, far);
    Vec3 light_target = vec3_add(renderer->light_directional.direction, light->calculated_position);
    Mat4 light_view = mat4_lookat(light->calculated_position, light_target, vec3_up());
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
    }

    glDisable(GL_CULL_FACE); // @TODO what the f investigate
    { /* render the scene from the light's point of view */
        glCullFace(GL_FRONT);
        /* configure shadow shader */
        serender_target_use(&renderer->shadow_render_target);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        for (u32 i = 0; i < count; ++i) {
            u32 mesh_index = mesh_indices[i];
            if (mesh_index >= renderer->user_meshes_count) continue; // this mesh does not exist
            Mat4 model_mat = transforms[i];

            recursive_render_directional_shadow_map_for_mesh(renderer, mesh_index, model_mat, light_space_mat);
        }
        glBindVertexArray(0);

        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE); // @remove after fixing whatever this is
    }

    serender_target_use(NULL);

    renderer->light_space_matrix = light_space_mat;
}

void se_render_omnidirectional_shadow_map(SE_Renderer3D *renderer, u32 *mesh_indices, Mat4 *transforms, u32 count) {
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        SE_Light_Point *point_light = &renderer->point_lights[i];
        glViewport(0, 0, renderer->omnidirectional_shadow_map_size, renderer->omnidirectional_shadow_map_size);
        glBindFramebuffer(GL_FRAMEBUFFER, point_light->depth_map_fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            Mat4 shadow_transforms[6];
            f32 far  = 25.0f;
            { // configure shader and matrices
                // projection
                f32 aspect = renderer->omnidirectional_shadow_map_size / (f32)renderer->omnidirectional_shadow_map_size;
                f32 near = 1.0f;
                Mat4 shadow_proj = mat4_perspective(SEMATH_DEG2RAD_MULTIPLIER * 90.0f, aspect, near, far);
                // views for each face
                shadow_transforms[0] = mat4_mul(
                    mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(1, 0, 0)), vec3_down()),
                    shadow_proj);
                shadow_transforms[1] = mat4_mul(
                    mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(-1, 0, 0)), vec3_down()),
                    shadow_proj);
                shadow_transforms[2] = mat4_mul(
                    mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, 1, 0)), vec3_forward()), // !if it doesn't work it's because I swapped froward and backward in semath.c. change these to vec3_backward()
                    shadow_proj);
                shadow_transforms[3] = mat4_mul(
                    mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, -1, 0)), vec3_forward()), // !if it doesn't work it's because I swapped froward and backward in semath.c. change these to vec3_backward()
                    shadow_proj);
                shadow_transforms[4] = mat4_mul(
                    mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, 0, 1)), vec3_down()),
                    shadow_proj);
                shadow_transforms[5] = mat4_mul(
                    mat4_lookat(point_light->position, vec3_add(point_light->position, v3f(0, 0, -1)), vec3_down()),
                    shadow_proj);
            }
            // render scene
            for (u32 i = 0; i < count; ++i) {
                u32 mesh_index = mesh_indices[i];
                if (mesh_index >= renderer->user_meshes_count) continue; // this mesh does not exist
                Mat4 model_mat = transforms[i];
                recursive_render_omnidir_shadow_map_for_mesh(renderer, mesh_index, model_mat, point_light, shadow_transforms, far);
            }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void se_render3d_reset_render_config() {
    /* default */
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glLineWidth(1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend mode
}

void se_render3d_init(SE_Renderer3D *renderer, SE_Camera3D *current_camera) {
    memset(renderer, 0, sizeof(SE_Renderer3D)); // default everything to zero
    renderer->current_camera = current_camera;
    renderer->light_directional.intensity = 0.5f;
    renderer->gamma = 2.2f;

        //- SHADERS
        // lit
    const char *lit_vertex_files[2] = {shader_filename_lit_header_vsd, shader_filename_lit_vsd};
    const char *lit_fragment_files[2] = {shader_filename_lit_header_fsd, shader_filename_lit_fsd};
    const char *skinned_vertex_files[1] = {shader_filename_lit_skinned_vsd};

        // shadow calc
    const char *shadow_calc_directional_vsd_files[1] = {
        shader_filename_shadow_calc_directional_vsd
    };
    const char *shadow_calc_directional_fsd_files[1] = {
        shader_filename_shadow_calc_directional_fsd
    };
    const char *shadow_calc_directional_skinned_vsd_files[1] = {
        shader_filename_shadow_calc_directional_skinned_mesh_vsd
    };
    const char *shadow_calc_omnidir_vsd_files[1] = {
        shader_filename_shadow_calc_omnidir_vsd
    };
    const char *shadow_calc_omnidir_fsd_files[1] = {
        shader_filename_shadow_calc_omnidir_fsd
    };
    const char *shadow_calc_omnidir_gsd_files[1] = {
        shader_filename_shadow_calc_omnidir_gsd
    };
    const char *shadow_calc_omnidir_skinned_vsd_files[1] = {
        shader_filename_shadow_calc_omnidir_skinned_mesh_vsd
    };

        // lines
    const char *lines_vsd_files[1] = {
        shader_filename_lines_vsd
    };
    const char *lines_fsd_files[1] = {
        shader_filename_lines_fsd
    };

        // outline
    const char *outline_vsd_files[1] = {
        shader_filename_outline_vsd
    };
    const char *outline_fsd_files[1] = {
        shader_filename_outline_fsd
    };

        // sprite
    const char *sprite_vsd_files[1] = {
        shader_filename_sprite_vsd
    };
    const char *sprite_fsd_files[1] = {
        shader_filename_sprite_fsd
    };

        // skeleton
    const char *skeleton_vsd_files[1] = {
        shader_filename_skeleton_vsd
    };

        // post process
    const char *post_process_vsd[1] = {
        shader_filename_post_process_header_vsd
    };

    const char *post_process_tonemap[2] = {
        shader_filename_post_process_header_fsd,
        shader_filename_post_process_tonemap
    };

    const char *post_process_blur[2] = {
        shader_filename_post_process_header_fsd,
        shader_filename_post_process_blur
    };

    const char *post_process_gaussian[2] = {
        shader_filename_post_process_header_fsd,
        shader_filename_post_process_gaussian
    };

    const char *post_process_downsample[2] = {
        shader_filename_post_process_header_fsd,
        shader_filename_post_process_downsample
    };

    const char *post_process_upsample[2] = {
        shader_filename_post_process_header_fsd,
        shader_filename_post_process_upsample
    };

    const char *post_process_bloom[2] = {
        shader_filename_post_process_header_fsd,
        shader_filename_post_process_bloom
    };

    renderer->shader_lit = se_render3d_add_shader(renderer,
        lit_vertex_files, 2,
        lit_fragment_files, 2,
        NULL, 0);

    renderer->shader_shadow_calc = se_render3d_add_shader(renderer,
        shadow_calc_directional_vsd_files, 1,
        shadow_calc_directional_fsd_files, 1,
        NULL, 0);

    renderer->shader_shadow_calc_skinned_mesh = se_render3d_add_shader(renderer,
        shadow_calc_directional_skinned_vsd_files, 1,
        shadow_calc_directional_fsd_files, 1,
        NULL, 0);

    renderer->shader_shadow_omnidir_calc = se_render3d_add_shader(renderer,
        shadow_calc_omnidir_vsd_files, 1,
        shadow_calc_omnidir_fsd_files, 1,
        shadow_calc_omnidir_gsd_files, 1);

    renderer->shader_shadow_omnidir_calc_skinned_mesh = se_render3d_add_shader(renderer,
        shadow_calc_omnidir_skinned_vsd_files, 1,
        shadow_calc_omnidir_fsd_files, 1,
        shadow_calc_omnidir_gsd_files, 1);

    renderer->shader_lines = se_render3d_add_shader(renderer,
        lines_vsd_files, 1,
        lines_fsd_files, 1,
        NULL, 0);

    renderer->shader_outline = se_render3d_add_shader(renderer,
        outline_vsd_files, 1,
        outline_fsd_files, 1,
        NULL, 0);

    renderer->shader_sprite = se_render3d_add_shader(renderer,
        sprite_vsd_files, 1,
        sprite_fsd_files, 1,
        NULL, 0);

    renderer->shader_skinned_mesh = se_render3d_add_shader(renderer,
        skinned_vertex_files, 1,
        lit_fragment_files, 2,
        NULL, 0);

    renderer->shader_skinned_mesh_skeleton = se_render3d_add_shader(renderer,
        skeleton_vsd_files, 1,
        lines_fsd_files, 1,
        NULL, 0);

    renderer->shader_post_process_tonemap = se_render3d_add_shader(renderer,
        post_process_vsd, 1,
        post_process_tonemap, 2,
        NULL, 0);

    renderer->shader_post_process_blur = se_render3d_add_shader(renderer,
        post_process_vsd, 1,
        post_process_blur, 2,
        NULL, 0);

    renderer->shader_post_process_downsample = se_render3d_add_shader(renderer,
        post_process_vsd, 1,
        post_process_downsample, 2,
        NULL, 0);

    renderer->shader_post_process_upsample = se_render3d_add_shader(renderer,
        post_process_vsd, 1,
        post_process_upsample, 2,
        NULL, 0);

    renderer->shader_post_process_bloom = se_render3d_add_shader(renderer,
        post_process_vsd, 1,
        post_process_bloom, 2,
        NULL, 0);

    renderer->shader_post_process_gaussian_blur = se_render3d_add_shader(renderer,
        post_process_vsd, 1,
        post_process_gaussian, 2,
        NULL, 0);

        //- MATERIALS
    //! We must have a default material at index zero.
    //! Because by default meshes point to the zero'th material.
    //! So we create one default material here and we refer to it.
    u32 default_material_index = se_render3d_add_material(renderer);
    se_assert(default_material_index == SE_DEFAULT_MATERIAL_INDEX && "The default material index that was created in the init() of renderer3D did not match what we expected");
    renderer->user_materials[default_material_index]->base_diffuse = (Vec4) {1, 1, 1, 1};
    se_texture_load(&renderer->user_materials[default_material_index]->texture_diffuse,
                    default_diffuse_filepath, SE_TEXTURE_LOAD_CONFIG_CONVERT_TO_LINEAR_SPACE);
    se_texture_load(&renderer->user_materials[default_material_index]->texture_normal,
                    default_normal_filepath, SE_TEXTURE_LOAD_CONFIG_NULL);
    se_texture_load(&renderer->user_materials[default_material_index]->texture_specular,
                    default_specular_filepath, SE_TEXTURE_LOAD_CONFIG_NULL);
    renderer->user_materials[default_material_index]->shader_index = renderer->shader_lit;
    renderer->user_materials[default_material_index]->type = SE_MATERIAL_TYPE_LIT;

        //- SHADOW MAPPING
    renderer->directional_shadow_map_size = 2048;
    serender_target_init(&renderer->shadow_render_target,
        v2f(renderer->directional_shadow_map_size, renderer->directional_shadow_map_size), 1, true);
    renderer->omnidirectional_shadow_map_size = 1024;

    {   // - POINT LIGHT SHADOW MAPPING
        for (u32 L = 0; L < SERENDERER3D_MAX_POINT_LIGHTS; ++L) {
            SE_Light_Point *point_light = &renderer->point_lights[L];
            glGenTextures(1, &point_light->depth_cube_map);
            glBindTexture(GL_TEXTURE_CUBE_MAP, point_light->depth_cube_map);
            for (u32 i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                    renderer->omnidirectional_shadow_map_size, renderer->omnidirectional_shadow_map_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            // ! note: Normally we'd attach a single face of a cubemap texture to the framebuffer object and render the scene 6 times,
            // ! each time swiching the depth buffer target of the framebuffer to a different cubemap face. Since we're going to
            // ! use a geometry shader, that allows us to render to all faces in a single pass, we can directly attach the cubemap
            // ! as a framebuffer's depth attachment with glFramebufferTexture (- from learnopengl.com)
            glGenFramebuffers(1, &point_light->depth_map_fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, point_light->depth_map_fbo);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, point_light->depth_cube_map, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
    }

    {   //- Screen Quad
            // generate buffer
        glGenBuffers(1, &renderer->screen_quad_vbo);
        glGenVertexArrays(1, &renderer->screen_quad_vao);

        glBindVertexArray(renderer->screen_quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->screen_quad_vbo);

                // fill it with a quad
            // f32 vertices[] = {
            //     +1, -1, // 0
            //     +1, +1, // 1
            //     -1, +1, // 2

            //     -1, +1, // 2
            //     -1, -1, // 3
            //     +1, -1  // 0
            // };
            f32 vertices[] = {
                +1,  0, // 0
                +1, +1, // 1
                 0, +1, // 2

                 0, +1, // 2
                 0,  0, // 3
                +1,  0  // 0
                };
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                // enable attributes
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void se_render3d_deinit(SE_Renderer3D *renderer) {
        //- User meshes
    for (u32 i = 0; i < renderer->user_meshes_count; ++i) {
        se_mesh_deinit(renderer->user_meshes[i]);
        free(renderer->user_meshes[i]);
    }
    renderer->user_meshes_count = 0;

        //- User skeletons
    for (u32 i = 0; i < renderer->user_skeletons_count; ++i) {
        se_skeleton_deinit(renderer->user_skeletons[i]);
        free(renderer->user_skeletons[i]);
    }
    renderer->user_skeletons_count = 0;

        //- User shaders
    for (u32 i = 0; i < renderer->user_shaders_count; ++i) {
        se_shader_deinit(renderer->user_shaders[i]);
        free(renderer->user_shaders[i]);
    }
    renderer->user_shaders_count = 0;

        //- Screen Quad (Post Process Quad)
    glDeleteBuffers(1, &renderer->screen_quad_vbo);
    glDeleteVertexArrays(1, &renderer->screen_quad_vao);

        //- User materials
    for (u32 i = 0; i < renderer->user_materials_count; ++i) {
        se_material_deinit(renderer->user_materials[i]);
    }
    renderer->user_materials_count = 0;

        //- Shadow mapping
    serender_target_deinit(&renderer->shadow_render_target);
    for (u32 L = 0; L < SERENDERER3D_MAX_POINT_LIGHTS; ++L) {
        glDeleteTextures(1, &renderer->point_lights[L].depth_cube_map);
        glDeleteFramebuffers(1, &renderer->point_lights[L].depth_map_fbo);
    }
}

u32 se_render3d_add_point_light(SE_Renderer3D *renderer) {
    return se_render3d_add_point_light_ext(renderer, 1.0f, 0.22f, 0.20f);
}

u32 se_render3d_add_point_light_ext(SE_Renderer3D *renderer, f32 constant, f32 linear, f32 quadratic) {
    se_assert(renderer->point_lights_count < SERENDERER3D_MAX_POINT_LIGHTS);

    u32 result = renderer->point_lights_count;
    renderer->point_lights_count++;

        //- Default values
        // NOTE(Matin): We don't need to generate the cubemap here because all cubemaps
        // have already been generated for the maximum number of point lights
    renderer->point_lights[result].position  = v3f(0, 0, 0);
    // renderer->point_lights[result].ambient   = (RGB) {10, 10, 10};
    renderer->point_lights[result].ambient   = (RGB) {255, 255, 255};
    renderer->point_lights[result].diffuse   = (RGB) {255, 255, 255};
    renderer->point_lights[result].specular  = (RGB) {0, 0, 0};
    renderer->point_lights[result].constant  = constant;
    renderer->point_lights[result].linear    = linear;
    renderer->point_lights[result].quadratic = quadratic;

    return result;
}

u32 se_render3d_add_material(SE_Renderer3D *renderer) {
    se_assert(renderer->user_materials_count < SERENDERER3D_MAX_MATERIALS);
    renderer->user_materials[renderer->user_materials_count] = NEW(SE_Material);
    memset(renderer->user_materials[renderer->user_materials_count], 0, sizeof(SE_Material));
    u32 material_index = renderer->user_materials_count;
    renderer->user_materials_count++;
    return material_index;
}

u32 se_render3d_add_skeleton(SE_Renderer3D *renderer) {
    se_assert(renderer->user_skeletons_count < SERENDERER3D_MAX_SKELETONS);
    u32 result = renderer->user_skeletons_count;
    renderer->user_skeletons_count++;
    renderer->user_skeletons[result] = NEW (SE_Skeleton);
        // it's important for bone_nodes.children_count be zero and so should the other things
    memset(renderer->user_skeletons[result], 0, sizeof(SE_Skeleton));
    return result;
}

u32 se_render3d_add_cube(SE_Renderer3D *renderer) {
    u32 result = renderer->user_meshes_count;

    renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
    memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
    se_mesh_generate_cube(renderer->user_meshes[renderer->user_meshes_count], vec3_one());

    renderer->user_meshes_count++;
    return result;
}

u32 se_render3d_add_plane(SE_Renderer3D *renderer, Vec3 scale) {
    u32 result = renderer->user_meshes_count;
    renderer->user_meshes_count++;

    renderer->user_meshes[result] = NEW(SE_Mesh);
    memset(renderer->user_meshes[result], 0, sizeof(SE_Mesh));
    semesh_generate_plane(renderer->user_meshes[result], scale);

    return result;
}

u32 se_render3d_add_sprite_mesh(SE_Renderer3D *renderer, Vec2 scale) {
    u32 result = renderer->user_meshes_count;

    renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
    memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
    se_mesh_generate_sprite(renderer->user_meshes[renderer->user_meshes_count], scale);

    renderer->user_meshes_count++;
    return result;
}

u32 se_render3d_add_line(SE_Renderer3D *renderer, Vec3 pos1, Vec3 pos2, f32 width, RGBA colour) {
    u32 result = renderer->user_meshes_count;

    renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
    memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
    se_mesh_generate_line(renderer->user_meshes[renderer->user_meshes_count], pos1, pos2, width, colour);

    renderer->user_meshes_count++;
    return result;
}

u32 se_render3d_add_mesh_empty(SE_Renderer3D *renderer) {
    u32 result = renderer->user_meshes_count;
    renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
    memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
    sedefault_mesh(renderer->user_meshes[renderer->user_meshes_count]);
    renderer->user_meshes_count++;
    return result;
}

u32 se_render3d_add_gizmos_coordniates(SE_Renderer3D *renderer) {
    u32 result = renderer->user_meshes_count;
    f32 width = 3;

    renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
    memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
    se_mesh_generate_gizmos_coordinates(renderer->user_meshes[renderer->user_meshes_count], width);

    renderer->user_meshes_count++;
    return result;
}

u32 se_render3d_add_gizmos_aabb
(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width) {
    u32 result = renderer->user_meshes_count;

    renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
    memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
    se_mesh_generate_gizmos_aabb(renderer->user_meshes[renderer->user_meshes_count], min, max, line_width);

    renderer->user_meshes_count++;
    return result;
}

void se_render3d_update_gizmos_aabb
(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width, u32 mesh_index) {
    memset(renderer->user_meshes[mesh_index], 0, sizeof(SE_Mesh));
    se_mesh_generate_gizmos_aabb(renderer->user_meshes[mesh_index], min, max, line_width);
}

u32 se_render3d_add_shader(SE_Renderer3D *renderer,
                            const char **vsd_files,
                            u32 vsd_count,
                            const char **fsd_files,
                            u32 fsd_count,
                            const char **gsd_files,
                            u32 gsd_count) {
    // add a shader to the renderer and initialise it
    se_assert(renderer->user_shaders_count < SERENDERER3D_MAX_SHADERS);
    u32 result = renderer->user_shaders_count;
    renderer->user_shaders_count++;

    renderer->user_shaders[result] = NEW(SE_Shader);
    memset(renderer->user_shaders[result], 0, sizeof(SE_Shader));

    se_shader_init_from_files(renderer->user_shaders[result],
                              vsd_files,
                              vsd_count,
                              fsd_files,
                              fsd_count,
                              gsd_files,
                              gsd_count);
    return result;
}