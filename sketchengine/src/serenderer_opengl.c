#include "serenderer_opengl.h"
#include <stdio.h> // for loading file as string
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
    cam->projection = mat4_perspective(SEMATH_PI * 0.25f,
                                        window_w / (f32) window_h,
                                        0.1f, 1000.0f);
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

        f32 camera_speed = 0.05f; // adjust accordingly

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
            f32 yoffset = seinput->mouse_screen_pos_delta.y * sensitivity * -1;

            if (semath_abs(xoffset) > sensitivity) camera->yaw += xoffset;
            if (semath_abs(yoffset) > sensitivity) camera->pitch += yoffset;
            if(camera->pitch > +89.0f) camera->pitch = +89.0f;
            if(camera->pitch < -89.0f) camera->pitch = -89.0f;
        }
    }
}

void seshader_init_from(SE_Shader *sp, const char *vertex_filename, const char *fragment_filename) {
    sp->loaded_successfully = true; // set to false later on if errors occure

    sp->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    sp->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    sp->shader_program = glCreateProgram();

    char *vertex_src = se_load_file_as_string(vertex_filename);
    char *frag_src = se_load_file_as_string(fragment_filename);

    GLchar error_log[512];
    GLint success = 0;

    glShaderSource(sp->vertex_shader, 1, &vertex_src, NULL);
    glCompileShader(sp->vertex_shader);

    glGetShaderiv(sp->vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // something failed with the vertex shader compilation
        printf ("vertex shader %s failed with error:\n", vertex_filename);
        glGetShaderInfoLog(sp->vertex_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", vertex_filename);
        // printf("%s\n", vertex_src);
    }

    glShaderSource(sp->fragment_shader, 1, &frag_src, NULL);
    glCompileShader(sp->fragment_shader);

    glGetShaderiv(sp->fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        printf ("fragment shader %s failed with error:\n", fragment_filename);
        glGetShaderInfoLog(sp->fragment_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", fragment_filename);
        // printf("%s\n", frag_src);
    }

    glAttachShader(sp->shader_program, sp->vertex_shader);
    glAttachShader(sp->shader_program, sp->fragment_shader);
    glLinkProgram(sp->shader_program);
    glGetProgramiv(sp->shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        printf ("Error linking shaders \\%s\\ and \\%s\\\n", vertex_filename, fragment_filename);
        glGetProgramInfoLog(sp->shader_program, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    }

    if (sp->loaded_successfully) {
        printf ("Shaders compiled and linked successfully.\n");
    } else {
        // if there was a problem, tell OpenGL that we don't need those resources after all
        glDeleteShader(sp->vertex_shader);
        glDeleteShader(sp->fragment_shader);
        glDeleteProgram(sp->shader_program);
    }

    free(vertex_src);
    free(frag_src);
}

void seshader_deinit(SE_Shader *shader) {
    if (shader->loaded_successfully) {
        glDeleteShader(shader->vertex_shader);
        glDeleteShader(shader->fragment_shader);
        glDeleteProgram(shader->shader_program);
    }
}

void seshader_use(const SE_Shader *shader) {
    glUseProgram(shader->shader_program);
}

GLuint seshader_get_uniform_loc(SE_Shader *shader, const char *uniform_name) {
    return glGetUniformLocation(shader->shader_program, uniform_name);
}

void seshader_set_uniform_f32  (SE_Shader *shader, const char *uniform_name, f32 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    // if ((i32)var_loc == -1) printf("Something's boned (%s)\n", uniform_name);
    seshader_use(shader);
    glUniform1f(var_loc, value);
}

void seshader_set_uniform_i32  (SE_Shader *shader, const char *uniform_name, i32 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    // if ((i32)var_loc == -1) printf("Something's boned (%s)\n", uniform_name);
    seshader_use(shader);
    glUniform1i(var_loc, value);
}

void seshader_set_uniform_vec3 (SE_Shader *shader, const char *uniform_name, Vec3 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniform3f(var_loc, value.x, value.y, value.z);
}

void seshader_set_uniform_vec2 (SE_Shader *shader, const char *uniform_name, Vec2 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniform2f(var_loc, value.x, value.y);
}

void seshader_set_uniform_rgb (SE_Shader *shader, const char *uniform_name, RGB value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    rgb_normalise(&value);
    glUniform3f(var_loc, value.r, value.g, value.b);
}

void seshader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, Mat4 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniformMatrix4fv(var_loc, 1, GL_FALSE, (const GLfloat*)&value);
}

char* se_load_file_as_string(const char *file_name) {
    // https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c
    char *source = NULL;
    FILE *fp = fopen(file_name, "r");
    if (fp != NULL) {
        // go to the end of the file
        if (fseek(fp, 0L, SEEK_END) == 0) {
            // get the size of the file
            long bufsize = ftell(fp);
            if (bufsize == -1) { // error
                printf("file reading error at %s: %i\n", __FILE__,__LINE__);
                return NULL;
            }
            // allocate our buffer to that size
            source = malloc(sizeof(char) * (bufsize + 1));

            // Go back to the start of the file
            if (fseek(fp, 0L, SEEK_SET) != 0) { // error
                printf("file reading error at %s: %i\n", __FILE__,__LINE__);
                return NULL;
            }

            // read the entire file into memory
            size_t new_len = fread(source, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0) {
                fputs("Errpr reading file", stderr);
            } else {
                source[new_len++] = '\0'; // just to be safe
            }
        }
        fclose(fp);
    } else {
        printf("file reading error at %s: %i\n", __FILE__,__LINE__);
    }
    return source;
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
    // @TODO add proper error handling

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
    SE_Vertex3D verts[8] = {0};

    scale = vec3_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec3) {+scale.x, +scale.y, +scale.z};
    verts[1].position = (Vec3) {-scale.x, +scale.y, +scale.z};
    verts[2].position = (Vec3) {-scale.x, -scale.y, +scale.z};
    verts[3].position = (Vec3) {+scale.x, -scale.y, +scale.z};
    verts[4].position = (Vec3) {+scale.x, -scale.y, -scale.z};
    verts[5].position = (Vec3) {+scale.x, +scale.y, -scale.z};
    verts[6].position = (Vec3) {-scale.x, +scale.y, -scale.z};
    verts[7].position = (Vec3) {-scale.x, -scale.y, -scale.z};

    verts[0].rgba = RGBA_WHITE;
    verts[1].rgba = RGBA_WHITE;
    verts[2].rgba = RGBA_WHITE;
    verts[3].rgba = RGBA_WHITE;
    verts[4].rgba = RGBA_WHITE;
    verts[5].rgba = RGBA_WHITE;
    verts[6].rgba = RGBA_WHITE;
    verts[7].rgba = RGBA_WHITE;

    // @TODO generate better normals
    verts[0].normal = (Vec3) {+0.5f, +0.5f, +0.5f};
    verts[1].normal = (Vec3) {-0.5f, +0.5f, +0.5f};
    verts[2].normal = (Vec3) {-0.5f, -0.5f, +0.5f};
    verts[3].normal = (Vec3) {+0.5f, -0.5f, +0.5f};
    verts[4].normal = (Vec3) {+0.5f, -0.5f, -0.5f};
    verts[5].normal = (Vec3) {+0.5f, +0.5f, -0.5f};
    verts[6].normal = (Vec3) {-0.5f, +0.5f, -0.5f};
    verts[7].normal = (Vec3) {-0.5f, -0.5f, -0.5f};

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
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
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

        // @incomplete use these procedure fully
        aiGetMaterialTexture(ai_material, aiTextureType_DIFFUSE , 0, ai_texture_path_diffuse, NULL, NULL, NULL, NULL, NULL, NULL);
        aiGetMaterialTexture(ai_material, aiTextureType_SPECULAR, 0, ai_texture_path_specular, NULL, NULL, NULL, NULL, NULL, NULL);
        aiGetMaterialTexture(ai_material, aiTextureType_NORMALS , 0, ai_texture_path_normal, NULL, NULL, NULL, NULL, NULL, NULL);

        sestring_append(&diffuse_path, ai_texture_path_diffuse->data);
        sestring_append(&specular_path, ai_texture_path_specular->data);
        sestring_append(&normal_path, ai_texture_path_normal->data);

        setexture_load(&renderer->materials[material_index]->texture_diffuse , diffuse_path.buffer);
        setexture_load(&renderer->materials[material_index]->texture_specular, specular_path.buffer);
        setexture_load(&renderer->materials[material_index]->texture_normal  , normal_path.buffer);

        free(ai_texture_path_diffuse);
        free(ai_texture_path_specular);
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
        // @remove once entities can render their own meshes
        // renderer->meshes[renderer->meshes_count]->transform = (Mat4) {
        //     1, 0, 0, 0,
        //     0, 1, 0, 0,
        //     0, 0, 1, 0,
        //     0, 0, 0, 1,
        // };
        result = renderer->meshes_count;
        renderer->meshes_count++;
    }
    return result;
}

void serender3d_render_mesh(const SE_Renderer3D *renderer, u32 mesh_index, Mat4 transform) {
    SE_Mesh *mesh = renderer->meshes[mesh_index];
    SE_Material *material = renderer->materials[mesh->material_index];

    // take the quad (world space) and project it to view space
    // then take that and project it to the clip space
    // then pass that final projection matrix and give it to the shader

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    seshader_use(renderer->shaders[0]); // use the default shader

    // the good old days when debugging:
    // material->texture_diffuse.width = 100;

    seshader_set_uniform_mat4(renderer->shaders[0], "projection_view_model", pvm);
    seshader_set_uniform_mat4(renderer->shaders[0], "model_matrix", transform);
    seshader_set_uniform_vec3(renderer->shaders[0], "camera_pos", renderer->current_camera->position);

    seshader_set_uniform_f32(renderer->shaders[0], "specular_power", 0.5f);

    seshader_set_uniform_i32(renderer->shaders[0], "texture_diffuse", 0);
    seshader_set_uniform_i32(renderer->shaders[0], "texture_specular", 1);
    seshader_set_uniform_i32(renderer->shaders[0], "texture_normal", 2);

    // light uniforms
    seshader_set_uniform_vec3(renderer->shaders[0], "L", renderer->light_directional.direction);
    seshader_set_uniform_rgb(renderer->shaders[0], "iA", renderer->light_directional.ambient);
    seshader_set_uniform_rgb(renderer->shaders[0], "iD", renderer->light_directional.diffuse);

    setexture_bind(&material->texture_diffuse, 0);
    setexture_bind(&material->texture_specular, 1);
    setexture_bind(&material->texture_normal, 2);

    glBindVertexArray(mesh->vao);

    if (mesh->indexed) {
        glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
    }

    glBindVertexArray(0);
}

void serender3d_add_shader(SE_Renderer3D *renderer, const char *vsd, const char *fsd) {
    // add a default shader
    renderer->shaders[renderer->shaders_count] = new (SE_Shader);
    seshader_init_from(renderer->shaders[renderer->shaders_count], vsd, fsd);
    renderer->shaders_count++;
}

void serender3d_init(SE_Renderer3D *renderer, SE_Camera3D *current_camera, const char *vsd, const char *fsd) {
    memset(renderer, 0, sizeof(SE_Renderer3D));
    renderer->current_camera = current_camera;

    serender3d_add_shader(renderer, vsd, fsd);
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
}

u32 serender3d_add_cube(SE_Renderer3D *renderer) {
    u32 result = renderer->meshes_count;

    renderer->meshes[renderer->meshes_count] = new(SE_Mesh);
    memset(renderer->meshes[renderer->meshes_count], 0, sizeof(SE_Mesh));
    semesh_generate_cube(renderer->meshes[renderer->meshes_count], vec3_one());

    renderer->meshes_count++;
    return result;
}