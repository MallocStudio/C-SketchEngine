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
        const u8 *keyboard = seinput->keyboard;
        i32 r = keyboard[SDL_SCANCODE_D] == true       ? 1 : 0;
        i32 l = keyboard[SDL_SCANCODE_A] == true       ? 1 : 0;
        i32 d = keyboard[SDL_SCANCODE_S] == true       ? 1 : 0;
        i32 u = keyboard[SDL_SCANCODE_W] == true       ? 1 : 0;
        i32 elevate = keyboard[SDL_SCANCODE_E] == true ? 1 : 0;
        i32 dive = keyboard[SDL_SCANCODE_Q] == true    ? 1 : 0;

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
            f32 sensitivity = 0.1f;
            f32 xoffset = seinput->mouse_screen_pos_delta.x * sensitivity;
            f32 yoffset = seinput->mouse_screen_pos_delta.y * sensitivity;

            if (semath_abs(xoffset) > sensitivity) camera->yaw += xoffset;
            if (semath_abs(yoffset) > sensitivity) camera->pitch += yoffset;
            if(camera->pitch > +89.0f) camera->pitch = +89.0f;
            if(camera->pitch < -89.0f) camera->pitch = -89.0f;
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
    SE_Vertex3D verts[4];

    scale = vec2_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec3) {-scale.x, 0, +scale.y};
    verts[1].position = (Vec3) {+scale.x, 0, +scale.y};
    verts[2].position = (Vec3) {-scale.x, 0, -scale.y};
    verts[3].position = (Vec3) {+scale.x, 0, -scale.y};

    verts[0].rgba = (RGBA) {255, 255, 255, 255};
    verts[1].rgba = (RGBA) {255, 255, 255, 255};
    verts[2].rgba = (RGBA) {255, 255, 255, 255};
    verts[3].rgba = (RGBA) {255, 255, 255, 255};

    verts[0].normal = (Vec3) {0, 1, 0};
    verts[1].normal = (Vec3) {0, 1, 0};
    verts[2].normal = (Vec3) {0, 1, 0};
    verts[3].normal = (Vec3) {0, 1, 0};

    u32 indices[6] = {0, 1, 2, 2, 1, 3};
    semesh_generate(mesh, 4, verts, 6, indices);
}

void semesh_generate_cube(SE_Mesh *mesh, Vec3 scale) {
#if 1
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

    verts[0].rgba = RGBA_WHITE;
    verts[1].rgba = RGBA_WHITE;
    verts[2].rgba = RGBA_WHITE;
    verts[3].rgba = RGBA_WHITE;
    verts[4].rgba = RGBA_WHITE;
    verts[5].rgba = RGBA_WHITE;
    verts[6].rgba = RGBA_WHITE;
    verts[7].rgba = RGBA_WHITE;

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
    SE_Vertex3D verts[24] = {0};
    scale = vec3_mul_scalar(scale, 0.5f);
    RGBA colour = RGBA_WHITE;

    verts[0+0].position = (Vec3) {-scale.x, -scale.y, +scale.z}; // negative y
    verts[1+0].position = (Vec3) {+scale.x, -scale.y, +scale.z};
    verts[2+0].position = (Vec3) {+scale.x, -scale.y, -scale.z};
    verts[3+0].position = (Vec3) {-scale.x, -scale.y, -scale.z};

    verts[0+0].rgba = colour;
    verts[1+0].rgba = colour;
    verts[2+0].rgba = colour;
    verts[3+0].rgba = colour;

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

    verts[0+1].rgba = colour;
    verts[1+1].rgba = colour;
    verts[2+1].rgba = colour;
    verts[3+1].rgba = colour;

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

    verts[0+2].rgba = colour;
    verts[1+2].rgba = colour;
    verts[2+2].rgba = colour;
    verts[3+2].rgba = colour;

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

    verts[0+3].rgba = colour;
    verts[1+3].rgba = colour;
    verts[2+3].rgba = colour;
    verts[3+3].rgba = colour;

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

    verts[0+4].rgba = colour;
    verts[1+4].rgba = colour;
    verts[2+4].rgba = colour;
    verts[3+4].rgba = colour;

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

    verts[0+5].rgba = colour;
    verts[1+5].rgba = colour;
    verts[2+5].rgba = colour;
    verts[3+5].rgba = colour;

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
    SE_Vertex3D verts[4] = {0};

    scale = vec3_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec3) {-scale.x, 0.0f, -scale.z};
    verts[1].position = (Vec3) {-scale.x, 0.0f, +scale.z};
    verts[2].position = (Vec3) {+scale.x, 0.0f, +scale.z};
    verts[3].position = (Vec3) {+scale.x, 0.0f, -scale.z};

    verts[0].rgba = RGBA_WHITE;
    verts[1].rgba = RGBA_WHITE;
    verts[2].rgba = RGBA_WHITE;
    verts[3].rgba = RGBA_WHITE;

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
    // -- enable color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, TYPEOF_RGBA_OPENGL, GL_TRUE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, rgba));
    // -- enable normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, normal));
    // -- enable uv
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, texture_coord));
    // -- enable tangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, tangent));
    // -- enable bitangent
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, bitangent));

    mesh->vert_count = index_count;
    mesh->indexed = true;

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

        // -- colour
        vertex.rgba = RGBA_WHITE;

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
        renderer->materials[renderer->materials_count] = new(SE_Material);
        memset(renderer->materials[renderer->materials_count], 0, sizeof(SE_Material));
        u32 material_index = renderer->materials_count;
        renderer->materials_count++;

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

        // @incomplete use these procedure fully
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
        if (has_diffuse) {
            sestring_append(&diffuse_path, ai_texture_path_diffuse->data);
            setexture_load(&renderer->materials[material_index]->texture_diffuse , diffuse_path.buffer);
            free(ai_texture_path_diffuse);
        } else {
            renderer->materials[material_index]->base_diffuse = (Vec4) {255, 255, 255, 255};
        }
        /* specular */
        if (has_specular) {
            sestring_append(&specular_path, ai_texture_path_specular->data);
            setexture_load(&renderer->materials[material_index]->texture_specular, specular_path.buffer);
            free(ai_texture_path_specular);
        }
        /* normal */
        if (has_normal) {
            sestring_append(&normal_path, ai_texture_path_normal->data);
            setexture_load(&renderer->materials[material_index]->texture_normal  , normal_path.buffer);
            free(ai_texture_path_normal);
        }

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

// make sure to call serender3d_render_mesh_setup before calling this procedure. Only needs to be done once.
void serender3d_render_mesh(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->meshes[mesh_index];
    SE_Material *material = renderer->materials[mesh->material_index];

    // take the quad (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    u32 shader = renderer->shader_lit;
    seshader_use(renderer->shaders[shader]); // use the default shader

    // the good old days when debugging:
    // material->texture_diffuse.width = 100;

    seshader_set_uniform_mat4(renderer->shaders[shader], "light_space_matrix", renderer->light_space_matrix);
    seshader_set_uniform_mat4(renderer->shaders[shader], "projection_view_model", pvm);
    seshader_set_uniform_mat4(renderer->shaders[shader], "model_matrix", transform);
    seshader_set_uniform_vec3(renderer->shaders[shader], "camera_pos", renderer->current_camera->position);

    /* material uniforms */
    seshader_set_uniform_f32 (renderer->shaders[shader], "specular_power", 0.1f);
    seshader_set_uniform_i32 (renderer->shaders[shader], "texture_diffuse", 0);
    seshader_set_uniform_i32 (renderer->shaders[shader], "texture_specular", 1);
    seshader_set_uniform_i32 (renderer->shaders[shader], "texture_normal", 2);
    seshader_set_uniform_i32 (renderer->shaders[shader], "shadow_map", 3);
    seshader_set_uniform_vec4(renderer->shaders[shader], "base_diffuse", material->base_diffuse);

    // light uniforms
    seshader_set_uniform_vec3(renderer->shaders[shader], "L", renderer->light_directional.direction);
    seshader_set_uniform_rgb(renderer->shaders[shader], "iA", renderer->light_directional.ambient);
    seshader_set_uniform_rgb(renderer->shaders[shader], "iD", renderer->light_directional.diffuse);

    setexture_bind(&material->texture_diffuse, 0);
    setexture_bind(&material->texture_specular, 1);
    setexture_bind(&material->texture_normal, 2);

    glActiveTexture(GL_TEXTURE0 + 3); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.texture);

    glBindVertexArray(mesh->vao);

    if (mesh->indexed) {
        glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
    }

    glBindVertexArray(0);
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

    renderer->shader_lit = serender3d_add_shader(renderer, "shaders/lit.vsd", "shaders/lit.fsd");
    renderer->shader_shadow_calc = serender3d_add_shader(renderer, "shaders/shadow_calc.vsd", "shaders/shadow_calc.fsd");
    renderer->shader_shadow_debug_render = serender3d_add_shader(renderer, "shaders/shadow_debug_render.vsd", "shaders/shadow_debug_render.fsd");

    /* shadow mapping */
    f32 shadow_w = 1024;
    f32 shadow_h = 1024;
    serender_target_init(&renderer->shadow_render_target, (Rect) {0, 0, shadow_w, shadow_h}, true);
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
