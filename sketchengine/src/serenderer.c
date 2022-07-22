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
    mesh->skeleton = NULL; // because we don't own the skeleton
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
    mesh->element_count = index_count;
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
        mesh->element_count = index_count;
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
        mesh->element_count = index_count;
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

    mesh->element_count = index_count;
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

void se_skeleton_deinit(SE_Skeleton *skeleton) {
    for (u32 i = 0; i < skeleton->animations_count; ++i) {
        for (u32 j = 0; j < skeleton->animations[i]->animated_bones_count; ++j) {
            free(skeleton->animations[i]->animated_bones[j].positions);
            free(skeleton->animations[i]->animated_bones[j].rotations);
            free(skeleton->animations[i]->animated_bones[j].scales);
            free(skeleton->animations[i]->animated_bones[j].position_time_stamps);
            free(skeleton->animations[i]->animated_bones[j].rotation_time_stamps);
            free(skeleton->animations[i]->animated_bones[j].scale_time_stamps);
        }
        free(skeleton->animations[i]->animated_bones);
        free(skeleton->animations[i]);
    }

    skeleton->animations_count = 0;
    skeleton->bone_count = 0;
    skeleton->bone_node_count = 0;
}

void se_skeleton_calculate_pose
(SE_Skeleton *skeleton, f32 frame) {
    se_assert(skeleton->animations_count > 0);
    if (skeleton->animations_count > 0) {
        recursive_calculate_bone_pose(skeleton, skeleton->animations[skeleton->current_animation], frame, &skeleton->bone_nodes[0], mat4_identity());
    } else {
        recursive_calc_skeleton_pose_without_animation(skeleton);
    }
}

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

void se_save_data_mesh_deinit(SE_Save_Data_Meshes *save_data) {
    b8 is_skeleton_freed = false;

    for (u32 i = 0; i < save_data->meshes_count; ++i) {
        SE_Mesh_Raw_Data *raw_data = &save_data->meshes[i];

            //- Vertices
        if (raw_data->type == SE_MESH_TYPE_SKINNED) {
            free(raw_data->skinned_verts);
        } else {
            free(raw_data->verts);
        }
        raw_data->vert_count = 0;

            //- Indices
        free(raw_data->indices);
        raw_data->index_count = 0;

            //- Material
        se_string_deinit(&raw_data->texture_diffuse_filepath);
        se_string_deinit(&raw_data->texture_specular_filepath);
        se_string_deinit(&raw_data->texture_normal_filepath);

            //- Skeleton and animations
        if (raw_data->skeleton_data && !is_skeleton_freed) {
            is_skeleton_freed = true;
            se_skeleton_deinit(raw_data->skeleton_data);
            free(raw_data->skeleton_data);
        }
        raw_data->skeleton_data = NULL;
    }
    free(save_data->meshes);
    save_data->meshes_count = 0;
}

void se_save_data_read_mesh(SE_Save_Data_Meshes *save_data, const char *save_file) {
    FILE *file;
    file = fopen(save_file, "rb"); // read binary
        fread(&save_data->meshes_count, sizeof(u32), 1, file);
        save_data->meshes = malloc(sizeof(SE_Mesh_Raw_Data) * save_data->meshes_count);
        memset(save_data->meshes, 0, sizeof(SE_Mesh_Raw_Data) * save_data->meshes_count);

        for (u32 i = 0; i < save_data->meshes_count; ++i) {
            SE_Mesh_Raw_Data *raw_data = &save_data->meshes[i];
                //- Header
            fread(&raw_data->type, sizeof(SE_MESH_TYPES), 1, file);
                //- Verts
                // read how many verts are in the file
                // make space for the verts and load them from file
            fread(&raw_data->vert_count, sizeof(u32), 1, file);

            raw_data->skinned_verts = NULL;
            raw_data->verts = NULL;

            if (raw_data->type == SE_MESH_TYPE_SKINNED) {
                raw_data->skinned_verts = malloc(sizeof(SE_Skinned_Vertex) * raw_data->vert_count);
                fread(raw_data->skinned_verts, sizeof(SE_Skinned_Vertex), raw_data->vert_count, file);
            } else {
                raw_data->verts = malloc(sizeof(SE_Vertex3D) * raw_data->vert_count);
                fread(raw_data->verts, sizeof(SE_Vertex3D), raw_data->vert_count, file);
            }

            // make space for indices
            fread(&raw_data->index_count, sizeof(u32), 1, file);
            raw_data->indices = malloc(sizeof(u32) * raw_data->index_count);
            fread(raw_data->indices, sizeof(u32), raw_data->index_count, file);

                //- Shape
            fread(&raw_data->line_width, sizeof(f32), 1, file);
            fread(&raw_data->point_radius, sizeof(f32), 1, file);
            fread(&raw_data->is_indexed, sizeof(b8), 1, file);
            fread(&raw_data->aabb, sizeof(AABB3D), 1, file);
            fread(&raw_data->should_cast_shadow, sizeof(b8), 1, file);

                //- Material
            fread(&raw_data->base_diffuse, sizeof(f32), 4, file);
            u32 diffuse_buffer_size;
            u32 specular_buffer_size;
            u32 normal_buffer_size;
            fread(&diffuse_buffer_size, sizeof(u32), 1, file);
            fread(&specular_buffer_size, sizeof(u32), 1, file);
            fread(&normal_buffer_size, sizeof(u32), 1, file);

            if (diffuse_buffer_size > 0) {
                char diffuse_buffer[1024];
                fread(diffuse_buffer, sizeof(char), diffuse_buffer_size + 1, file);
                se_string_init(&raw_data->texture_diffuse_filepath, diffuse_buffer);
            }

            if (specular_buffer_size > 0) {
                char specular_buffer[1024];
                fread(specular_buffer, sizeof(char), specular_buffer_size + 1, file);
                se_string_init(&raw_data->texture_specular_filepath, specular_buffer);
            }

            if (normal_buffer_size > 0) {
                char normal_buffer[1024];
                fread(normal_buffer, sizeof(char), normal_buffer_size + 1, file);
                se_string_init(&raw_data->texture_normal_filepath, normal_buffer);
            }

                //- Skeleton
            if (raw_data->type == SE_MESH_TYPE_SKINNED) {
                raw_data->skeleton_data = malloc(sizeof(SE_Skeleton));
                memset(raw_data->skeleton_data, 0, sizeof(SE_Skeleton));
                read_skeleton_from_disk_binary(raw_data->skeleton_data, file);
            }
        }
    fclose(file);
}

void se_save_data_write_mesh(const SE_Save_Data_Meshes *save_data, const char *save_file) {
    FILE *file;
    file = fopen(save_file, "wb"); // write binary
        fwrite(&save_data->meshes_count, sizeof(u32), 1, file);
        for (u32 i = 0; i < save_data->meshes_count; ++i) {
            SE_Mesh_Raw_Data *raw_data = &save_data->meshes[i];
                //- Header
            fwrite(&raw_data->type, sizeof(SE_MESH_TYPES), 1, file);
                //- Verts
            fwrite(&raw_data->vert_count, sizeof(u32), 1, file);
            if (raw_data->type == SE_MESH_TYPE_SKINNED) {
                // then write skinned_verts data
                fwrite(raw_data->skinned_verts, sizeof(SE_Skinned_Vertex), raw_data->vert_count, file);
            } else {
                // write verts data
                // then write skinned_verts as null / what's already stored
                fwrite(raw_data->verts, sizeof(SE_Vertex3D), raw_data->vert_count, file);
            }

            fwrite(&raw_data->index_count, sizeof(u32), 1, file);
            fwrite(raw_data->indices, sizeof(u32), raw_data->index_count, file);

                //- Shape
            fwrite(&raw_data->line_width, sizeof(f32), 1, file);
            fwrite(&raw_data->point_radius, sizeof(f32), 1, file);
            fwrite(&raw_data->is_indexed, sizeof(b8), 1, file);
            fwrite(&raw_data->aabb, sizeof(AABB3D), 1, file);
            fwrite(&raw_data->should_cast_shadow, sizeof(b8), 1, file);

                //- Material
            fwrite(&raw_data->base_diffuse, sizeof(f32), 4, file);
            fwrite(&raw_data->texture_diffuse_filepath.size, sizeof(u32), 1, file);
            fwrite(&raw_data->texture_specular_filepath.size, sizeof(u32), 1, file);
            fwrite(&raw_data->texture_normal_filepath.size, sizeof(u32), 1, file);

            if (raw_data->texture_diffuse_filepath.size > 0) {
                fwrite(raw_data->texture_diffuse_filepath.buffer,
                        sizeof(char), raw_data->texture_diffuse_filepath.size + 1, file);
            }

            if (raw_data->texture_specular_filepath.size > 0) {
                fwrite(raw_data->texture_specular_filepath.buffer,
                        sizeof(char), raw_data->texture_specular_filepath.size + 1, file);
            }

            if (raw_data->texture_normal_filepath.size > 0) {
                fwrite(raw_data->texture_normal_filepath.buffer,
                        sizeof(char), raw_data->texture_normal_filepath.size + 1, file);
            }

                //- Skeleton
            if (raw_data->type == SE_MESH_TYPE_SKINNED && raw_data->skeleton_data) {
                write_skeleton_to_disk_binary(raw_data->skeleton_data, file);
            }
        }
    fclose(file);
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
        set_material_uniforms_lit(renderer, material, transform);
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
        glDrawElements(primitive, mesh->element_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->element_count);
    }

    glBindVertexArray(0);
}

void se_render_mesh_with_lit_shader
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
        glDrawElements(primitive, mesh->element_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->element_count);
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
        glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->element_count);
    }
    glCullFace(GL_BACK);

    glBindVertexArray(0);
}

void se_render_post_process(SE_Renderer3D *renderer, SE_RENDER_POSTPROCESS post_process, const SE_Render_Target *previous_render_pass) {
    SE_Shader *shader;
    switch (post_process) {
        case SE_RENDER_POSTPROCESS_TONEMAP: {
            shader = &renderer->shader_post_process_tonemap;
        } break;
        case SE_RENDER_POSTPROCESS_BLUR: {
            shader = &renderer->shader_post_process_blur;
            se_shader_use(shader);
            se_shader_set_uniform_i32(shader, "texture_to_blur", 1); // get the bright colour channel from lit_footer.fsd
        } break;
        case SE_RENDER_POSTPROCESS_DOWNSAMPLE: {
            shader = &renderer->shader_post_process_downsample;
            se_shader_use(shader);
            se_shader_set_uniform_vec2(shader, "src_resolution",
                v2f(renderer->viewport.w, renderer->viewport.h));

        } break;
        case SE_RENDER_POSTPROCESS_UPSAMPLE: {
            shader = &renderer->shader_post_process_upsample;
            se_shader_use(shader);
            se_shader_set_uniform_f32(shader, "src_resolution", 3.0f);
        } break;
        case SE_RENDER_POSTPROCESS_BLOOM: {
            shader = &renderer->shader_post_process_bloom;
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
    SE_Shader *shader = &renderer->shader_post_process_gaussian_blur;

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

    se_shader_init_from_files(&renderer->shader_lit,
                                lit_vertex_files, 2,
                                lit_fragment_files, 2,
                                NULL, 0);

    se_shader_init_from_files(&renderer->shader_shadow_calc,
                            shadow_calc_directional_vsd_files, 1,
                            shadow_calc_directional_fsd_files, 1,
                            NULL, 0);

    se_shader_init_from_files(&renderer->shader_shadow_calc_skinned_mesh,
        shadow_calc_directional_skinned_vsd_files, 1,
        shadow_calc_directional_fsd_files, 1,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_shadow_omnidir_calc,
        shadow_calc_omnidir_vsd_files, 1,
        shadow_calc_omnidir_fsd_files, 1,
        shadow_calc_omnidir_gsd_files, 1);

    se_shader_init_from_files(&renderer->shader_shadow_omnidir_calc_skinned_mesh,
                            shadow_calc_omnidir_skinned_vsd_files, 1,
                            shadow_calc_omnidir_fsd_files, 1,
                            shadow_calc_omnidir_gsd_files, 1);

    se_shader_init_from_files(&renderer->shader_lines,
        lines_vsd_files, 1,
        lines_fsd_files, 1,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_outline,
        outline_vsd_files, 1,
        outline_fsd_files, 1,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_sprite,
        sprite_vsd_files, 1,
        sprite_fsd_files, 1,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_skinned_mesh,
            skinned_vertex_files, 1,
            lit_fragment_files, 2,
            NULL, 0);

    se_shader_init_from_files(&renderer->shader_skinned_mesh_skeleton,
        skeleton_vsd_files, 1,
        lines_fsd_files, 1,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_post_process_tonemap,
        post_process_vsd, 1,
        post_process_tonemap, 2,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_post_process_blur,
        post_process_vsd, 1,
        post_process_blur, 2,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_post_process_downsample,
        post_process_vsd, 1,
        post_process_downsample, 2,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_post_process_upsample,
        post_process_vsd, 1,
        post_process_upsample, 2,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_post_process_bloom,
        post_process_vsd, 1,
        post_process_bloom, 2,
        NULL, 0);

    se_shader_init_from_files(&renderer->shader_post_process_gaussian_blur,
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
    se_shader_deinit(&renderer->shader_shadow_omnidir_calc_skinned_mesh);

        //- Post Process shaders
    se_shader_deinit(&renderer->shader_post_process_tonemap);
    se_shader_deinit(&renderer->shader_post_process_blur);
    se_shader_deinit(&renderer->shader_post_process_gaussian_blur);
    se_shader_deinit(&renderer->shader_post_process_downsample);
    se_shader_deinit(&renderer->shader_post_process_upsample);
    se_shader_deinit(&renderer->shader_post_process_bloom);

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