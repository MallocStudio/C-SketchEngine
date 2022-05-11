#include "serenderer.h"
#include "stb_image.h"

#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "sestring.h"

#include "seinput.h" // for camera

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
        u8 mouse_state = SDL_GetMouseState(NULL, NULL);
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
/// TEXTURE
///

void setexture_load(SE_Texture *texture, const char *filepath) {
    texture->loaded = true;

    ubyte *image_data = stbi_load(filepath, &texture->width, &texture->height, &texture->channel_count, 0);
    if (image_data != NULL) {
        setexture_load_data(texture, image_data);
    } else {
        printf("ERROR: cannot load %s (%s)\n", filepath, stbi_failure_reason());
        texture->loaded = false;
    }
}

void setexture_load_data(SE_Texture *texture, ubyte *image_data) {
    glGenTextures(1, &texture->id);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    if (texture->channel_count == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    } else if (texture->channel_count == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    } else {
        printf("ERROR: cannot load texture, because we don't support %i channels\n", texture->channel_count);
        texture->loaded = false;
    }

    // @TODO
    // glGenerateMipmap(GL_TEXTURE_2D);

    // Instead of generating mipmaps we can set the texture param to not use mipmaps. We have to do one of these or our texture won't appear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image_data);
}

void setexture_unload(SE_Texture *texture) {
    if (texture->loaded) {
        glDeleteTextures(1, &texture->id);
    }
}

void setexture_bind(const SE_Texture *texture, u32 index) { // @TODO change index to an enum of different texture types that map to an index internally
    SDL_assert(texture->loaded == true && "texture was not loaded so we can't bind");
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void setexture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

///
/// TEXTURE ATLAS
///

void setexture_atlas_load(SE_Texture_Atlas *texture_atlas, const char *filepath, u32 columns, u32 rows) {
    setexture_load(&texture_atlas->texture, filepath);
    if (texture_atlas->texture.loaded) {
        texture_atlas->columns = columns;
        texture_atlas->rows = rows;
    }
}

void setexture_atlas_unload(SE_Texture_Atlas *texture_atlas) {
    setexture_unload(&texture_atlas->texture);
    texture_atlas->columns = 0;
    texture_atlas->rows = 0;
}

void setexture_atlas_bind(SE_Texture_Atlas *texture_atlas) {
    setexture_bind(&texture_atlas->texture, 0);
}

void setexture_atlas_unbind() {
    setexture_unbind();
}

///
/// MESH
///

void semesh_deinit(SE_Mesh *mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ibo);
    mesh->material_index = 0;
}

void semesh_generate_quad(SE_Mesh *mesh, Vec2 scale) {
    mesh->is_line = false;
    SE_Vertex3D verts[4];

    scale = vec2_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec3) {-scale.x, 0, +scale.y};
    verts[1].position = (Vec3) {+scale.x, 0, +scale.y};
    verts[2].position = (Vec3) {-scale.x, 0, -scale.y};
    verts[3].position = (Vec3) {+scale.x, 0, -scale.y};

    verts[0].normal = (Vec3) {0, 1, 0};
    verts[1].normal = (Vec3) {0, 1, 0};
    verts[2].normal = (Vec3) {0, 1, 0};
    verts[3].normal = (Vec3) {0, 1, 0};

    u32 indices[6] = {0, 1, 2, 2, 1, 3};
    semesh_generate(mesh, 4, verts, 6, indices);
}

void semesh_generate_cube(SE_Mesh *mesh, Vec3 scale) {
#if 1
    mesh->is_line = false;
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
#else
    mesh->is_line = false;
    SE_Vertex3D verts[24] = {0};
    scale = vec3_mul_scalar(scale, 0.5f);
    RGBA colour = RGBA_WHITE;

    verts[0+0].position = (Vec3) {-scale.x, -scale.y, +scale.z}; // negative y
    verts[1+0].position = (Vec3) {+scale.x, -scale.y, +scale.z};
    verts[2+0].position = (Vec3) {+scale.x, -scale.y, -scale.z};
    verts[3+0].position = (Vec3) {-scale.x, -scale.y, -scale.z};

    verts[0+0].normal = (Vec3) {0.0f, -1.0f, 0.0f};
    verts[1+0].normal = (Vec3) {0.0f, -1.0f, 0.0f};
    verts[2+0].normal = (Vec3) {0.0f, -1.0f, 0.0f};
    verts[3+0].normal = (Vec3) {0.0f, -1.0f, 0.0f};

    verts[0+0].texture_coord = (Vec2) {0, 0};
    verts[1+0].texture_coord = (Vec2) {0, 1};
    verts[2+0].texture_coord = (Vec2) {1, 1};
    verts[3+0].texture_coord = (Vec2) {1, 0};

    verts[0+1].position = (Vec3) {-scale.x, +scale.y, +scale.z}; // positive y
    verts[1+1].position = (Vec3) {-scale.x, +scale.y, -scale.z};
    verts[2+1].position = (Vec3) {+scale.x, +scale.y, -scale.z};
    verts[3+1].position = (Vec3) {+scale.x, +scale.y, +scale.z};

    verts[0+1].normal = (Vec3) {0.0f, +1.0f, 0.0f};
    verts[1+1].normal = (Vec3) {0.0f, +1.0f, 0.0f};
    verts[2+1].normal = (Vec3) {0.0f, +1.0f, 0.0f};
    verts[3+1].normal = (Vec3) {0.0f, +1.0f, 0.0f};

    verts[0+1].texture_coord = (Vec2) {0, 0};
    verts[1+1].texture_coord = (Vec2) {0, 1};
    verts[2+1].texture_coord = (Vec2) {1, 1};
    verts[3+1].texture_coord = (Vec2) {1, 0};

    verts[0+2].position = (Vec3) {+scale.x, -scale.y, +scale.z}; // positive x
    verts[1+2].position = (Vec3) {+scale.x, +scale.y, +scale.z};
    verts[2+2].position = (Vec3) {+scale.x, +scale.y, -scale.z};
    verts[3+2].position = (Vec3) {+scale.x, -scale.y, -scale.z};

    verts[0+2].normal = (Vec3) {+1.0f, 0.0f, 0.0f};
    verts[1+2].normal = (Vec3) {+1.0f, 0.0f, 0.0f};
    verts[2+2].normal = (Vec3) {+1.0f, 0.0f, 0.0f};
    verts[3+2].normal = (Vec3) {+1.0f, 0.0f, 0.0f};

    verts[0+2].texture_coord = (Vec2) {0, 0};
    verts[1+2].texture_coord = (Vec2) {0, 1};
    verts[2+2].texture_coord = (Vec2) {1, 1};
    verts[3+2].texture_coord = (Vec2) {1, 0};

    verts[0+3].position = (Vec3) {-scale.x, -scale.y, +scale.z}; // negative x
    verts[1+3].position = (Vec3) {-scale.x, -scale.y, -scale.z};
    verts[2+3].position = (Vec3) {-scale.x, +scale.y, -scale.z};
    verts[3+3].position = (Vec3) {-scale.x, +scale.y, +scale.z};

    verts[0+3].normal = (Vec3) {-1.0f, 0.0f, 0.0f};
    verts[1+3].normal = (Vec3) {-1.0f, 0.0f, 0.0f};
    verts[2+3].normal = (Vec3) {-1.0f, 0.0f, 0.0f};
    verts[3+3].normal = (Vec3) {-1.0f, 0.0f, 0.0f};

    verts[0+3].texture_coord = (Vec2) {0, 0};
    verts[1+3].texture_coord = (Vec2) {0, 1};
    verts[2+3].texture_coord = (Vec2) {1, 1};
    verts[3+3].texture_coord = (Vec2) {1, 0};

    verts[0+4].position = (Vec3) {-scale.x, -scale.y, +scale.z}; // positive z
    verts[1+4].position = (Vec3) {-scale.x, +scale.y, +scale.z};
    verts[2+4].position = (Vec3) {+scale.x, +scale.y, +scale.z};
    verts[3+4].position = (Vec3) {+scale.x, -scale.y, +scale.z};

    verts[0+4].normal = (Vec3) {0.0f, 0.0f, +1.0f};
    verts[1+4].normal = (Vec3) {0.0f, 0.0f, +1.0f};
    verts[2+4].normal = (Vec3) {0.0f, 0.0f, +1.0f};
    verts[3+4].normal = (Vec3) {0.0f, 0.0f, +1.0f};

    verts[0+4].texture_coord = (Vec2) {0, 0};
    verts[1+4].texture_coord = (Vec2) {0, 1};
    verts[2+4].texture_coord = (Vec2) {1, 1};
    verts[3+4].texture_coord = (Vec2) {1, 0};

    verts[0+5].position = (Vec3) {-scale.x, -scale.y, -scale.z}; // negative z
    verts[1+5].position = (Vec3) {+scale.x, -scale.y, -scale.z};
    verts[2+5].position = (Vec3) {-scale.x, +scale.y, -scale.z};
    verts[3+5].position = (Vec3) {+scale.x, +scale.y, -scale.z};

    verts[0+5].normal = (Vec3) {0.0f, 0.0f, -1.0f};
    verts[1+5].normal = (Vec3) {0.0f, 0.0f, -1.0f};
    verts[2+5].normal = (Vec3) {0.0f, 0.0f, -1.0f};
    verts[3+5].normal = (Vec3) {0.0f, 0.0f, -1.0f};

    verts[0+5].texture_coord = (Vec2) {0, 0};
    verts[1+5].texture_coord = (Vec2) {0, 1};
    verts[2+5].texture_coord = (Vec2) {1, 1};
    verts[3+5].texture_coord = (Vec2) {1, 0};

    u32 indices[36] = {
        0+0, 1+0, 2+0, // negative y
        2+0, 3+0, 0+0,
        0+1, 1+1, 2+1, // positive y
        2+1, 3+1, 0+1,
        0+2, 1+2, 2+2, // positive x
        2+2, 3+2, 0+2,
        0+3, 1+3, 2+3, // negative x
        2+3, 3+3, 0+3,
        0+4, 1+4, 2+4, // positive z
        2+4, 3+4, 0+4,
        0+5, 1+5, 2+5, // negative z
        2+5, 3+5, 0+5,
    };
    semesh_generate(mesh, 36, verts, 24, indices);
#endif
}

void semesh_generate_plane(SE_Mesh *mesh, Vec3 scale) {
    mesh->is_line = false;
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
    mesh->is_line = true;
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
    mesh->is_line = true;
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
    mesh->is_line = true;
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
    mesh->is_line = true;
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

void semesh_generate(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices, u32 index_count, u32 *indices) {
    // generate buffers
    glGenBuffers(1, &mesh->vbo);
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->ibo);

    // @note once we bind a VBO or IBO it "sticks" to the currently bound VAO, so we start by
    // binding VAO and then VBO.
    // This is so that later on, we'll just need to bind the vertex array object and not the buffers
    glBindVertexArray(mesh->vao); // start the macro
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Vertex3D) * vert_count, vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);

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

    mesh->vert_count = index_count;
    mesh->indexed = true;
    mesh->aabb = semesh_calc_aabb(vertices, vert_count);

    // unselect
    glBindVertexArray(0); // stop the macro
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // glDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);
    // glDisableVertexAttribArray(2);
    // glDisableVertexAttribArray(3);
}

///
/// RENDER 3D
///

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

static void semesh_construct
(SE_Renderer3D *renderer, SE_Mesh *mesh, const struct aiMesh *ai_mesh, const char *filepath, const struct aiScene *scene) {
    u32 verts_count = 0;
    u32 index_count = 0;
    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * ai_mesh->mNumVertices);
    u32       *indices = malloc(sizeof(u32) * ai_mesh->mNumFaces * 3);

    // -- vertices

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
        }

        verts[verts_count] = vertex;
        verts_count++;
    }

    // -- indices

    for (u32 i = 0; i < ai_mesh->mNumFaces; ++i) {
        // ! we triangulate on import, so every face has three vertices
        indices[index_count+0] = ai_mesh->mFaces[i].mIndices[0];
        indices[index_count+1] = ai_mesh->mFaces[i].mIndices[1];
        indices[index_count+2] = ai_mesh->mFaces[i].mIndices[2];
        index_count += 3;
    }

    semesh_generate(mesh, verts_count, verts, index_count, indices);


    if (scene->mNumMaterials > 0) { // -- materials
        // add a material to the renderer
        u32 material_index = serender3d_add_material(renderer);

        mesh->material_index = material_index;

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
        renderer->materials[material_index]->base_diffuse = (Vec4) {1, 1, 1, 1};
        if (has_diffuse) {
            sestring_append(&diffuse_path, ai_texture_path_diffuse->data);
            setexture_load(&renderer->materials[material_index]->texture_diffuse , diffuse_path.buffer);
        } else {
            setexture_load(&renderer->materials[material_index]->texture_diffuse, "assets/textures/checkerboard.png");
        }
        free(ai_texture_path_diffuse);
        /* specular */
        if (has_specular) {
            sestring_append(&specular_path, ai_texture_path_specular->data);
            setexture_load(&renderer->materials[material_index]->texture_specular, specular_path.buffer);
        }
        free(ai_texture_path_specular);
        /* normal */
        if (has_normal) {
            sestring_append(&normal_path, ai_texture_path_normal->data);
            setexture_load(&renderer->materials[material_index]->texture_normal  , normal_path.buffer);
        } else {
            setexture_load(&renderer->materials[material_index]->texture_diffuse, "assets/textures/default_normal.png");
        }
        free(ai_texture_path_normal);

        sestring_deinit(&diffuse_path);
        sestring_deinit(&specular_path);
        sestring_deinit(&normal_path);

        sestring_deinit(&dir);
    }
}

u32 serender3d_load_mesh(SE_Renderer3D *renderer, const char *model_filepath) {
    u32 result = -1;
    // load mesh from file
    const struct aiScene *scene = aiImportFile(model_filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (scene == NULL) {
        printf("ERROR: could not load load mesh from %s (%s)\n", model_filepath, aiGetErrorString());
        return result;
    }

    for (u32 i = 0; i < scene->mNumMeshes; ++i) {
        struct aiMesh *ai_mesh = scene->mMeshes[i];

        // add a mesh to the renderer
        renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
        memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));

        semesh_construct(renderer, renderer->meshes[renderer->meshes_count], ai_mesh, model_filepath, scene);
        result = renderer->meshes_count;
        renderer->meshes_count++;
    }
    return result;
}

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

// make sure to call serender3d_render_mesh_setup before calling this procedure. Only needs to be done once.
void serender3d_render_mesh(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->meshes[mesh_index];

    // take the mesh (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    i32 primitive = GL_TRIANGLES;
    if (mesh->is_line) {
        primitive = GL_LINES;
        glLineWidth(mesh->line_width);
        SE_Material *material = renderer->materials[renderer->material_lines];
        serender3d_render_set_material_uniforms_lines(renderer, material, transform);
    } else {
        SE_Material *material = renderer->materials[mesh->material_index];
        serender3d_render_set_material_uniforms_lit(renderer, material, transform);
    }

    glBindVertexArray(mesh->vao);

    if (mesh->indexed) {
        glDrawElements(primitive, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->vert_count);
    }

    if (mesh->is_line) {
        glLineWidth(1); // reset
    }

    glBindVertexArray(0);
}

void serender3d_render_mesh_outline(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->meshes[mesh_index];
    if (mesh->is_line) return;
    // take the mesh (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    i32 primitive = GL_TRIANGLES;

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
        glDrawElements(primitive, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, mesh->vert_count);
    }
    glCullFace(GL_BACK);

    if (mesh->is_line) {
        glLineWidth(1); // reset
    }

    glBindVertexArray(0);
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

    renderer->shader_lit = serender3d_add_shader(renderer, "shaders/lit.vsd", "shaders/lit_better.fsd");
    renderer->shader_shadow_calc = serender3d_add_shader(renderer, "shaders/shadow_calc.vsd", "shaders/shadow_calc.fsd");
    renderer->shader_shadow_omnidir_calc = serender3d_add_shader_with_geometry(renderer, "shaders/shadow_omni_calc.vsd", "shaders/shadow_omni_calc.fsd", "shaders/shadow_omni_calc.gsd");
    renderer->shader_lines = serender3d_add_shader(renderer, "shaders/lines.vsd", "shaders/lines.fsd");
    renderer->shader_outline = serender3d_add_shader(renderer, "shaders/outline.vsd", "shaders/outline.fsd");

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