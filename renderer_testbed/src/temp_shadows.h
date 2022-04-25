/* this file should be removed. For now it should only be included in application.c because we're trying out shadows and don't want to properly abstarct it yet */

#include "serenderer_opengl.h"
#include "application.h"

/* shadow mapping */
#define shadow_w  1024
#define shadow_h  1024
u32 shadow_depth_map_fbo;
u32 shadow_depth_map;
SE_Shader shadow_shader;
// SE_Shader shadow_depth_map_shader; // to debug depth map

// @remove
u32 shadow_vbo;

/* render the entities of a scene using the given shader */
void shadow_render_scene(SE_Shader *shader, Application *app) { // @temp
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

void shadow_render(Application *app, SE_Renderer3D *renderer) {
    glViewport(0, 0, shadow_w, shadow_h);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_map_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        // configure shader and matrices
        seshader_use(&shadow_shader);
        f32 near_plane = 1.0f, far_plane = 70.5f;
        Mat4 light_projection = mat4_ortho(-200.0f, 200.0f, -200.0f, 200.0f, near_plane, far_plane); // what is visible to the light
        // Mat4 light_view = mat4_lookat(vec3_zero(), renderer->light_directional.direction, vec3_up());
        Vec3 light_pos = vec3_create(-2, 4, -1);
        Vec3 light_target = vec3_add(renderer->light_directional.direction, light_pos);
        Mat4 light_view = mat4_lookat(light_pos, light_target, vec3_up());

        Mat4 light_space_mat = mat4_mul(light_projection, light_view);
        // Mat4 light_space_mat = mat4_mul(renderer->current_camera->projection, renderer->current_camera->view);

        seshader_set_uniform_mat4(&shadow_shader, "light_space_matrix", light_space_mat);
        shadow_render_scene(&shadow_shader, app);

        // render scene
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}