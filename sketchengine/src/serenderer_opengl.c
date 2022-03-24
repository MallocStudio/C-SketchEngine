#include "serenderer_opengl.h"
#include <stdio.h> // for loading file as string

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

seshader_deinit(SE_Shader *shader) {
    if (shader->loaded_successfully) {
        glDeleteShader(shader->vertex_shader);
        glDeleteShader(shader->fragment_shader);
        glDeleteProgram(shader->shader_program);
    }
}

seshader_use(SE_Shader *shader) {
    glUseProgram(shader->shader_program);
}

GLuint seshader_get_uniform_loc(SE_Shader *shader, const char *uniform_name) {
    return glGetUniformLocation(shader->shader_program, uniform_name);
}

void seshader_set_uniform_f32  (SE_Shader *shader, const char *uniform_name, f32 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniform1f(var_loc, value);
}

void seshader_set_uniform_vec3 (SE_Shader *shader, const char *uniform_name, Vec3 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniform3f(var_loc, value.x, value.y, value.z);
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
/// MESH
///

void semesh_deinit(SE_Mesh *mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ibo);
}

void semesh_generate_quad(SE_Mesh *mesh, Vec2 scale) {
    SE_Vertex3D verts[4];

    scale = vec2_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec4) {-scale.x, 0, +scale.y, 1};
    verts[1].position = (Vec4) {+scale.x, 0, +scale.y, 1};
    verts[2].position = (Vec4) {-scale.x, 0, -scale.y, 1};
    verts[3].position = (Vec4) {+scale.x, 0, -scale.y, 1};

    verts[0].rgba = (RGBA) {255, 255, 255, 255};
    verts[1].rgba = (RGBA) {255, 255, 255, 255};
    verts[2].rgba = (RGBA) {255, 255, 255, 255};
    verts[3].rgba = (RGBA) {255, 255, 255, 255};

    verts[0].normal = (Vec4) {0, 1, 0, 1};
    verts[1].normal = (Vec4) {0, 1, 0, 1};
    verts[2].normal = (Vec4) {0, 1, 0, 1};
    verts[3].normal = (Vec4) {0, 1, 0, 1};

    u32 indices[6] = {0, 1, 2, 2, 1, 3};
    semesh_generate(mesh, 4, verts, 6, indices);
}

void semesh_generate_cube(SE_Mesh *mesh, Vec3 scale) {
    SE_Vertex3D verts[8];

    scale = vec3_mul_scalar(scale, 0.5f);
    verts[0].position = (Vec4) {+scale.x, +scale.y, +scale.z, 1};
    verts[1].position = (Vec4) {-scale.x, +scale.y, +scale.z, 1};
    verts[2].position = (Vec4) {-scale.x, -scale.y, +scale.z, 1};
    verts[3].position = (Vec4) {+scale.x, -scale.y, +scale.z, 1};
    verts[4].position = (Vec4) {+scale.x, -scale.y, -scale.z, 1};
    verts[5].position = (Vec4) {+scale.x, +scale.y, -scale.z, 1};
    verts[6].position = (Vec4) {-scale.x, +scale.y, -scale.z, 1};
    verts[7].position = (Vec4) {-scale.x, -scale.y, -scale.z, 1};

    verts[0].rgba = RGBA_WHITE;
    verts[1].rgba = RGBA_WHITE;
    verts[2].rgba = RGBA_WHITE;
    verts[3].rgba = RGBA_WHITE;
    verts[4].rgba = RGBA_WHITE;
    verts[5].rgba = RGBA_WHITE;
    verts[6].rgba = RGBA_WHITE;
    verts[7].rgba = RGBA_WHITE;

    // @TODO generate better normals
    verts[0].normal = (Vec4) {+0.5f, +0.5f, +0.5f, 1};
    verts[1].normal = (Vec4) {-0.5f, +0.5f, +0.5f, 1};
    verts[2].normal = (Vec4) {-0.5f, -0.5f, +0.5f, 1};
    verts[3].normal = (Vec4) {+0.5f, -0.5f, +0.5f, 1};
    verts[4].normal = (Vec4) {+0.5f, -0.5f, -0.5f, 1};
    verts[5].normal = (Vec4) {+0.5f, +0.5f, -0.5f, 1};
    verts[6].normal = (Vec4) {-0.5f, +0.5f, -0.5f, 1};
    verts[7].normal = (Vec4) {-0.5f, -0.5f, -0.5f, 1};

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
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, position));
    // -- enable color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, rgba));
    // -- enable normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, normal));

    mesh->vert_count = index_count;
    mesh->indexed = true;

    // unselect
    glBindVertexArray(0); // stop the macro
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
}

void semesh_generate_unindexed(SE_Mesh *mesh, u32 vert_count, const SE_Vertex3D *vertices) {
    // generate buffers
    glGenBuffers(1, &mesh->vbo);
    glGenVertexArrays(1, &mesh->vao);

    glBindVertexArray(mesh->vao); // start the macro
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Vertex3D) * vert_count, vertices, GL_STATIC_DRAW);

    // enable first attribute as position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, position));
    // enable second attribute as color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SE_Vertex3D), (void*)offsetof(SE_Vertex3D, rgba));

    mesh->vert_count = vert_count;
    mesh->indexed = false;

    // unselect
    glBindVertexArray(0); // stop the macro
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
}

void semesh_draw(SE_Mesh *mesh) {
    glBindVertexArray(mesh->vao);

    if (mesh->indexed) {
        glDrawElements(GL_TRIANGLES, mesh->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count);
    }
    glBindVertexArray(0);
}