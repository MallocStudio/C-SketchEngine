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
        printf("%s\n", vertex_src);
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
        printf("%s\n", frag_src);
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

void semesh_generate_quad(SE_Mesh *mesh) {
    // -- generate buffers
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    // @note once we bind a VBO or IBO it "sticks" to the currently bound VAO, so we start by
    // binding VAO and then VBO.
    // This is so that later on, we'll just need to bind the vertex array object and not the buffers
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    // @temp generate the data here and send them to the GPU
    SE_Vertex3D verts[6];
    verts[0].position = (Vec4) {-0.5f, 0, +0.5f, 1};
    verts[1].position = (Vec4) {+0.5f, 0, +0.5f, 1};
    verts[2].position = (Vec4) {-0.5f, 0, -0.5f, 1};

    verts[3].position = (Vec4) {-0.5f, 0, -0.5f, 1};
    verts[4].position = (Vec4) {+0.5f, 0, +0.5f, 1};
    verts[5].position = (Vec4) {+0.5f, 0, -0.5f, 1};
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(SE_Vertex3D), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SE_Vertex3D), 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->tri_count = 2;

    // glDisableVertexAttribArray(0);
}

void semesh_draw(SE_Mesh *mesh) {
    glBindVertexArray(mesh->vao);
    // the number of vertices is tri_count * 3
    // @note glDrawElements is used for index buffers
    // but for now we're not using the index buffer
    glDrawArrays(GL_TRIANGLES, 0, 3 * mesh->tri_count);

    glBindVertexArray(0);
}

///
/// RENDERER
///

void serender3d_init(SE_Renderer3D *renderer, const char *vertex_filepath, const char *fragment_filepath) {
    seshader_init_from(&renderer->shader_program, vertex_filepath, fragment_filepath);

    glGenBuffers(1, &renderer->vertices_buffer_id);
    renderer->initialised = true;
}

void serender3d_deinit(SE_Renderer3D *renderer) {
    if (renderer->initialised) {
        // renderer->shapes_count = 0;
        // glGenBuffers(1, &renderer->vertices_buffer_id);
        // glGenBuffers(1, &renderer->colours_buffer_id); // @question wtf?
        glDeleteBuffers(1, &renderer->vertices_buffer_id);
        seshader_deinit(&renderer->shader_program);
        renderer->initialised = false;
    }
}

void serender2d_compile(SE_Renderer3D *renderer) {
    // select vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vertices_buffer_id);
    // give it the data of vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Vertex3D) * renderer->vertices_count, renderer->vertices, GL_DYNAMIC_DRAW);
}

void serender3d_update_frame(SE_Renderer3D *renderer) {
    if (renderer->vertices_count > 0) {
        serender2d_compile(renderer);
        serender3d_render(renderer);
    }
    serender3d_clear(renderer);
}

void serender3d_clear(SE_Renderer3D *renderer) {
    renderer->vertices_count = 0;
}

void serender3d_render(SE_Renderer3D *renderer) {
    seshader_use(&renderer->shader_program);
    // select the vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vertices_buffer_id);
    glEnableVertexAttribArray(0); // location 0
    glEnableVertexAttribArray(1); // location 1
    // must take a look at how a SE_Vertex3D is arranged
    // Vec3 position;
    // RGBA (Vec4 f32) rgba;
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(RGBA), (const void*)sizeof(Vec3)); // @check
    // set the selected buffer back to zero
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // -- render each triangle
    // @TODO

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}