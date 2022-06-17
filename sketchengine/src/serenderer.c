#include "serenderer.h"

#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "sestring.h"

#include "seinput.h" // for camera
#include "stdio.h" // @temp for debugging

//// UTILITY DEBUG PROCEDURES

static void debug_print_skeleton(const SE_Skeleton *skeleton, const SE_Bone_Node *parent) {
    static i32 call = 0;
    printf("%i: parent id: %i | ", call, parent->bones_info_index);
    call++;
    for (u32 i = 0; i < parent->children_count; ++i) {
        printf("%i | ", parent->children[i]);
    }
    printf("\n");

    for (u32 i = 0; i < parent->children_count; ++i) {
        debug_print_skeleton(skeleton, &skeleton->bone_nodes[parent->children[i]]);
    }
}

void debug_print_animation(const SE_Skeletal_Animation *anim, const SE_Bone_Animations *parent) {
    static i32 call = 0;
    printf("%i: parent name: %s | ", call, parent->name.buffer);
    call++;
}

///
/// Materials
///

void sematerial_deinit(SE_Material *material) {
    if (material->texture_diffuse.loaded)  setexture_unload(&material->texture_diffuse);
    if (material->texture_specular.loaded) setexture_unload(&material->texture_specular);
    if (material->texture_normal.loaded)   setexture_unload(&material->texture_normal);
}

///
/// Camera
///

void secamera3d_init(SE_Camera3D *cam) {
    cam->position = vec3_zero();
    cam->yaw = 0;
    cam->pitch = 0;
    cam->up = vec3_up();
}

Vec3 secamera3d_get_front(const SE_Camera3D *cam) {
    f32 yaw = cam->yaw;
    f32 pitch = cam->pitch;
    Vec3 camera_front = {0};
    camera_front.x = semath_cos(yaw * SEMATH_DEG2RAD_MULTIPLIER) * semath_cos(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    camera_front.y = semath_sin(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    camera_front.z = semath_sin(yaw * SEMATH_DEG2RAD_MULTIPLIER) * semath_cos(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    vec3_normalise(&camera_front);
    return camera_front;
}

Mat4 secamera3d_get_view(const SE_Camera3D *cam) {
    Vec3 camera_front = secamera3d_get_front(cam);
    return mat4_lookat(cam->position, vec3_add(cam->position, camera_front), cam->up);
}

void secamera3d_update_projection(SE_Camera3D *cam, i32 window_w, i32 window_h) {
    cam->view = secamera3d_get_view(cam);
    // f32 near_plane = 0.01f, far_plane = 70.5f;
    // cam->projection = mat4_ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    cam->projection = mat4_perspective(SEMATH_PI * 0.25f, window_w / (f32) window_h, 0.1f, 1000.0f);
}

void secamera3d_input(SE_Camera3D *camera, SE_Input *seinput) {
    { // movement
        i32 r       = seinput_is_key_down(seinput, SDL_SCANCODE_D) == true ? 1 : 0;
        i32 l       = seinput_is_key_down(seinput, SDL_SCANCODE_A) == true ? 1 : 0;
        i32 d       = seinput_is_key_down(seinput, SDL_SCANCODE_S) == true ? 1 : 0;
        i32 u       = seinput_is_key_down(seinput, SDL_SCANCODE_W) == true ? 1 : 0;
        i32 elevate = seinput_is_key_down(seinput, SDL_SCANCODE_E) == true ? 1 : 0;
        i32 dive    = seinput_is_key_down(seinput, SDL_SCANCODE_Q) == true ? 1 : 0;

        Vec3 input = vec3_create(r - l, d - u, elevate - dive);

        f32 camera_speed = 0.2f; // adjust accordingly

        Vec3 movement = {
            -input.x * camera_speed,
            -input.y * camera_speed,
            input.z * camera_speed,
        };

        Vec3 camera_front = secamera3d_get_front(camera);
        Vec3 camera_right = vec3_normalised(vec3_cross(camera->up, camera_front));
        Vec3 camera_up = vec3_normalised(vec3_cross(camera_front, camera_right));

        if (movement.x != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_right, movement.x));
        }
        if (movement.y != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_front, movement.y));
        }
        if (movement.z != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_up, movement.z));
        }
    }
    { // -- rotate camera
        u32 mouse_state = SDL_GetMouseState(NULL, NULL);
        if (mouse_state & SDL_BUTTON_RMASK) {
            seui_mouse_fps_activate(seinput);
            f32 sensitivity = 0.15f;
            f32 xoffset = seinput->mouse_screen_pos_delta.x * sensitivity;
            f32 yoffset = seinput->mouse_screen_pos_delta.y * sensitivity;

            if (semath_abs(xoffset) > sensitivity) camera->yaw += xoffset;
            if (semath_abs(yoffset) > sensitivity) camera->pitch += yoffset;
            if(camera->pitch > +89.0f) camera->pitch = +89.0f;
            if(camera->pitch < -89.0f) camera->pitch = -89.0f;
        } else {
            seui_mouse_fps_deactivate(seinput);
        }
    }
}

///
/// MESH
///

static void sedefault_mesh(SE_Mesh *mesh) {
    mesh->next_mesh_index = -1;
    mesh->type = SE_MESH_TYPE_NORMAL;
    mesh->skeleton = NULL;
}

void semesh_deinit(SE_Mesh *mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ibo);
    mesh->material_index = 0;
    if (mesh->type == SE_MESH_TYPE_SKINNED) {
        free(mesh->skeleton);
    }
}

void semesh_generate_quad(SE_Mesh *mesh, Vec2 scale) { // 2d plane
    sedefault_mesh(mesh);
    SE_Vertex3D verts[4];

    scale = vec2_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec3) {-scale.x, -scale.y, 0.0f}; // no z
    verts[1].position = (Vec3) {-scale.x, +scale.y, 0.0f}; // no z
    verts[2].position = (Vec3) {+scale.x, +scale.y, 0.0f}; // no z
    verts[3].position = (Vec3) {+scale.x, -scale.y, 0.0f}; // no z

    verts[0].normal = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[1].normal = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[2].normal = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[3].normal = (Vec3) {0.0f, 1.0f, 0.0f};

    verts[0].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[1].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[2].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[3].tangent = (Vec3) {1.0f, 0.0f, 0.0f};

    verts[0].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[1].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[2].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[3].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};

    verts[0].texture_coord = (Vec2) {0, 0};
    verts[1].texture_coord = (Vec2) {0, 1};
    verts[2].texture_coord = (Vec2) {1, 1};
    verts[3].texture_coord = (Vec2) {1, 0};

    u32 indices[6] = {
        0, 1, 2,
        2, 3, 0
    };

    semesh_generate(mesh, 4, verts, 6, indices);
}

void semesh_generate_sprite(SE_Mesh *mesh, Vec2 scale) {
    sedefault_mesh(mesh);
    mesh->type = SE_MESH_TYPE_SPRITE;
    SE_Vertex3D verts[4];

    scale = vec2_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec3) {-scale.x, -scale.y, 0.0f}; // no z
    verts[1].position = (Vec3) {+scale.x, -scale.y, 0.0f}; // no z
    verts[2].position = (Vec3) {+scale.x, +scale.y, 0.0f}; // no z
    verts[3].position = (Vec3) {-scale.x, +scale.y, 0.0f}; // no z

    verts[0].normal = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[1].normal = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[2].normal = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[3].normal = (Vec3) {0.0f, 0.0f, 1.0f};

    verts[0].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[1].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[2].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[3].tangent = (Vec3) {1.0f, 0.0f, 0.0f};

    verts[0].bitangent = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[1].bitangent = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[2].bitangent = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[3].bitangent = (Vec3) {0.0f, 1.0f, 0.0f};

    verts[0].texture_coord = (Vec2) {0, 1};
    verts[1].texture_coord = (Vec2) {1, 1};
    verts[2].texture_coord = (Vec2) {1, 0};
    verts[3].texture_coord = (Vec2) {0, 0};

    u32 indices[6] = {
        0, 1, 2,
        2, 3, 0
    };

    semesh_generate(mesh, 4, verts, 6, indices);
}

void semesh_generate_cube(SE_Mesh *mesh, Vec3 scale) {
    sedefault_mesh(mesh);
    SE_Vertex3D verts[8] = {0};

    scale = vec3_mul_scalar(scale, 0.5f);

    verts[0].position = (Vec3) {-scale.x, -scale.y, +scale.z}; // 0
    verts[1].position = (Vec3) {-scale.x, +scale.y, +scale.z}; // 1
    verts[2].position = (Vec3) {+scale.x, +scale.y, +scale.z}; // 2
    verts[3].position = (Vec3) {+scale.x, -scale.y, +scale.z}; // 3
    verts[4].position = (Vec3) {+scale.x, -scale.y, -scale.z}; // 4
    verts[5].position = (Vec3) {+scale.x, +scale.y, -scale.z}; // 5
    verts[6].position = (Vec3) {-scale.x, +scale.y, -scale.z}; // 6
    verts[7].position = (Vec3) {-scale.x, -scale.y, -scale.z}; // 7

    // @TODO generate better normals
    verts[0].normal = (Vec3) {0, 0, 1};
    verts[1].normal = (Vec3) {0, 0, 1};
    verts[2].normal = (Vec3) {0, 0, 1};
    verts[3].normal = (Vec3) {0, 0, 1};
    verts[4].normal = (Vec3) {1, 0, 0};
    verts[5].normal = (Vec3) {1, 0, 0};
    verts[6].normal = (Vec3) {0, 1, 0};
    verts[7].normal = (Vec3) {-1, 0, 0};

    u32 indices[12 * 3] = {
        0, 1, 2,
        2, 3, 0,
        3, 4, 0,
        0, 4, 5,
        5, 7, 6,
        7, 4, 5,
        2, 7, 1,
        1, 7, 6,
        1, 6, 0,
        0, 6, 5,
        2, 7, 3,
        3, 7, 4
    };
    semesh_generate(mesh, 8, verts, 12 * 3, indices);
}

void semesh_generate_plane(SE_Mesh *mesh, Vec3 scale) {
    sedefault_mesh(mesh);
    SE_Vertex3D verts[4] = {0};

    scale = vec3_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec3) {-scale.x, 0.0f, -scale.z};
    verts[1].position = (Vec3) {-scale.x, 0.0f, +scale.z};
    verts[2].position = (Vec3) {+scale.x, 0.0f, +scale.z};
    verts[3].position = (Vec3) {+scale.x, 0.0f, -scale.z};

    verts[0].normal = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[1].normal = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[2].normal = (Vec3) {0.0f, 1.0f, 0.0f};
    verts[3].normal = (Vec3) {0.0f, 1.0f, 0.0f};

    verts[0].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[1].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[2].tangent = (Vec3) {1.0f, 0.0f, 0.0f};
    verts[3].tangent = (Vec3) {1.0f, 0.0f, 0.0f};

    verts[0].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[1].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[2].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};
    verts[3].bitangent = (Vec3) {0.0f, 0.0f, 1.0f};

    verts[0].texture_coord = (Vec2) {0, 0};
    verts[1].texture_coord = (Vec2) {0, 1};
    verts[2].texture_coord = (Vec2) {1, 1};
    verts[3].texture_coord = (Vec2) {1, 0};

    u32 indices[6] = {
        0, 1, 2,
        2, 3, 0
    };

    semesh_generate(mesh, 4, verts, 6, indices);
}

void semesh_generate_line(SE_Mesh *mesh, Vec3 pos1, Vec3 pos2, f32 width) {
    sedefault_mesh(mesh);
    mesh->type = SE_MESH_TYPE_LINE;
    mesh->line_width = width;

    SE_Vertex3D verts[2] = {
        {.position = pos1},
        {.position = pos2}
    };

    u32 indices[2] = {
        0, 1
    };
    semesh_generate(mesh, 2, verts, 2, indices);
}

void semesh_generate_line_fan(SE_Mesh *mesh, Vec3 origin, Vec3 *positions, u32 positions_count, f32 line_width) {
    sedefault_mesh(mesh);
    mesh->type = SE_MESH_TYPE_LINE;
    mesh->line_width = line_width;

    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * (positions_count + 1));
    verts[0].position = origin;
    for (u32 i = 0; i < positions_count; ++i) {
        verts[i + 1].position = positions[i];
    }

    u32 *indices = malloc(sizeof(u32) * (positions_count * 2));

    u32 index = 1;
    for (u32 i = 0; i < positions_count * 2; i += 2) {
        indices[i] = 0;
        indices[i + 1] = index;
        ++index;
    }
    semesh_generate(mesh, positions_count + 1, verts, positions_count * 2, indices);

    free(verts);
    free(indices);
}

void semesh_generate_gizmos_aabb(SE_Mesh *mesh, Vec3 min, Vec3 max, f32 line_width) {
    sedefault_mesh(mesh);
    mesh->type = SE_MESH_TYPE_LINE;
    mesh->line_width = line_width;

    SE_Vertex3D verts[8] = {
        {.position = (Vec3) {min.x, min.y, min.z}}, // front bottom left - 0
        {.position = (Vec3) {max.x, min.y, min.z}}, // front bottom right - 1
        {.position = (Vec3) {max.x, max.y, min.z}}, // front top right - 2
        {.position = (Vec3) {min.x, max.y, min.z}}, // front top left - 3

        {.position = (Vec3) {min.x, min.y, max.z}}, // behind bottom left - 4
        {.position = (Vec3) {max.x, min.y, max.z}}, // behind bottom right - 5
        {.position = (Vec3) {max.x, max.y, max.z}}, // behind top right - 6
        {.position = (Vec3) {min.x, max.y, max.z}}, // behind top left - 7
    };

    u32 indices[24] = {
        0, 1,
        0, 4,
        0, 3,
        5, 1,
        5, 6,
        5, 4,
        2, 1,
        2, 3,
        2, 6,
        7, 3,
        7, 6,
        7, 4
    };
    semesh_generate(mesh, 8, verts, 24, indices);
}

void semesh_generate_gizmos_coordinates(SE_Mesh *mesh, f32 scale, f32 width) {
    sedefault_mesh(mesh);
    mesh->type = SE_MESH_TYPE_LINE;
    mesh->line_width = width;

    Vec3 pos_o = vec3_zero();
    Vec3 pos_x = vec3_right();
    Vec3 pos_y = vec3_up();
    Vec3 pos_z = vec3_forward();

    SE_Vertex3D verts[6] = {
        {.position = pos_o}, // x
        {.position = pos_x},
        {.position = pos_o}, // y
        {.position = pos_y},
        {.position = pos_o}, // z
        {.position = pos_z}
    };

    u32 indices[6] = {
        0, 1, // x
        2, 3, // y
        4, 5, // z
    };
    semesh_generate(mesh, 6, verts, 6, indices);
}

static void semesh_generate_skinned // same as semesh_generate but for skinned vertices
(SE_Mesh *mesh, u32 vert_count, const SE_Skinned_Vertex *vertices, u32 index_count, u32 *indices) {
    // generate buffers
    glGenBuffers(1, &mesh->vbo);
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->ibo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Skinned_Vertex) * vert_count, vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);

    se_assert(mesh->type == SE_MESH_TYPE_SKINNED && "mesh type was something other than skinned but we tried to generate one");

        // enable position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.position));
        // enable normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.normal));
        // enable uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.texture_coord));
        // enable tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.tangent));
        // enable bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.bitangent));
        // enable bone ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, bone_ids));
        // enable bone weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, bone_weights));

    mesh->vert_count = index_count;
    mesh->indexed = true;
    mesh->aabb = (AABB3D) {0}; //semesh_calc_aabb(vertices, vert_count);

    // unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

    /// Returns the final transformation of a given SE_Bone_Node based on its parents
static Mat4 recursive_calc_bone_transform(const SE_Skeleton *skeleton, const SE_Bone_Node *node) {
    se_assert(node->bones_info_index >= 0 && node->bones_info_index < skeleton->bone_count);
    if (node->parent >= 0) { // if we have a parent go up the tree
        Mat4 parent_transform = recursive_calc_bone_transform(skeleton, &skeleton->bone_nodes[node->parent]);
        return mat4_mul(parent_transform, node->local_transform);
    }
    return node->local_transform;
}

static Mat4 seskeleton_get_bone_final_transform(const SE_Skeleton *skeleton, i32 bone_node_index) {
    se_assert(bone_node_index >= 0 && bone_node_index < skeleton->bone_node_count);
    return recursive_calc_bone_transform(skeleton, &skeleton->bone_nodes[bone_node_index]);
}

static void recursive_generate_skeleton_verts
(const SE_Skeleton *skeleton, SE_Skinned_Vertex *verts, u32 *vert_count, u32 *indices, u32 *index_count, const SE_Bone_Node *node, Mat4 parent_transform) {
    Mat4 final_transform = node->local_transform;
    final_transform = mat4_mul(parent_transform, node->local_transform);

    verts[*vert_count].vert.position = mat4_get_translation(final_transform);
    verts[*vert_count].bone_ids[0] = node->bones_info_index;
    (*vert_count)++;

    se_assert(node->bones_info_index >= 0);
    for (u32 i = 0; i < node->children_count; ++i) {
            // draw a line between the node and the child
            // we can assign a node's id as the index because we're adding
            // one vertex for each node. If this doesn't work consider changing
            // verts[*vert_count] to verts[node->id] to be more explicit
        indices[*index_count] = node->bones_info_index;
        (*index_count)++;
        indices[*index_count] = skeleton->bone_nodes[node->children[i]].bones_info_index;
        (*index_count)++;
        recursive_generate_skeleton_verts(skeleton, verts, vert_count, indices, index_count, &skeleton->bone_nodes[node->children[i]], final_transform);
    }
}

static void recursive_generate_static_skeleton_verts
(const SE_Skeleton *skeleton, SE_Vertex3D *verts, u32 *vert_count, u32 *indices, u32 *index_count, const SE_Bone_Node *node, Mat4 parent_transform) {
    Mat4 final_transform = node->local_transform;
    final_transform = mat4_mul(parent_transform, node->local_transform);

    verts[*vert_count].position = mat4_get_translation(final_transform);
    (*vert_count)++;

    se_assert(node->bones_info_index >= 0);
    for (u32 i = 0; i < node->children_count; ++i) {
            // draw a line between the node and the child
            // we can assign a node's id as the index because we're adding
            // one vertex for each node. If this doesn't work consider changing
            // verts[*vert_count] to verts[node->id] to be more explicit
        indices[*index_count] = node->bones_info_index;
        (*index_count)++;
        indices[*index_count] = skeleton->bone_nodes[node->children[i]].bones_info_index;
        (*index_count)++;
        recursive_generate_static_skeleton_verts(skeleton, verts, vert_count, indices, index_count, &skeleton->bone_nodes[node->children[i]], final_transform);
    }
}

static void recursive_generate_skeleton_verts_as_points
(const SE_Skeleton *skeleton, SE_Skinned_Vertex *verts, u32 *vert_count, u32 *indices, u32 *index_count, const SE_Bone_Node *node, Mat4 parent_transform) {
        // the problem here is that as we come back up the recursion stack our index_count and vert_count go back to their old values
        // instead of retaining their values.

    Mat4 final_transform = node->local_transform;
    final_transform = mat4_mul(parent_transform, node->local_transform);

        // draw a point at the given bone position
    verts[*vert_count].vert.position = mat4_get_translation(final_transform);
    (*vert_count)++;
    indices[*index_count] = node->bones_info_index;
    (*index_count)++;

    se_assert(node->bones_info_index >= 0);
    for (u32 i = 0; i < node->children_count; ++i) {
        recursive_generate_skeleton_verts_as_points(skeleton, verts, vert_count, indices, index_count, &skeleton->bone_nodes[node->children[i]], final_transform);
    }
}

void semesh_generate_static_skeleton
(SE_Mesh *mesh, const SE_Skeleton *skeleton) {
        // generate buffers
    glGenBuffers(1, &mesh->vbo);
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->ibo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

        //- generate vertices for the skeleton
    u32 vert_count = 0;
    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * skeleton->bone_node_count);
    u32 index_count = 0;
    u32 *indices = malloc(sizeof(u32) * skeleton->bone_node_count * 2);

    // recursive_generate_static_skeleton_verts(skeleton, verts, &vert_count, indices, &index_count,
    //                                         &skeleton->bone_nodes[0], mat4_identity());

    // @debug each bone's inverse_neutral_transform
        //- generate the verts based on the model space transform of each bone
    for (u32 i = 0; i < skeleton->bone_node_count; ++i) {
        verts[vert_count].position = mat4_get_translation(mat4_inverse(skeleton->bone_nodes[i].inverse_neutral_transform));
        indices[index_count] = i;
        vert_count++;
        index_count++;
    }

        //- mesh settings
    mesh->vert_count = index_count;
    mesh->indexed = true;
    mesh->aabb = (AABB3D) {0};
    mesh->line_width = 2;
    // mesh->type = SE_MESH_TYPE_LINE; // sense we're going to generate a skeleton we're going to be lines
    mesh->point_radius = 3; // @temp
    mesh->type = SE_MESH_TYPE_POINT; // sense we're going to generate a skeleton we're going to be lines
    mesh->skeleton = NULL; // we don't want to remember the original skeleton. No reason yet.

        //- fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Vertex3D) * vert_count,    verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);

        //- enable position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, position));

        //- mamange memory
    free(verts);
    free(indices);

        //- unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void semesh_generate_skinned_skeleton
(SE_Mesh *mesh, SE_Skeleton *skeleton, bool line, bool with_animation) {
        // generate buffers
    glGenBuffers(1, &mesh->vbo);
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->ibo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

        // generate vertices for the skeleton
    u32 vert_count;
    SE_Skinned_Vertex *verts;
    u32 index_count;
    u32 *indices;
    if (line) {
            //* line rendering
#if 1
        vert_count = 0;
        verts = malloc(sizeof(SE_Skinned_Vertex) * skeleton->bone_node_count);
        index_count = 0;
        indices = malloc(sizeof(u32) * skeleton->bone_node_count * 2);
        recursive_generate_skeleton_verts(skeleton, verts, &vert_count, indices, &index_count, &skeleton->bone_nodes[0], mat4_identity());
        // se_assert(vert_count == skeleton->bone_node_count); @debug
#else // @debug
        vert_count = 0;
        verts = malloc(sizeof(SE_Skinned_Vertex) * skeleton->bone_count);
        index_count = 0;
        indices = malloc(sizeof(u32) * skeleton->bone_count * 2);
        recursive_generate_skeleton_verts(skeleton, verts, &vert_count, indices, &index_count, &skeleton->bone_nodes[0], mat4_identity());
        // se_assert(vert_count == skeleton->bone_count); @debug
#endif
            // mesh settings
        mesh->vert_count = index_count;
        mesh->indexed = true;
        mesh->aabb = (AABB3D) {0};
        mesh->line_width = 2;
        mesh->type = SE_MESH_TYPE_LINE; // sense we're going to generate a skeleton we're going to be lines
    } else {
            //* point rendering
        vert_count = 0;
        verts = malloc(sizeof(SE_Skinned_Vertex) * skeleton->bone_node_count);
        index_count = 0;
        indices = malloc(sizeof(u32) * skeleton->bone_node_count);
        recursive_generate_skeleton_verts_as_points(skeleton, verts, &vert_count, indices, &index_count, &skeleton->bone_nodes[0], mat4_identity());
        se_assert(vert_count == skeleton->bone_node_count);
        se_assert(index_count == skeleton->bone_node_count);

            // mesh settings
        mesh->vert_count = index_count;
        mesh->indexed = true;
        mesh->point_radius = 8;
        mesh->aabb = (AABB3D) {0};
        mesh->type = SE_MESH_TYPE_POINT; // sense we're going to generate a skeleton we're going to be lines
    }

        //* fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Skinned_Vertex) * vert_count,    verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);

        // enable position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.position));

    if (with_animation) {
            // enable bone ids
        glEnableVertexAttribArray(1);
        glVertexAttribIPointer(1, 1, GL_INT, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, bone_ids[0]));
    }

    free(verts);
    free(indices);

        // unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        //* Animation
    mesh->skeleton = skeleton;
}

void semesh_generate(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices, u32 index_count, u32 *indices) {
    // generate buffers
    glGenBuffers(1, &mesh->vbo);
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->ibo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Vertex3D) * vert_count, vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);

    if (mesh->type == SE_MESH_TYPE_NORMAL || mesh->type == SE_MESH_TYPE_LINE) {
            // -- enable position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, position));
            // -- enable normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, normal));
            // -- enable uv
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, texture_coord));
            // -- enable tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, tangent));
            // -- enable bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, bitangent));
    } else
    if (mesh->type == SE_MESH_TYPE_SPRITE) {
            // -- enable position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, position));
            // -- enable uv
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, texture_coord));
    } else {
        se_assert(false && "mesh type was something other than normal, line, or sprite but we tried to generate one");
    }

    mesh->vert_count = index_count;
    mesh->indexed = true;
    mesh->aabb = semesh_calc_aabb(vertices, vert_count);

    // unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//// RENDER 3D ////

AABB3D semesh_calc_aabb(const SE_Vertex3D *verts, u32 verts_count) {
    f32 xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;

    for (u32 i = 0; i < verts_count; ++i) {
        Vec3 vert_pos = verts[i].position;
        if (xmin > vert_pos.x) xmin = vert_pos.x;
        if (ymin > vert_pos.y) ymin = vert_pos.y;
        if (zmin > vert_pos.z) zmin = vert_pos.z;

        if (xmax < vert_pos.x) xmax = vert_pos.x;
        if (ymax < vert_pos.y) ymax = vert_pos.y;
        if (zmax < vert_pos.z) zmax = vert_pos.z;
    }

    AABB3D result = {(Vec3) {xmin, ymin, zmin}, (Vec3) {xmax, ymax, zmax}};
    return result;
}

AABB3D aabb3d_from_points(Vec3 point1, Vec3 point2, Mat4 transform) {
    /* generate a new aabb that's transformed */
    Vec4 points[8] = {0};
    points[0].x = point1.x;
    points[0].y = point1.y;
    points[0].z = point1.z;

    points[1].x = point2.x;
    points[1].y = point1.y;
    points[1].z = point1.z;

    points[2].x = point2.x;
    points[2].y = point2.y;
    points[2].z = point1.z;

    points[3].x = point1.x;
    points[3].y = point2.y;
    points[3].z = point1.z;

    points[4].x = point1.x;
    points[4].y = point1.y;
    points[4].z = point2.z;

    points[5].x = point2.x;
    points[5].y = point1.y;
    points[5].z = point2.z;

    points[6].x = point2.x;
    points[6].y = point2.y;
    points[6].z = point2.z;

    points[7].x = point1.x;
    points[7].y = point2.y;
    points[7].z = point2.z;

    points[0].w = 0.0f;
    points[1].w = 0.0f;
    points[2].w = 0.0f;
    points[3].w = 0.0f;
    points[4].w = 0.0f;
    points[5].w = 0.0f;
    points[6].w = 0.0f;
    points[7].w = 0.0f;

    /* transform the points */
    points[0] = mat4_mul_vec4(transform, points[0]);
    points[1] = mat4_mul_vec4(transform, points[1]);
    points[2] = mat4_mul_vec4(transform, points[2]);
    points[3] = mat4_mul_vec4(transform, points[3]);
    points[4] = mat4_mul_vec4(transform, points[4]);
    points[5] = mat4_mul_vec4(transform, points[5]);
    points[6] = mat4_mul_vec4(transform, points[6]);
    points[7] = mat4_mul_vec4(transform, points[7]);

    /* calculate the new aabb based on transformed points */
    SE_Vertex3D dummy_verts[8] = {0};
    dummy_verts[0].position = (Vec3) {points[0].x, points[0].y, points[0].z};
    dummy_verts[1].position = (Vec3) {points[1].x, points[1].y, points[1].z};
    dummy_verts[2].position = (Vec3) {points[2].x, points[2].y, points[2].z};
    dummy_verts[3].position = (Vec3) {points[3].x, points[3].y, points[3].z};
    dummy_verts[4].position = (Vec3) {points[4].x, points[4].y, points[4].z};
    dummy_verts[5].position = (Vec3) {points[5].x, points[5].y, points[5].z};
    dummy_verts[6].position = (Vec3) {points[6].x, points[6].y, points[6].z};
    dummy_verts[7].position = (Vec3) {points[7].x, points[7].y, points[7].z};
    return semesh_calc_aabb(dummy_verts, 8);
}

AABB3D aabb3d_calc(const AABB3D *aabbs, u32 aabb_count) {
    f32 pos1x = 0, pos2x = 0, pos1y = 0, pos2y = 0, pos1z = 0, pos2z = 0;
    for (u32 i = 0; i < aabb_count; ++i) {
        // convert obj aabb to world space aabb (as in take rotation into account)
        AABB3D aabb = aabbs[i];
        Vec3 min = aabb.min;
        Vec3 max = aabb.max;

        if (pos1x > min.x) pos1x = min.x;
        if (pos1y > min.y) pos1y = min.y;
        if (pos1z > min.z) pos1z = min.z;

        if (pos2x < max.x) pos2x = max.x;
        if (pos2y < max.y) pos2y = max.y;
        if (pos2z < max.z) pos2z = max.z;
    }

    AABB3D result = {(Vec3) {pos1x, pos1y, pos1z}, (Vec3) {pos2x, pos2y, pos2z}};
    return result;
}

static void copy_ai_matrix_to_mat4(struct aiMatrix4x4 aiMat, Mat4 *mat4) {
    mat4->data[0] = aiMat.a1; mat4->data[4] = aiMat.a2; mat4->data[8]  = aiMat.a3; mat4->data[12] = aiMat.a4;
    mat4->data[1] = aiMat.b1; mat4->data[5] = aiMat.b2; mat4->data[9]  = aiMat.b3; mat4->data[13] = aiMat.b4;
    mat4->data[2] = aiMat.c1; mat4->data[6] = aiMat.c2; mat4->data[10] = aiMat.c3; mat4->data[14] = aiMat.c4;
    mat4->data[3] = aiMat.d1; mat4->data[7] = aiMat.d2; mat4->data[11] = aiMat.d3; mat4->data[15] = aiMat.d4;
}

static void semesh_construct_material // only meant to be called form serender3d_load_mesh
(SE_Material *material, const struct aiMesh *ai_mesh, const char *filepath, const struct aiScene *scene) {
    // find the directory part of filepath
    SE_String filepath_string;
    sestring_init(&filepath_string, filepath);

    SE_String dir;
    sestring_init(&dir, "");

    u32 slash_index = sestring_lastof(&filepath_string, '/');
    if (slash_index == SESTRING_MAX_SIZE) {
        sestring_append(&dir, "/");
    } else if (slash_index == 0) {
        sestring_append(&dir, ".");
    } else {
        sestring_append_length(&dir, filepath, slash_index);
        sestring_append(&dir, "/");
    }

    // now add the texture path to directory
    const struct aiMaterial *ai_material = scene->mMaterials[ai_mesh->mMaterialIndex];

    SE_String diffuse_path;
    SE_String specular_path;
    SE_String normal_path;

    sestring_init(&diffuse_path, dir.buffer);
    sestring_init(&specular_path, dir.buffer);
    sestring_init(&normal_path, dir.buffer);

    struct aiString *ai_texture_path_diffuse  = new(struct aiString);
    struct aiString *ai_texture_path_specular = new(struct aiString);
    struct aiString *ai_texture_path_normal   = new(struct aiString);

    bool has_diffuse  = true;
    bool has_specular = true;
    bool has_normal   = true;

    if (AI_SUCCESS != aiGetMaterialTexture(ai_material, aiTextureType_DIFFUSE , 0, ai_texture_path_diffuse, NULL, NULL, NULL, NULL, NULL, NULL)) {
        has_diffuse = false;
    }

    if (AI_SUCCESS != aiGetMaterialTexture(ai_material, aiTextureType_SPECULAR, 0, ai_texture_path_specular, NULL, NULL, NULL, NULL, NULL, NULL)) {
        has_specular = false;
    }

    if (AI_SUCCESS != aiGetMaterialTexture(ai_material, aiTextureType_NORMALS , 0, ai_texture_path_normal, NULL, NULL, NULL, NULL, NULL, NULL)) {
        has_normal = false;
    }

    /* diffuse */
    material->base_diffuse = (Vec4) {1, 1, 1, 1};
    if (has_diffuse) {
        sestring_append(&diffuse_path, ai_texture_path_diffuse->data);
        setexture_load(&material->texture_diffuse , diffuse_path.buffer);
    } else {
        setexture_load(&material->texture_diffuse, "assets/textures/checkerboard.png");
    }
    free(ai_texture_path_diffuse);

    /* specular */
    if (has_specular) {
        sestring_append(&specular_path, ai_texture_path_specular->data);
        setexture_load(&material->texture_specular, specular_path.buffer);
    }
    free(ai_texture_path_specular);

    /* normal */
    if (has_normal) {
        sestring_append(&normal_path, ai_texture_path_normal->data);
        setexture_load(&material->texture_normal  , normal_path.buffer);
    } else {
        setexture_load(&material->texture_diffuse, "assets/textures/default_normal.png");
    }
    free(ai_texture_path_normal);

    sestring_deinit(&diffuse_path);
    sestring_deinit(&specular_path);
    sestring_deinit(&normal_path);

    sestring_deinit(&dir);
}

static void semesh_construct_normal_mesh // only meant to be called from serender3d_load_mesh
(SE_Mesh *mesh, const struct aiMesh *ai_mesh, const char *filepath, const struct aiScene *scene) {
    sedefault_mesh(mesh);
    u32 verts_count = 0;
    u32 index_count = 0;
    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * ai_mesh->mNumVertices);
    u32       *indices = malloc(sizeof(u32) * ai_mesh->mNumFaces * 3);

    // -- this is a normal static mesh
    mesh->type = SE_MESH_TYPE_NORMAL;

        // vertices
    for (u32 i = 0; i < ai_mesh->mNumVertices; ++i) {
        SE_Vertex3D vertex = {0};

        // -- pos
        vertex.position.x = ai_mesh->mVertices[i].x;
        vertex.position.y = ai_mesh->mVertices[i].y;
        vertex.position.z = ai_mesh->mVertices[i].z;

        // -- normals
        vertex.normal.x = ai_mesh->mNormals[i].x;
        vertex.normal.y = ai_mesh->mNormals[i].y;
        vertex.normal.z = ai_mesh->mNormals[i].z;

        // -- tangents // @incomplete we assume we have tangent and bi-tangent (because we've passed in a flag to calculate those) investigate
        vertex.tangent.x = ai_mesh->mTangents[i].x;
        vertex.tangent.y = ai_mesh->mTangents[i].y;
        vertex.tangent.z = ai_mesh->mTangents[i].z;

        // -- bi-tangents
        vertex.bitangent.x = ai_mesh->mBitangents[i].x;
        vertex.bitangent.y = ai_mesh->mBitangents[i].y;
        vertex.bitangent.z = ai_mesh->mBitangents[i].z;

        // -- uvs
        if (ai_mesh->mTextureCoords[0] != NULL) { // if this mesh has uv mapping
            vertex.texture_coord.x = ai_mesh->mTextureCoords[0][i].x;
            vertex.texture_coord.y = ai_mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texture_coord = v2f(0, 0);
        }

        verts[verts_count] = vertex;
        verts_count++;
    }

        // indices
    for (u32 i = 0; i < ai_mesh->mNumFaces; ++i) {
        // ! we triangulate on import, so every face has three vertices
        indices[index_count+0] = ai_mesh->mFaces[i].mIndices[0];
        indices[index_count+1] = ai_mesh->mFaces[i].mIndices[1];
        indices[index_count+2] = ai_mesh->mFaces[i].mIndices[2];
        index_count += 3;
    }

    semesh_generate(mesh, verts_count, verts, index_count, indices);

    free(verts);
    free(indices);
}

#if 0 // no reason to implement this yet.
static void recursive_read_animated_bones(const struct aiAnimation *animation, SE_Skeleton *skeleton) {
        // read the channels / animated bones and their keyframes
    for (u32 i = 0; i < animation->mNumChannels; ++i) {
            // NOTE(Matin): If we were not able to find the bone in skeleton for some reason, add it.
            // Sometimes when animations in fbx are loaded separately they contain additional bones not
            // found in the mesh fbx file itself. Might want to investigate this.
        if () {

        }
    }
}
#endif

static void recursive_read_bone_heirarchy
(SE_Skeleton *skeleton, i32 parent_id, Mat4 parent_model_space_transform, const struct aiNode *src) {
    se_assert(src);

        // See if this node is a bone node by comparing names with the bones
        // found in the mesh (as loaded to "skeleton" previously)
    bool found = false;
    SE_String src_name;
    sestring_init(&src_name, src->mName.data);
    for (u32 i = 0; i < skeleton->bone_count; ++i) {
        if (sestring_compare(&skeleton->bones_info[i].name, &src_name)) {
            found = true;
            break;
        }
    }
    sestring_deinit(&src_name);

        // if this node is a bone node, add it to the skeleton's node heirarchy
    SE_Bone_Node *new_bone_node = NULL;
    Mat4 model_space_transform_of_this_node;
    if (found) {
        new_bone_node = &skeleton->bone_nodes[skeleton->bone_node_count];
        new_bone_node->bones_info_index = skeleton->bone_node_count;
        skeleton->bone_node_count++;

        new_bone_node->parent = parent_id;
        sestring_init(&new_bone_node->name, src->mName.data);
        copy_ai_matrix_to_mat4(src->mTransformation, &new_bone_node->local_transform);
            //- inverse model space transform of the bone
        model_space_transform_of_this_node = mat4_mul(parent_model_space_transform, new_bone_node->local_transform);
        new_bone_node->inverse_neutral_transform = mat4_inverse(model_space_transform_of_this_node);

            //- link this node to its parent
        if (parent_id >= 0) {
            skeleton->bone_nodes[parent_id].children[skeleton->bone_nodes[parent_id].children_count] = new_bone_node->bones_info_index;
            skeleton->bone_nodes[parent_id].children_count++;
        }

        printf("CREATING A NEW BONE NODE WITH ID %i\n", new_bone_node->bones_info_index);
    }

    for (u32 i = 0; i < src->mNumChildren; ++i) {
        if (new_bone_node == NULL) {
            recursive_read_bone_heirarchy(skeleton, parent_id, parent_model_space_transform, src->mChildren[i]);
        } else {
            recursive_read_bone_heirarchy(skeleton, new_bone_node->bones_info_index, model_space_transform_of_this_node, src->mChildren[i]);
        }
    }
}

static void semesh_construct_skinned_mesh // only meant to be called from serender3d_load_mesh
(SE_Mesh *mesh, const struct aiMesh *ai_mesh, const struct aiScene *scene) {
    sedefault_mesh(mesh);
    u32 verts_count = 0;
    u32 index_count = 0;
    SE_Skinned_Vertex *verts = malloc(sizeof(SE_Skinned_Vertex) * ai_mesh->mNumVertices);
    u32             *indices = malloc(sizeof(u32) * ai_mesh->mNumFaces * 3);

        // remember that this is a skinned mesh
    mesh->type = SE_MESH_TYPE_SKINNED;

        // vertices
    for (u32 i = 0; i < ai_mesh->mNumVertices; ++i) {
        SE_Skinned_Vertex vertex = {0};
            // set the bone data to their default values
        for (i32 j = 0; j < SE_MAX_BONE_WEIGHTS; ++j) {
                // NOTE(Matin): we set vertex.bone_ids and vertex.bone_weights later on during the
                // extraction of bone vertex ids and bone weights from the ai_mesh further down...
            vertex.bone_ids[j] = -1;
            vertex.bone_weights[j] = 0.0f;
        }
            // pos
        vertex.vert.position.x = ai_mesh->mVertices[i].x;
        vertex.vert.position.y = ai_mesh->mVertices[i].y;
        vertex.vert.position.z = ai_mesh->mVertices[i].z;

            // normals
        vertex.vert.normal.x = ai_mesh->mNormals[i].x;
        vertex.vert.normal.y = ai_mesh->mNormals[i].y;
        vertex.vert.normal.z = ai_mesh->mNormals[i].z;

            // tangents // @incomplete we assume we have tangent and bi-tangent (because we've passed in a flag to calculate those) investigate
        vertex.vert.tangent.x = ai_mesh->mTangents[i].x;
        vertex.vert.tangent.y = ai_mesh->mTangents[i].y;
        vertex.vert.tangent.z = ai_mesh->mTangents[i].z;

            // bi-tangents
        vertex.vert.bitangent.x = ai_mesh->mBitangents[i].x;
        vertex.vert.bitangent.y = ai_mesh->mBitangents[i].y;
        vertex.vert.bitangent.z = ai_mesh->mBitangents[i].z;

            // uvs
        if (ai_mesh->mTextureCoords[0] != NULL) { // if this mesh has uv mapping
            vertex.vert.texture_coord.x = ai_mesh->mTextureCoords[0][i].x;
            vertex.vert.texture_coord.y = ai_mesh->mTextureCoords[0][i].y;
        } else {
            vertex.vert.texture_coord = v2f(0, 0);
        }

        verts[verts_count] = vertex;
        verts_count++;
    }

        // indices
    for (u32 i = 0; i < ai_mesh->mNumFaces; ++i) {
        // ! we triangulate on import, so every face has three vertices
        indices[index_count+0] = ai_mesh->mFaces[i].mIndices[0];
        indices[index_count+1] = ai_mesh->mFaces[i].mIndices[1];
        indices[index_count+2] = ai_mesh->mFaces[i].mIndices[2];
        index_count += 3;
    }

    {   //- extract bone info for each vertex
        mesh->skeleton = new (SE_Skeleton); // @leak
        mesh->skeleton->bone_count = 0;
            // copy the bone data to skeleton
        for (i32 bone_index = 0; bone_index < ai_mesh->mNumBones; ++bone_index) {
                // add bone to skeleton
            i32 bone_id = mesh->skeleton->bone_count;
            mesh->skeleton->bone_count++;
            se_assert(bone_id != -1); // just to be clear

                // copy the bone info over
            SE_Bone_Info *bone = &mesh->skeleton->bones_info[bone_index];
            bone->id = bone_id;
            sestring_init(&bone->name, ai_mesh->mBones[bone_index]->mName.data); // @leak we never deinit this string

            copy_ai_matrix_to_mat4(ai_mesh->mBones[bone_index]->mOffsetMatrix, &bone->offset);

                // copy over the vertex ids that this bone affects and the weights
            for (i32 weight_index = 0; weight_index < ai_mesh->mBones[bone_index]->mNumWeights; ++weight_index) {
                i32 vert_id = ai_mesh->mBones[bone_index]->mWeights[weight_index].mVertexId;
                f32 weight  = ai_mesh->mBones[bone_index]->mWeights[weight_index].mWeight;
                {   // set vertex bone data
                    for (i32 xxx = 0; xxx < SE_MAX_BONE_WEIGHTS; ++xxx) {
                        if (verts[vert_id].bone_ids[xxx] < 0) {
                            verts[vert_id].bone_ids[xxx] = bone_id;
                            verts[vert_id].bone_weights[xxx] = weight;
                        }
                    }
                }
            }
        }
        se_assert(mesh->skeleton->bone_count == ai_mesh->mNumBones);
    }
    {   //- bone node heirarchy
        recursive_read_bone_heirarchy(mesh->skeleton, -1, mat4_identity(), scene->mRootNode);
        se_assert(mesh->skeleton->bone_count == mesh->skeleton->bone_node_count); // @debug
        {   // @debug
            for (u32 i = 0; i < mesh->skeleton->bone_node_count; ++i) {
                se_assert(mesh->skeleton->bone_nodes[i].bones_info_index >= 0 && mesh->skeleton->bone_nodes[i].bones_info_index < mesh->skeleton->bone_count);
            }
        }
    }

        // generate the vao
    semesh_generate_skinned(mesh, verts_count, verts, index_count, indices);

    free(verts);
    free(indices);
}

//// ANIMATION BONES ////

    /// Gets the normalised value of 'amount' for lerp and slerp
static f32 get_scale_factor(f32 last_time_stamp, f32 next_time_stamp, f32 animation_time) {
    f32 mid_way_length = animation_time - last_time_stamp;
    f32 frame_delta    = next_time_stamp - last_time_stamp;
    return mid_way_length / frame_delta;
}

    /// Returns the interpolated translation of the bone at the given animation time
static Mat4 interpolate_bone_pos(SE_Bone_Animations *bone, f32 animation_time) {
    if (bone->position_count == 0) return mat4_identity();
    if (bone->position_count == 1) {
        return mat4_translation(bone->positions[0]);
    }

        // find index 0 and 1
    i32 index_0;
    for (u32 i = 0; i < bone->position_count - 1; ++i) {
        if (animation_time < bone->position_time_stamps[i + 1]) {
            index_0 = i;
            break;
        }
    }
    i32 index_1 = index_0 + 1;

        // interpolate
    f32 scale_factor = get_scale_factor(bone->position_time_stamps[index_0], bone->position_time_stamps[index_1], animation_time);
    Vec3 interpolated_pos = vec3_lerp(bone->positions[index_0], bone->positions[index_1], scale_factor);
    return mat4_translation(interpolated_pos);
}

    /// Returns the interpolated rotation of the bone at the given animation time
static Mat4 interpolate_bone_rot(SE_Bone_Animations *bone, f32 animation_time) {
    if (bone->rotation_count == 0) return mat4_identity();
    if (bone->rotation_count == 1) {
        Quat rot = quat_normalize(bone->rotations[0]);
        return quat_to_mat4(rot);
    }

        // find index 0 and 1
    i32 index_0;
    for (u32 i = 0; i < bone->rotation_count - 1; ++i) {
        if (animation_time < bone->rotation_time_stamps[i + 1]) {
            index_0 = i;
            break;
        }
    }
    i32 index_1 = index_0 + 1;

        // interpolate
    f32 scale_factor = get_scale_factor(bone->rotation_time_stamps[index_0], bone->rotation_time_stamps[index_1], animation_time);
    Quat interpolated_rot = quat_slerp(bone->rotations[index_0], bone->rotations[index_1], scale_factor);
    interpolated_rot = quat_normalize(interpolated_rot);
    // return quat_to_mat4(interpolated_rot);
    return quat_to_rotation_matrix(interpolated_rot, v3f(0,0,0));
}

    /// Returns the interpolated scale of the bone at the given animation time
static Mat4 interpolate_bone_scale(SE_Bone_Animations *bone, f32 animation_time) {
    if (bone->scale_count == 0) return mat4_identity();
    if (bone->scale_count == 1) {
        return mat4_scale(bone->scales[0]);
    }

        // find index 0 and 1
    i32 index_0;
    for (u32 i = 0; i < bone->scale_count - 1; ++i) {
        if (animation_time < bone->scale_time_stamps[i + 1]) {
            index_0 = i;
            break;
        }
    }
    i32 index_1 = index_0 + 1;

        // interpolate
    f32 scale_factor = get_scale_factor(bone->scale_time_stamps[index_0], bone->scale_time_stamps[index_1], animation_time);
    Vec3 interpolated_scale = vec3_lerp(bone->scales[index_0], bone->scales[index_1], scale_factor);
    return mat4_scale(interpolated_scale);
}

static Mat4 get_interpolated_bone_transform(SE_Bone_Animations *bone, f32 animation_time) {
    Mat4 translation = interpolate_bone_pos   (bone, animation_time);
    Mat4 rotation    = interpolate_bone_rot   (bone, animation_time);
    Mat4 scale       = interpolate_bone_scale (bone, animation_time);

    Mat4 result = scale;
    result = mat4_mul(result, rotation);
    result = mat4_mul(result, translation);
    return result;
}

static void bone_animations_deinit(SE_Bone_Animations *bone) {
    // bone->bone_node_index = -1;
    sestring_deinit(&bone->name);
    free(bone->positions);
    free(bone->rotations);
    free(bone->scales);
    free(bone->position_time_stamps);
    free(bone->rotation_time_stamps);
    free(bone->scale_time_stamps);
}

    /// Populates the given bone with animation data
static void bone_animations_init(SE_Bone_Animations *bone, const struct aiNodeAnim *channel) {
    // bone->bone_node_index = bone_node_index;
    bone->position_count  = channel->mNumPositionKeys;
    bone->rotation_count  = channel->mNumRotationKeys;
    bone->scale_count     = channel->mNumScalingKeys;
    sestring_init(&bone->name, channel->mNodeName.data);

        // allocate memory for the arrays
    bone->positions = malloc(sizeof(Vec3) * bone->position_count);
    bone->rotations = malloc(sizeof(Quat) * bone->rotation_count);
    bone->scales    = malloc(sizeof(Vec3) * bone->scale_count);
    bone->position_time_stamps = malloc(sizeof(f32) * bone->position_count);
    bone->rotation_time_stamps = malloc(sizeof(f32) * bone->rotation_count);
    bone->scale_time_stamps    = malloc(sizeof(f32) * bone->scale_count);

        // copy the data over
    for (u32 i = 0; i < bone->position_count; ++i) {
        bone->positions[i] = (Vec3) {
            .x = channel->mPositionKeys[i].mValue.x,
            .y = channel->mPositionKeys[i].mValue.y,
            .z = channel->mPositionKeys[i].mValue.z
        };
        bone->position_time_stamps[i] = (f32) channel->mPositionKeys[i].mTime;
    }

    for (u32 i = 0; i < bone->rotation_count; ++i) {
        bone->rotations[i] = (Quat) {
            .x = channel->mRotationKeys[i].mValue.x,
            .y = channel->mRotationKeys[i].mValue.y,
            .z = channel->mRotationKeys[i].mValue.z,
            .w = channel->mRotationKeys[i].mValue.w
        };
        bone->rotation_time_stamps[i] = (f32) channel->mRotationKeys[i].mTime;
    }

    for (u32 i = 0; i < bone->scale_count; ++i) {
        bone->scales[i] = (Vec3) {
            .x = channel->mScalingKeys[i].mValue.x,
            .y = channel->mScalingKeys[i].mValue.y,
            .z = channel->mScalingKeys[i].mValue.z
        };
        bone->scale_time_stamps[i] = (f32) channel->mScalingKeys[i].mTime;
    }
}

static void recursive_calculate_bone_pose // calculate the pose of the given bone based on the animation, do the same for its children
(SE_Skeleton *skeleton, const SE_Skeletal_Animation *animation, f32 animation_time, const SE_Bone_Node *node, Mat4 parent_transform) {
    se_assert(node->bones_info_index >= 0 && node->bones_info_index < animation->animated_bones_count);

    SE_Bone_Animations *animated_bone = NULL;
    for (u32 i = 0; i < animation->animated_bones_count; ++i) {
        if (sestring_compare(&animation->animated_bones[i].name, &node->name)) { // @performance: change this from a string compare to a hash compare
            animated_bone = &animation->animated_bones[i];
            break;
        }
    }

    Mat4 global_node_transform = node->local_transform;
        // the node transform with its parents taken into account
    if (animated_bone != NULL) {
        global_node_transform = get_interpolated_bone_transform(animated_bone, animation_time);
        global_node_transform = mat4_mul(global_node_transform, parent_transform);
    }

    if (node->bones_info_index >= 0 && node->bones_info_index < skeleton->bone_count) {
        i32 index = skeleton->bones_info[node->bones_info_index].id;
        Mat4 offset = skeleton->bones_info[node->bones_info_index].offset;
            // inverse neutral pose
        // global_node_transform = mat4_mul(node->inverse_neutral_transform, global_node_transform);
        skeleton->final_pose[index] = mat4_mul(offset, global_node_transform);
    }

        // repeat for children
    for (u32 i = 0; i < node->children_count; ++i) {
        recursive_calculate_bone_pose(skeleton, animation, animation_time, &skeleton->bone_nodes[node->children[i]], global_node_transform);
    }
}

static void recursive_calc_skeleton_pose_without_animation(SE_Skeleton *skeleton) {
    for (u32 i = 0; i < SE_SKELETON_BONES_CAPACITY; ++i) {
        skeleton->final_pose[i] = seskeleton_get_bone_final_transform(skeleton, i);
    }
}

void seskeleton_calculate_pose
(SE_Skeleton *skeleton, f32 frame) {
    se_assert(skeleton->animations_count > 0);
    // if (skeleton->animations_count > 0) {
        recursive_calculate_bone_pose(skeleton, skeleton->animations[skeleton->current_animation], frame, &skeleton->bone_nodes[0], mat4_identity());
    // } else {
        // recursive_calc_skeleton_pose_without_animation(skeleton);
    // }
}

//// RENDERER ////

static void serender3d_render_set_material_uniforms_lit(const SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform) {
    u32 shader = renderer->shader_lit;
    seshader_use(renderer->shaders[shader]);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    // the good old days when debugging:
    // material->texture_diffuse.width = 100;
    /* vertex */
    seshader_set_uniform_mat4(renderer->shaders[shader], "projection_view_model", pvm);
    seshader_set_uniform_mat4(renderer->shaders[shader], "model_matrix", transform);
    seshader_set_uniform_vec3(renderer->shaders[shader], "camera_pos", renderer->current_camera->position);
    seshader_set_uniform_mat4(renderer->shaders[shader], "light_space_matrix", renderer->light_space_matrix);

    /* material uniforms */
    seshader_set_uniform_f32 (renderer->shaders[shader], "material.shininess", 0.1f);
    seshader_set_uniform_i32 (renderer->shaders[shader], "material.diffuse", 0);
    seshader_set_uniform_i32 (renderer->shaders[shader], "material.specular", 1);
    seshader_set_uniform_i32 (renderer->shaders[shader], "material.normal", 2);
    seshader_set_uniform_vec4(renderer->shaders[shader], "material.base_diffuse", material->base_diffuse);

    // directional light uniforms
    seshader_set_uniform_vec3(renderer->shaders[shader], "dir_light.direction", renderer->light_directional.direction);
    seshader_set_uniform_rgb (renderer->shaders[shader], "dir_light.ambient", renderer->light_directional.ambient);
    seshader_set_uniform_rgb (renderer->shaders[shader], "dir_light.diffuse", renderer->light_directional.diffuse);
    seshader_set_uniform_rgb (renderer->shaders[shader], "dir_light.specular", (RGB) {0, 0, 0});
    seshader_set_uniform_f32 (renderer->shaders[shader], "dir_light.intensity", renderer->light_directional.intensity);
    seshader_set_uniform_i32 (renderer->shaders[shader], "shadow_map", 3);

    // point light uniforms
    seshader_set_uniform_vec3(renderer->shaders[shader], "point_lights[0].position",  renderer->point_lights[0].position);
    seshader_set_uniform_rgb (renderer->shaders[shader], "point_lights[0].ambient",   renderer->point_lights[0].ambient);
    seshader_set_uniform_rgb (renderer->shaders[shader], "point_lights[0].diffuse",   renderer->point_lights[0].diffuse);
    seshader_set_uniform_rgb (renderer->shaders[shader], "point_lights[0].specular",  renderer->point_lights[0].specular);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].constant" , renderer->point_lights[0].constant);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].linear"   , renderer->point_lights[0].linear);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].quadratic", renderer->point_lights[0].quadratic);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].far_plane", 25.0f); // @temp magic value set to the projection far plane when calculating the shadow maps (cube texture)
    seshader_set_uniform_i32 (renderer->shaders[shader], "point_lights[0].shadow_map", 4); // ! need to change 4 to 4 + the index of light point once multiple point lights are supported

    /* textures */
    if (material->texture_diffuse.loaded) {
        setexture_bind(&material->texture_diffuse, 0);
    } else {
        setexture_bind(&renderer->texture_default_diffuse, 0);
    }

    if (material->texture_specular.loaded) {
        setexture_bind(&material->texture_specular, 1);
    } else {
        setexture_bind(&renderer->texture_default_specular, 1);
    }

    if (material->texture_normal.loaded) {
        setexture_bind(&material->texture_normal, 2);
    } else {
        setexture_bind(&renderer->texture_default_normal, 2);
    }

    glActiveTexture(GL_TEXTURE0 + 3); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.texture);

    /* omnidirectional shadow map */
    glActiveTexture(GL_TEXTURE0 + 4); // shadow map
    glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->point_lights[0].depth_cube_map);
}

static void serender3d_render_set_material_uniforms_lines(const SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform) {
    u32 shader = renderer->shader_lines;
    seshader_use(renderer->shaders[shader]);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    /* vertex */
    seshader_set_uniform_mat4(renderer->shaders[shader], "projection_view_model", pvm);

    /* material */
    seshader_set_uniform_vec4(renderer->shaders[shader], "base_diffuse", material->base_diffuse);
}

static void serender3d_render_set_material_uniforms_sprite(const SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform) {
    u32 shader = renderer->shader_sprite;
    seshader_use(renderer->shaders[shader]);

    /* always look at the camera */
    // Mat4 look_at_camera = mat4_lookat(mat4_get_translation(transform), renderer->current_camera->position, renderer->current_camera->up);
    // // transform = mat4_mul(transform, look_at_camera);
    // transform = mat4_mul(mat4_translation(mat4_get_translation(transform)), look_at_camera);
    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    /* vertex */
    seshader_set_uniform_mat4(renderer->shaders[shader], "projection_view_model", pvm);

    /* material */
    seshader_set_uniform_vec4(renderer->shaders[shader], "base_diffuse", material->base_diffuse);
    seshader_set_uniform_i32(renderer->shaders[shader], "sprite_texture", 0);

    /* textures */
    if (material->sprite.texture.loaded) {
        setexture_bind(&material->sprite.texture, 0);
    } else {
        setexture_bind(&renderer->texture_default_diffuse, 0);
    }
}

static u32 add_animation_to_skeleton(SE_Skeleton *skeleton) {
    u32 anim = skeleton->animations_count;
    skeleton->animations_count++;

    skeleton->animations[anim] = new(SE_Skeletal_Animation);
    memset(skeleton->animations[anim], 0, sizeof(SE_Skeletal_Animation));
    return anim;
}

static void load_animation(SE_Skeleton *skeleton, const char *model_filepath) {
        // load scene from file
    const struct aiScene *scene = aiImportFile(model_filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (scene == NULL) {
        printf("ERROR: could not mesh from %s (%s)\n", model_filepath, aiGetErrorString());
        return;
    }

    for (u32 i = 0; i < scene->mNumAnimations; ++i) {
            // add the animation to renderer
        u32 anim_index = add_animation_to_skeleton(skeleton);
        SE_Skeletal_Animation *anim = skeleton->animations[anim_index];
            // update the data of animation
        anim->duration = scene->mAnimations[i]->mDuration;
        anim->ticks_per_second = scene->mAnimations[i]->mTicksPerSecond;
        sestring_init(&anim->name, scene->mAnimations[i]->mName.data); // @leak we need to free anim->name (deinit)

            // load the data of each animated bone
        anim->animated_bones_count = scene->mAnimations[i]->mNumChannels;
        anim->animated_bones = malloc(sizeof(SE_Bone_Animations) * anim->animated_bones_count);
        for (u32 c = 0; c < scene->mAnimations[i]->mNumChannels; ++c) {
            SE_Bone_Animations *animated_bone = &anim->animated_bones[c];
            bone_animations_init(animated_bone, scene->mAnimations[i]->mChannels[c]);
        }
    }
}

u32 serender3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath, bool with_skeleton) {
    u32 result = -1;
        // load scene from file
    const struct aiScene *scene = aiImportFile(model_filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (scene == NULL) {
        printf("ERROR: could not mesh from %s (%s)\n", model_filepath, aiGetErrorString());
        return result;
    }
        //- load meshes within the scene
    result = renderer->meshes_count; // the first mesh in the chain
    for (u32 i = 0; i < scene->mNumMeshes; ++i) {
        struct aiMesh *ai_mesh = scene->mMeshes[i];

        // add a mesh to the renderer
        renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
        memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));

            //- load the skeleton of this mesh
        if (with_skeleton && ai_mesh->mNumBones > 0) {
                //- load a skinned mesh ready to be animated
            SE_Mesh *mesh = renderer->meshes[renderer->meshes_count];
            semesh_construct_skinned_mesh(mesh, ai_mesh, scene);
        } else {
                // load normal static mesh
            semesh_construct_normal_mesh(renderer->meshes[renderer->meshes_count], ai_mesh, model_filepath, scene);
        }

            //- load the material of this mesh
        if (scene->mNumMaterials > 0) { // -- materials
            u32 material_index = serender3d_add_material(renderer);
            renderer->meshes[renderer->meshes_count]->material_index = material_index;

            semesh_construct_material(renderer->materials[material_index], ai_mesh, model_filepath, scene);
        }

            //- Link meshes together:
            //- If there are multiple meshes within this scene, add them on in a linked list
        if (i > 0) {
            renderer->meshes[renderer->meshes_count-1]->next_mesh_index = result + i;
        }
        renderer->meshes_count++;
    }

        //- load animations associated with this mesh
    if (renderer->meshes[result]->skeleton != NULL && scene->mNumAnimations > 0) {
        i32 current_mesh = result;
        while (current_mesh >= 0) {
            load_animation(renderer->meshes[current_mesh]->skeleton, model_filepath);
            printf("-------------------------------\n");
            for (u32 i = 0; i < renderer->meshes[result]->skeleton->animations[0]->animated_bones_count; i++) {
                printf("%i: parent name: %s\n", i, renderer->meshes[result]->skeleton->animations[0]->animated_bones[i].name.buffer);
            }
            current_mesh = renderer->meshes[current_mesh]->next_mesh_index;
        }

    }


    // the final mesh in the linked list has no next (signified by -1 next_mesh_index)
    renderer->meshes[renderer->meshes_count - 1]->next_mesh_index = -1;

    return result;
}

static void
serender3d_render_set_material_uniforms_skinned(const SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform) {
    u32 shader = renderer->shader_skinned_mesh;
    seshader_use(renderer->shaders[shader]);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

     /* vertex */
    seshader_set_uniform_mat4(renderer->shaders[shader], "projection_view_model", pvm);
    seshader_set_uniform_mat4(renderer->shaders[shader], "model_matrix", transform);
    seshader_set_uniform_vec3(renderer->shaders[shader], "camera_pos", renderer->current_camera->position);
    seshader_set_uniform_mat4(renderer->shaders[shader], "light_space_matrix", renderer->light_space_matrix);

    /* material uniforms */
    seshader_set_uniform_f32 (renderer->shaders[shader], "material.shininess", 0.1f);
    seshader_set_uniform_i32 (renderer->shaders[shader], "material.diffuse", 0);
    seshader_set_uniform_i32 (renderer->shaders[shader], "material.specular", 1);
    seshader_set_uniform_i32 (renderer->shaders[shader], "material.normal", 2);
    seshader_set_uniform_vec4(renderer->shaders[shader], "material.base_diffuse", material->base_diffuse);

    // directional light uniforms
    seshader_set_uniform_vec3(renderer->shaders[shader], "dir_light.direction", renderer->light_directional.direction);
    seshader_set_uniform_rgb (renderer->shaders[shader], "dir_light.ambient", renderer->light_directional.ambient);
    seshader_set_uniform_rgb (renderer->shaders[shader], "dir_light.diffuse", renderer->light_directional.diffuse);
    seshader_set_uniform_rgb (renderer->shaders[shader], "dir_light.specular", (RGB) {0, 0, 0});
    seshader_set_uniform_f32 (renderer->shaders[shader], "dir_light.intensity", renderer->light_directional.intensity);
    seshader_set_uniform_i32 (renderer->shaders[shader], "shadow_map", 3);

    // point light uniforms
    seshader_set_uniform_vec3(renderer->shaders[shader], "point_lights[0].position",  renderer->point_lights[0].position);
    seshader_set_uniform_rgb (renderer->shaders[shader], "point_lights[0].ambient",   renderer->point_lights[0].ambient);
    seshader_set_uniform_rgb (renderer->shaders[shader], "point_lights[0].diffuse",   renderer->point_lights[0].diffuse);
    seshader_set_uniform_rgb (renderer->shaders[shader], "point_lights[0].specular",  renderer->point_lights[0].specular);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].constant" , renderer->point_lights[0].constant);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].linear"   , renderer->point_lights[0].linear);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].quadratic", renderer->point_lights[0].quadratic);
    seshader_set_uniform_f32 (renderer->shaders[shader], "point_lights[0].far_plane", 25.0f); // @temp magic value set to the projection far plane when calculating the shadow maps (cube texture)
    seshader_set_uniform_i32 (renderer->shaders[shader], "point_lights[0].shadow_map", 4); // ! need to change 4 to 4 + the index of light point once multiple point lights are supported

    /* textures */
    if (material->texture_diffuse.loaded) {
        setexture_bind(&material->texture_diffuse, 0);
    } else {
        setexture_bind(&renderer->texture_default_diffuse, 0);
    }

    if (material->texture_specular.loaded) {
        setexture_bind(&material->texture_specular, 1);
    } else {
        setexture_bind(&renderer->texture_default_specular, 1);
    }

    if (material->texture_normal.loaded) {
        setexture_bind(&material->texture_normal, 2);
    } else {
        setexture_bind(&renderer->texture_default_normal, 2);
    }

    glActiveTexture(GL_TEXTURE0 + 3); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.texture);

    /* omnidirectional shadow map */
    glActiveTexture(GL_TEXTURE0 + 4); // shadow map
    glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->point_lights[0].depth_cube_map);
}

static void
serender3d_render_set_material_uniforms_skinned_skeleton(const SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform) {
    u32 shader = renderer->shader_skinned_mesh_skeleton;
    seshader_use(renderer->shaders[shader]);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

     /* vertex */
    seshader_set_uniform_mat4(renderer->shaders[shader], "projection_view_model", pvm);
    seshader_set_uniform_mat4(renderer->shaders[shader], "model_matrix", transform);

    /* material uniforms */
    seshader_set_uniform_vec3 (renderer->shaders[shader], "base_diffuse", v3f(1, 0, 0));
}

void serender_mesh(const SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform) {
    serender3d_reset_render_config(); // Reset configs to their default values
    // take the mesh (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    i32 primitive = GL_TRIANGLES;
    SE_Material *material = renderer->materials[mesh->material_index];

    /* configs for this mesh */
    if (mesh->type == SE_MESH_TYPE_LINE) { // LINE
        primitive = GL_LINES;
        glLineWidth(mesh->line_width);
        if (mesh->skeleton != NULL && mesh->skeleton->animations_count > 0) {
                // used for animated skeleton
            serender3d_render_set_material_uniforms_skinned_skeleton(renderer, material, transform);
            seshader_set_uniform_mat4_array(renderer->shaders[renderer->shader_skinned_mesh_skeleton], "bones", mesh->skeleton->final_pose, SE_SKELETON_BONES_CAPACITY);
        } else {
                // render the line without animation
            serender3d_render_set_material_uniforms_lines(renderer, material, transform);
        }
    } else
    if (mesh->type == SE_MESH_TYPE_NORMAL) { // NORMAL
        serender3d_render_set_material_uniforms_lit(renderer, material, transform);
    } else
    if (mesh->type == SE_MESH_TYPE_SPRITE) { // SPRITE
        serender3d_render_set_material_uniforms_sprite(renderer, material, transform);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
    } else
    if (mesh->type == SE_MESH_TYPE_SKINNED) { // SKELETAL ANIMATION
        serender3d_render_set_material_uniforms_skinned(renderer, material, transform);
        // for (u32 i = 0; i < SE_SKELETON_BONES_CAPACITY; ++i) { // @temp // @debug for debugging purposes
        //     mesh->skeleton->final_pose[i] = mat4_identity();
        // }
        seshader_set_uniform_mat4_array(renderer->shaders[renderer->shader_skinned_mesh], "bones", mesh->skeleton->final_pose, SE_SKELETON_BONES_CAPACITY);
    } else
    if (mesh->type == SE_MESH_TYPE_POINT) { // MESH MADE OUT OF POINTS
        primitive = GL_POINTS;
        glPointSize(mesh->point_radius);
        serender3d_render_set_material_uniforms_lines(renderer, material, transform);
    }

    glBindVertexArray(mesh->vao);
    if (mesh->indexed) {
        glDrawElements(primitive, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->vert_count);
    }

    glBindVertexArray(0);
}

// make sure to call serender3d_render_mesh_setup before calling this procedure. Only needs to be done once.
void serender_mesh_index(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->meshes[mesh_index];
    serender_mesh(renderer, mesh, transform);

    // if (mesh->next_mesh_index > -1 && mesh->type != SE_MESH_TYPE_SKINNED) { // @temp checking if it's not skinned because for some reason the other mesh does not get a proper skeleton final pose
    if (mesh->next_mesh_index > -1) {
        serender_mesh_index(renderer, mesh->next_mesh_index, transform);
    }
}

void serender3d_render_mesh_outline(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->meshes[mesh_index];
    if (mesh->type == SE_MESH_TYPE_LINE) return;
    // take the mesh (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    SE_Mesh outline_mesh;
    outline_mesh.material_index = renderer->material_lines;
    semesh_generate_gizmos_aabb(&outline_mesh, mesh->aabb.min, mesh->aabb.max, 2);
    serender_mesh(renderer, &outline_mesh, transform);

    { // setup the shader
        u32 shader = renderer->shader_outline;
        seshader_use(renderer->shaders[shader]); // use the outline shader

        Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
        pvm = mat4_mul(pvm, renderer->current_camera->projection);

        seshader_set_uniform_mat4(renderer->shaders[shader], "_pvm", pvm);
        seshader_set_uniform_f32(renderer->shaders[shader], "_outline_width", 0.02f);
        seshader_set_uniform_rgb(renderer->shaders[shader], "_outline_colour", (RGB) {255, 255, 255});
        static f32 time = 0;
        time += 0.167;
        seshader_set_uniform_f32(renderer->shaders[shader], "_time", time);
    }

    glBindVertexArray(mesh->vao);
    glCullFace(GL_FRONT);
    if (mesh->indexed) {
        glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
    }
    glCullFace(GL_BACK);

    glBindVertexArray(0);

    semesh_deinit(&outline_mesh);
}

void serender3d_reset_render_config() {
    /* default */
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glLineWidth(1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend mode
}

static u32 serender3d_add_shader_with_geometry(SE_Renderer3D *renderer, const char *vsd, const char *fsd, const char *gsd) {
    // add a default shader
    u32 shader = renderer->shaders_count;
    renderer->shaders[shader] = new (SE_Shader);
    seshader_init_from_with_geometry(renderer->shaders[shader], vsd, fsd, gsd);
    renderer->shaders_count++;
    return shader;
}

u32 serender3d_add_shader(SE_Renderer3D *renderer, const char *vsd, const char *fsd) {
    // add a default shader
    u32 shader = renderer->shaders_count;
    renderer->shaders[shader] = new (SE_Shader);
    seshader_init_from(renderer->shaders[shader], vsd, fsd);
    renderer->shaders_count++;
    return shader;
}

void serender3d_init(SE_Renderer3D *renderer, SE_Camera3D *current_camera) {
    memset(renderer, 0, sizeof(SE_Renderer3D));
    renderer->current_camera = current_camera;
    renderer->light_directional.intensity = 0.5f;
    // point lights
    renderer->point_lights_count = 1;
    renderer->point_lights[0].position = v3f(2, 1, 1);
    renderer->point_lights[0].ambient   = (RGB) {100, 100, 100};
    renderer->point_lights[0].diffuse   = (RGB) {255, 255, 255};
    renderer->point_lights[0].specular  = (RGB) {0, 0, 0};
    renderer->point_lights[0].constant  = 1.0f;
    renderer->point_lights[0].linear    = 0.22f;
    renderer->point_lights[0].quadratic = 0.20f;

    /* default shaders */
    renderer->shader_lit = serender3d_add_shader(renderer, "shaders/lit.vsd", "shaders/lit_better.fsd");
    renderer->shader_shadow_calc = serender3d_add_shader(renderer, "shaders/shadow_calc.vsd", "shaders/shadow_calc.fsd");
    renderer->shader_shadow_omnidir_calc = serender3d_add_shader_with_geometry(renderer, "shaders/shadow_omni_calc.vsd", "shaders/shadow_omni_calc.fsd", "shaders/shadow_omni_calc.gsd");
    renderer->shader_lines = serender3d_add_shader(renderer, "shaders/lines.vsd", "shaders/lines.fsd");
    renderer->shader_outline = serender3d_add_shader(renderer, "shaders/outline.vsd", "shaders/outline.fsd");
    renderer->shader_sprite = serender3d_add_shader(renderer, "shaders/sprite.vsd", "shaders/sprite.fsd");
    renderer->shader_skinned_mesh = serender3d_add_shader(renderer, "shaders/skinned_vertex.vsd", "shaders/lit_better.fsd");
    renderer->shader_skinned_mesh_skeleton = serender3d_add_shader(renderer, "shaders/skinned_skeleton_lines.vsd", "shaders/lines.fsd");

    /* default materials */
    renderer->material_lines = serender3d_add_material(renderer);
    renderer->materials[renderer->material_lines]->base_diffuse = (Vec4) {1, 1, 1, 1};

    setexture_load(&renderer->texture_default_diffuse, "assets/textures/checkerboard.png");
    setexture_load(&renderer->texture_default_normal, "assets/textures/default_normal.png");
    setexture_load(&renderer->texture_default_specular, "assets/textures/default_specular.png");

    /* shadow mapping */
    f32 shadow_w = 1024;
    f32 shadow_h = 1024;
    serender_target_init(&renderer->shadow_render_target, (Rect) {0, 0, shadow_w, shadow_h}, true, true);

    { /* omnidirectional shadow mapping */
        SE_Light_Point *point_light = &renderer->point_lights[0];
        glGenTextures(1, &point_light->depth_cube_map); // @leak
        glBindTexture(GL_TEXTURE_CUBE_MAP, point_light->depth_cube_map);
        for (u32 i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
        glGenFramebuffers(1, &point_light->depth_map_fbo); // @leak
        glBindFramebuffer(GL_FRAMEBUFFER, point_light->depth_map_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, point_light->depth_cube_map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
}

void serender3d_deinit(SE_Renderer3D *renderer) {
    for (u32 i = 0; i < renderer->meshes_count; ++i) {
        semesh_deinit(renderer->meshes[i]);
    }
    renderer->meshes_count = 0;

    for (u32 i = 0; i < renderer->shaders_count; ++i) {
        seshader_deinit(renderer->shaders[i]);
    }
    renderer->shaders_count = 0;

    for (u32 i = 0; i < renderer->materials_count; ++i) {
        sematerial_deinit(renderer->materials[i]);
    }
    renderer->materials_count = 0;

    /* shadow mapping */
    serender_target_deinit(&renderer->shadow_render_target);

    /* default stuff */
    setexture_unload(&renderer->texture_default_diffuse);
    setexture_unload(&renderer->texture_default_normal);
    setexture_unload(&renderer->texture_default_specular);
}

u32 serender3d_add_material(SE_Renderer3D *renderer) {
    renderer->materials[renderer->materials_count] = new(SE_Material);
    memset(renderer->materials[renderer->materials_count], 0, sizeof(SE_Material));
    u32 material_index = renderer->materials_count;
    renderer->materials_count++;
    return material_index;
}

u32 serender3d_add_cube(SE_Renderer3D *renderer) {
    u32 result = renderer->meshes_count;

    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    semesh_generate_cube(renderer->meshes[renderer->meshes_count], vec3_one());

    renderer->meshes_count++;
    return result;
}

u32 serender3d_add_plane(SE_Renderer3D *renderer, Vec3 scale) {
    u32 result = renderer->meshes_count;

    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    semesh_generate_plane(renderer->meshes[renderer->meshes_count], scale);

    renderer->meshes_count++;
    return result;
}

u32 serender3d_add_sprite_mesh(SE_Renderer3D *renderer, Vec2 scale) {
    u32 result = renderer->meshes_count;

    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    semesh_generate_sprite(renderer->meshes[renderer->meshes_count], scale);

    renderer->meshes_count++;
    return result;
}

u32 serender3d_add_line(SE_Renderer3D *renderer, Vec3 pos1, Vec3 pos2, f32 width) {
    u32 result = renderer->meshes_count;

    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    semesh_generate_line(renderer->meshes[renderer->meshes_count], pos1, pos2, width);

    renderer->meshes_count++;
    return result;
}

u32 serender3d_add_mesh_empty(SE_Renderer3D *renderer) {
    u32 result = renderer->meshes_count;
    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    sedefault_mesh(renderer->meshes[renderer->meshes_count]);
    renderer->meshes_count++;
    return result;
}

u32 serender3d_add_gizmos_coordniates(SE_Renderer3D *renderer, f32 scale, f32 width) {
    u32 result = renderer->meshes_count;

    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    semesh_generate_gizmos_coordinates(renderer->meshes[renderer->meshes_count], scale, width);

    renderer->meshes_count++;
    return result;
}

u32 serender3d_add_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width) {
    u32 result = renderer->meshes_count;

    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    semesh_generate_gizmos_aabb(renderer->meshes[renderer->meshes_count], min, max, line_width);

    renderer->meshes_count++;
    return result;
}

void serender3d_update_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width, u32 mesh_index) {

    memset(renderer->meshes[mesh_index], 0, sizeof(SE_Mesh));
    semesh_generate_gizmos_aabb(renderer->meshes[mesh_index], min, max, line_width);
}