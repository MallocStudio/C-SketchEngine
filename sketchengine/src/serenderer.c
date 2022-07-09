#include "serenderer_util.h"

///
/// Materials
///

void se_material_deinit(SE_Material *material) {
    if (material->texture_diffuse.loaded)  se_texture_unload(&material->texture_diffuse);
    if (material->texture_specular.loaded) se_texture_unload(&material->texture_specular);
    if (material->texture_normal.loaded)   se_texture_unload(&material->texture_normal);
}

///
/// MESH
///

void se_mesh_deinit(SE_Mesh *mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ibo);
    mesh->material_index = 0;
    if (mesh->type == SE_MESH_TYPE_SKINNED) {
        free(mesh->skeleton);
    }
}

void se_mesh_generate_quad(SE_Mesh *mesh, Vec2 scale) { // 2d plane
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

    se_mesh_generate(mesh, 4, verts, 6, indices);
}

void se_mesh_generate_sprite(SE_Mesh *mesh, Vec2 scale) {
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

    se_mesh_generate(mesh, 4, verts, 6, indices);
}

void se_mesh_generate_cube(SE_Mesh *mesh, Vec3 scale) {
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
    se_mesh_generate(mesh, 8, verts, 12 * 3, indices);
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

    se_mesh_generate(mesh, 4, verts, 6, indices);
}

void se_mesh_generate_line(SE_Mesh *mesh, Vec3 pos1, Vec3 pos2, f32 width, RGBA colour) {
    sedefault_mesh(mesh);
    mesh->type = SE_MESH_TYPE_LINE;
    mesh->line_width = width;

    SE_Vertex3D verts[2] = {
        {.position = pos1},
        {.position = pos2}
    };

    verts[0].colour = colour;
    verts[1].colour = colour;

    u32 indices[2] = {
        0, 1
    };
    se_mesh_generate(mesh, 2, verts, 2, indices);
}

void se_mesh_generate_line_fan(SE_Mesh *mesh, Vec3 origin, Vec3 *positions, u32 positions_count, f32 line_width) {
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
    se_mesh_generate(mesh, positions_count + 1, verts, positions_count * 2, indices);

    free(verts);
    free(indices);
}

void se_mesh_generate_gizmos_aabb(SE_Mesh *mesh, Vec3 min, Vec3 max, f32 line_width) {
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

    verts[0].colour = RGBA_WHITE;
    verts[1].colour = RGBA_WHITE;
    verts[2].colour = RGBA_WHITE;
    verts[3].colour = RGBA_WHITE;
    verts[4].colour = RGBA_WHITE;
    verts[5].colour = RGBA_WHITE;
    verts[6].colour = RGBA_WHITE;
    verts[7].colour = RGBA_WHITE;

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
    se_mesh_generate(mesh, 8, verts, 24, indices);
}

void se_mesh_generate_gizmos_coordinates(SE_Mesh *mesh, f32 width) {
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

    verts[0].colour = RGBA_RED;
    verts[1].colour = RGBA_RED;
    verts[2].colour = RGBA_GREEN;
    verts[3].colour = RGBA_GREEN;
    verts[4].colour = RGBA_BLUE;
    verts[5].colour = RGBA_BLUE;

    u32 indices[6] = {
        0, 1, // x
        2, 3, // y
        4, 5, // z
    };
    se_mesh_generate(mesh, 6, verts, 6, indices);
}

void se_mesh_generate_static_skeleton
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

#define DEBUG_BONE_INVERSE_NEUTRAL_TRANSFORM
#ifdef DEBUG_BONE_INVERSE_NEUTRAL_TRANSFORM // render each bone's inverse_neutral_transform for debugging purposes
        // generate the verts based on the model space transform of each bone
    for (u32 i = 0; i < skeleton->bone_node_count; ++i) {
        verts[vert_count].position = mat4_get_translation(mat4_inverse(skeleton->bone_nodes[i].inverse_neutral_transform));
        indices[index_count] = i;
        vert_count++;
        index_count++;
    }
#else
    recursive_generate_static_skeleton_verts(skeleton, verts, &vert_count, indices, &index_count,
                                            &skeleton->bone_nodes[0], mat4_identity());
#endif

        //- mesh settings
    mesh->vert_count = index_count;
    mesh->indexed = true;
    mesh->aabb = (AABB3D) {0};
#ifdef DEBUG_BONE_INVERSE_NEUTRAL_TRANSFORM
    mesh->point_radius = 3; // @temp
    mesh->type = SE_MESH_TYPE_POINT; // sense we're going to generate a skeleton we're going to be lines
#else
    mesh->line_width = 2;
    mesh->type = SE_MESH_TYPE_LINE; // sense we're going to generate a skeleton we're going to be lines
#endif
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

void se_mesh_generate_skinned_skeleton
(SE_Mesh *mesh, SE_Skeleton *skeleton, b8 line, b8 with_animation) {
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

void se_mesh_generate(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices, u32 index_count, u32 *indices) {
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

    if (mesh->type == SE_MESH_TYPE_NORMAL || mesh->type == SE_MESH_TYPE_LINE || mesh->type == SE_MESH_TYPE_POINT) {
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

        if (mesh->type == SE_MESH_TYPE_LINE || mesh->type == SE_MESH_TYPE_POINT) {
            // -- enable colour
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, TYPEOF_RGBA_OPENGL, GL_TRUE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, colour));
        }

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
    mesh->aabb = se_mesh_calc_aabb(vertices, vert_count);

    // unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//// RENDER 3D ////

AABB3D se_mesh_calc_aabb_skinned(const SE_Skinned_Vertex *verts, u32 verts_count) {
    f32 xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;

    for (u32 i = 0; i < verts_count; ++i) {
        Vec3 vert_pos = verts[i].vert.position;
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

AABB3D se_mesh_calc_aabb(const SE_Vertex3D *verts, u32 verts_count) {
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
    return se_mesh_calc_aabb(dummy_verts, 8);
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

void se_skeleton_calculate_pose
(SE_Skeleton *skeleton, f32 frame) {
    se_assert(skeleton->animations_count > 0);
    // if (skeleton->animations_count > 0) {
        recursive_calculate_bone_pose(skeleton, skeleton->animations[skeleton->current_animation], frame, &skeleton->bone_nodes[0], mat4_identity());
    // } else {
        // recursive_calc_skeleton_pose_without_animation(skeleton);
    // }
}

u32 se_render3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath, b8 with_skeleton) {
    u32 result = -1;
        // load scene from file
    const struct aiScene *scene = aiImportFile(model_filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (scene == NULL) {
        printf("ERROR: could not mesh from %s (%s)\n", model_filepath, aiGetErrorString());
        return result;
    }
        //- load meshes within the scene
    result = renderer->user_meshes_count; // the first mesh in the chain

    SE_Skeleton *skeleton = NULL;
    if (with_skeleton) {
        skeleton = NEW (SE_Skeleton);
            // it's important for bone_nodes.children_count be zero and so should the other things
        memset(skeleton, 0, sizeof(SE_Skeleton));
    }

    for (u32 i = 0; i < scene->mNumMeshes; ++i) {
        struct aiMesh *ai_mesh = scene->mMeshes[i];

        // add a mesh to the renderer
        renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
        memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));

            //- load the skeleton of this mesh
        if (with_skeleton && ai_mesh->mNumBones > 0) {
                // load a skinned mesh ready to be animated
            SE_Mesh *mesh = renderer->user_meshes[renderer->user_meshes_count];
            semesh_construct_skinned_mesh(mesh, skeleton, ai_mesh, scene);
        } else {
                // load normal static mesh
            semesh_construct_normal_mesh(renderer->user_meshes[renderer->user_meshes_count], ai_mesh, model_filepath, scene);
        }

            //- load the material of this mesh
        if (scene->mNumMaterials > 0) { // -- materials
            u32 material_index = se_render3d_add_material(renderer);
            renderer->user_meshes[renderer->user_meshes_count]->material_index = material_index;

            semesh_construct_material(renderer->user_materials[material_index], ai_mesh, model_filepath, scene);
        }

            //- Link meshes together: If there are multiple meshes within this scene, add them on in a linked list
        if (i > 0) {
            renderer->user_meshes[renderer->user_meshes_count-1]->next_mesh_index = result + i;
        }
        renderer->user_meshes_count++;
    }

        //- load animations associated with this mesh
    if (renderer->user_meshes[result]->skeleton != NULL && scene->mNumAnimations > 0) {
        i32 current_mesh = result;
        while (current_mesh >= 0) {
            load_animation(renderer->user_meshes[current_mesh]->skeleton, scene);
#if 0 // debug
            printf("-------------------------------\n");
            for (u32 i = 0; i < renderer->meshes[result]->skeleton->animations[0]->animated_bones_count; i++) {
                printf("%i: parent name: %s\n", i, renderer->meshes[result]->skeleton->animations[0]->animated_bones[i].name.buffer);
            }
#endif
            current_mesh = renderer->user_meshes[current_mesh]->next_mesh_index;
        }
    }

        //- the final mesh in the linked list has no next (signified by -1 next_mesh_index)
    renderer->user_meshes[renderer->user_meshes_count - 1]->next_mesh_index = -1;

    aiReleaseImport(scene);
    return result;
}

void se_render_mesh(SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform) {
    se_render3d_reset_render_config(); // Reset configs to their default values
    // take the mesh (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    i32 primitive = GL_TRIANGLES;
    SE_Material *material = renderer->user_materials[mesh->material_index];

    /* configs for this mesh */
    if (mesh->type == SE_MESH_TYPE_LINE) { // LINE
        primitive = GL_LINES;
        glLineWidth(mesh->line_width);
        if (mesh->skeleton != NULL && mesh->skeleton->animations_count > 0) {
        //- LINE
                // used for animated skeleton
            set_material_uniforms_skinned_skeleton(renderer, material, transform, mesh->skeleton->final_pose);
        } else {
        //- ANIMATED LINES
                // render the line without animation
            serender3d_render_set_material_uniforms_lines(renderer, transform);
        }
    } else
    if (mesh->type == SE_MESH_TYPE_NORMAL) { // NORMAL
        //- STATIC MESH
        util_serender3d_render_set_material_uniforms_lit(renderer, material, transform);
    } else
    if (mesh->type == SE_MESH_TYPE_SPRITE) { // SPRITE
        //- SPRITE
        serender3d_render_set_material_uniforms_sprite(renderer, material, transform);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
    } else
        //- SKINNED MESH
    if (mesh->type == SE_MESH_TYPE_SKINNED) { // SKELETAL ANIMATION
        set_material_uniforms_skinned(renderer, material, transform, mesh->skeleton->final_pose);
    } else
    if (mesh->type == SE_MESH_TYPE_POINT) { // MESH MADE OUT OF POINTS
        //- POINT
        primitive = GL_POINTS;
        glPointSize(mesh->point_radius);
            // Note: points use the same shader as lines
        serender3d_render_set_material_uniforms_lines(renderer, transform);
    }

        //- Draw Call
    glBindVertexArray(mesh->vao);
    if (mesh->indexed) {
        glDrawElements(primitive, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->vert_count);
    }

    glBindVertexArray(0);
}

void se_render_mesh_with_shader
(SE_Renderer3D *renderer, SE_Mesh *mesh, Mat4 transform, SE_Shader *shader) {
    se_render3d_reset_render_config(); // Reset configs to their default values
    se_shader_use(shader);
    i32 primitive = GL_TRIANGLES;

        //- LINE
    if (mesh->type == SE_MESH_TYPE_LINE) { // LINE
        primitive = GL_LINES;
        glLineWidth(mesh->line_width);
    } else
        //- POINT
    if (mesh->type == SE_MESH_TYPE_POINT) { // MESH MADE OUT OF POINTS
        primitive = GL_POINTS;
        glPointSize(mesh->point_radius);
    }

        //- Draw Call
    glBindVertexArray(mesh->vao);
    if (mesh->indexed) {
        glDrawElements(primitive, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->vert_count);
    }

    glBindVertexArray(0);
}

// make sure to call serender3d_render_mesh_setup before calling this procedure. Only needs to be done once.
void se_render_mesh_index(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->user_meshes[mesh_index];
    se_render_mesh(renderer, mesh, transform);

    // if (mesh->next_mesh_index > -1 && mesh->type != SE_MESH_TYPE_SKINNED) { // @temp checking if it's not skinned because for some reason the other mesh does not get a proper skeleton final pose
    if (mesh->next_mesh_index > -1) {
        se_render_mesh_index(renderer, mesh->next_mesh_index, transform);
    }
}

void se_render3d_render_mesh_outline(SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->user_meshes[mesh_index];
    if (mesh->type == SE_MESH_TYPE_LINE || mesh->type == SE_MESH_TYPE_POINT) return;
    // take the mesh (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    { // setup the shader
        SE_Shader *shader = &renderer->shader_outline;
        se_shader_use(shader); // use the outline shader

        Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
        pvm = mat4_mul(pvm, renderer->current_camera->projection);

        se_shader_set_uniform_mat4(shader, "_pvm", pvm);
        se_shader_set_uniform_f32(shader, "_outline_width", 0.02f);
        se_shader_set_uniform_rgb(shader, "_outline_colour", (RGB) {255, 255, 255});
        static f32 time = 0;
        time += 0.167;
        se_shader_set_uniform_f32(shader, "_time", time);
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
}

void se_render_directional_shadow_map(SE_Renderer3D *renderer, u32 *mesh_indices, Mat4 *transforms, u32 count, AABB3D world_aabb) {
    se_assert(count <= renderer->user_meshes_count && "the number of transforms must be less than or equal to the number of meshes");
    SE_Light *light = &renderer->light_directional;
        // -- shadow mapping
    /* calculate the matrices */
    f32 left   = world_aabb.min.x;
    f32 right  = world_aabb.max.x;
    f32 bottom = world_aabb.min.y;
    f32 top    = world_aabb.max.y;
    f32 near   = world_aabb.min.z;
    f32 far    = world_aabb.max.z;
    // Vec3 light_pos = v3f(10,0,10);
    // light->calculated_position = v3f(
    //     -light->direction.x,
    //     -light->direction.y,
    //     // -renderer->light_directional.direction.z,
    //     0
    // );
    // light->calculated_position = vec3_mul_scalar(light->calculated_position, (far) * 0.5f);
    light->calculated_position = v3f(
        right * 0.99f * -light->direction.x,
        top   * 0.99f * -light->direction.y,
        far   * 0.99f * -light->direction.z
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
            SE_Mesh *mesh = renderer->user_meshes[mesh_index];


            recursive_render_directional_shadow_map_for_mesh(renderer, mesh_index, model_mat, light_space_mat);
        }
        glBindVertexArray(0);

        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE); // @remove after fixing whatever this is
    }

    serender_target_use(NULL);

    renderer->light_space_matrix = light_space_mat;
}

void se_render_omnidirectional_shadow_map(SE_Renderer3D *renderer, Mat4 *transforms, u32 count) {
    se_assert(count <= renderer->user_meshes_count && "the number of transforms must be less than or equal to the number of meshes");

    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        SE_Light_Point *point_light = &renderer->point_lights[i];
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

                // configure shader
                se_shader_use(&renderer->shader_shadow_omnidir_calc);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, point_light->depth_cube_map);
                se_shader_set_uniform_f32 (&renderer->shader_shadow_omnidir_calc, "far_plane", far);
                se_shader_set_uniform_vec3(&renderer->shader_shadow_omnidir_calc, "light_pos", point_light->position);
                se_shader_set_uniform_mat4(&renderer->shader_shadow_omnidir_calc, "shadow_matrices[0]", shadow_transforms[0]);
                se_shader_set_uniform_mat4(&renderer->shader_shadow_omnidir_calc, "shadow_matrices[1]", shadow_transforms[1]);
                se_shader_set_uniform_mat4(&renderer->shader_shadow_omnidir_calc, "shadow_matrices[2]", shadow_transforms[2]);
                se_shader_set_uniform_mat4(&renderer->shader_shadow_omnidir_calc, "shadow_matrices[3]", shadow_transforms[3]);
                se_shader_set_uniform_mat4(&renderer->shader_shadow_omnidir_calc, "shadow_matrices[4]", shadow_transforms[4]);
                se_shader_set_uniform_mat4(&renderer->shader_shadow_omnidir_calc, "shadow_matrices[5]", shadow_transforms[5]);
            }
            // render scene
            for (u32 i = 0; i < count; ++i) {
                SE_Mesh *mesh = renderer->user_meshes[i];
                Mat4 model_mat = transforms[i];

                se_shader_set_uniform_mat4(&renderer->shader_shadow_omnidir_calc, "model", model_mat);

                glBindVertexArray(mesh->vao);
                if (mesh->indexed) {
                    glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
                } else {
                    glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
                }
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

        //- SHADERS
    se_shader_init_from(&renderer->shader_lit,
        shader_filename_lit_vsd, shader_filename_lit_fsd);

    se_shader_init_from(&renderer->shader_shadow_calc,
        shader_filename_shadow_calc_directional_vsd,
        shader_filename_shadow_calc_directional_fsd);

    se_shader_init_from(&renderer->shader_shadow_calc_skinned_mesh,
        shader_filename_shadow_calc_directional_skinned_mesh_vsd,
        shader_filename_shadow_calc_directional_fsd);

    se_shader_init_from_with_geometry(&renderer->shader_shadow_omnidir_calc,
        shader_filename_shadow_calc_omnidir_vsd,
        shader_filename_shadow_calc_omnidir_fsd,
        shader_filename_shadow_calc_omnidir_gsd);

    se_shader_init_from(&renderer->shader_lines,
        shader_filename_lines_vsd,
        shader_filename_lines_fsd);

    se_shader_init_from(&renderer->shader_outline,
        shader_filename_outline_vsd,
        shader_filename_outline_fsd);

    se_shader_init_from(&renderer->shader_sprite,
        shader_filename_sprite_vsd,
        shader_filename_sprite_fsd);

    se_shader_init_from(&renderer->shader_skinned_mesh,
        shader_filename_lit_skinned_vsd,
        shader_filename_lit_fsd);

    se_shader_init_from(&renderer->shader_skinned_mesh_skeleton,
        shader_filename_skeleton_vsd,
        shader_filename_lines_fsd);

    se_shader_init_from(&renderer->shader_mouse_picking,
        shader_filename_mouse_picking_vsd,
        shader_filename_mouse_picking_fsd);

        //- MATERIALS
    //! We must have a default material at index zero.
    //! Because by default meshes point to the zero'th material.
    //! So we create one default material here and we refer to it.
    u32 default_material_index = se_render3d_add_material(renderer);
    se_assert(default_material_index == SE_DEFAULT_MATERIAL_INDEX && "The default material index that was created in the init() of renderer3D did not match what we expected");
    renderer->user_materials[default_material_index]->base_diffuse = (Vec4) {1, 1, 1, 1};
    se_texture_load(&renderer->user_materials[default_material_index]->texture_diffuse,
                    default_diffuse_filepath);
    se_texture_load(&renderer->user_materials[default_material_index]->texture_normal,
                    default_normal_filepath);
    se_texture_load(&renderer->user_materials[default_material_index]->texture_specular,
                    default_specular_filepath);

        //- SHADOW MAPPING
    f32 shadow_w = 1024;
    f32 shadow_h = 1024;
    serender_target_init(&renderer->shadow_render_target, (Rect) {0, 0, shadow_w, shadow_h}, true, true);

    {   // - POINT LIGHT SHADOW MAPPING
        for (u32 L = 0; L < SERENDERER3D_MAX_POINT_LIGHTS; ++L) {
            SE_Light_Point *point_light = &renderer->point_lights[L];
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
}

void se_render3d_deinit(SE_Renderer3D *renderer) {
        //- User meshes
    for (u32 i = 0; i < renderer->user_meshes_count; ++i) {
        se_mesh_deinit(renderer->user_meshes[i]);
    }
    renderer->user_meshes_count = 0;

    // for (u32 i = 0; i < renderer->shaders_count; ++i) {
    //     se_shader_deinit(renderer->shaders[i]);
    // }
    // renderer->shaders_count = 0;
        //- Default shaders
    se_shader_deinit(&renderer->shader_lit);
    se_shader_deinit(&renderer->shader_skinned_mesh);
    se_shader_deinit(&renderer->shader_skinned_mesh_skeleton);
    se_shader_deinit(&renderer->shader_shadow_calc);
    se_shader_deinit(&renderer->shader_shadow_calc_skinned_mesh);
    se_shader_deinit(&renderer->shader_shadow_omnidir_calc);
    se_shader_deinit(&renderer->shader_lines);
    se_shader_deinit(&renderer->shader_outline);
    se_shader_deinit(&renderer->shader_sprite);
    se_shader_deinit(&renderer->shader_mouse_picking);

        //- User materials
    for (u32 i = 0; i < renderer->user_materials_count; ++i) {
        se_material_deinit(renderer->user_materials[i]);
    }
    renderer->user_materials_count = 0;

        //- Shadow mapping
    serender_target_deinit(&renderer->shadow_render_target);
}

u32 se_render3d_add_point_light(SE_Renderer3D *renderer) {
    se_assert(renderer->point_lights_count < SERENDERER3D_MAX_POINT_LIGHTS);

    u32 result = renderer->point_lights_count;
    renderer->point_lights_count++;

        //- Default values
        // NOTE(Matin): We don't need to generate the cubmap here because all cubemaps
        // have already been generated for the maximum number of point lights
    renderer->point_lights[result].position  = v3f(0, 0, 0);
    renderer->point_lights[result].ambient   = (RGB) {100, 100, 100};
    renderer->point_lights[result].diffuse   = (RGB) {255, 255, 255};
    renderer->point_lights[result].specular  = (RGB) {0, 0, 0};
    renderer->point_lights[result].constant  = 1.0f;
    renderer->point_lights[result].linear    = 0.22f;
    renderer->point_lights[result].quadratic = 0.20f;

    return result;
}

u32 se_render3d_add_material(SE_Renderer3D *renderer) {
    renderer->user_materials[renderer->user_materials_count] = NEW(SE_Material);
    memset(renderer->user_materials[renderer->user_materials_count], 0, sizeof(SE_Material));
    u32 material_index = renderer->user_materials_count;
    renderer->user_materials_count++;
    return material_index;
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

u32 se_render3d_add_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width) {
    u32 result = renderer->user_meshes_count;

    renderer->user_meshes[renderer->user_meshes_count] = NEW(SE_Mesh);
    memset(renderer->user_meshes[renderer->user_meshes_count], 0, sizeof(SE_Mesh));
    se_mesh_generate_gizmos_aabb(renderer->user_meshes[renderer->user_meshes_count], min, max, line_width);

    renderer->user_meshes_count++;
    return result;
}

void se_render3d_update_gizmos_aabb(SE_Renderer3D *renderer, Vec3 min, Vec3 max, f32 line_width, u32 mesh_index) {

    memset(renderer->user_meshes[mesh_index], 0, sizeof(SE_Mesh));
    se_mesh_generate_gizmos_aabb(renderer->user_meshes[mesh_index], min, max, line_width);
}