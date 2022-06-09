#include "seshader.h"
#include <stdio.h> // for loading file as string

void seshader_init_from(SE_Shader *sp, const char *vertex_filename, const char *fragment_filename) {
    sp->loaded_successfully = true; // set to false later on if errors occure
    sp->has_geometry = false;

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

void seshader_init_from_with_geometry(SE_Shader *sp, const char *vertex_filename, const char *fragment_filename, const char *geometry_filename) {
    // @copypasta massive from above
    sp->loaded_successfully = true; // set to false later on if errors occure
    sp->has_geometry = true;

    sp->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    sp->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    sp->geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    sp->shader_program = glCreateProgram();

    char *vertex_src = se_load_file_as_string(vertex_filename);
    char *frag_src = se_load_file_as_string(fragment_filename);
    char *geometric_src = se_load_file_as_string(geometry_filename);

    GLchar error_log[512];
    GLint success = 0;

    // vertex
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

    // fragment
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

    // geometry
    glShaderSource(sp->geometry_shader, 1, &geometric_src, NULL);
    glCompileShader(sp->geometry_shader);

    glGetShaderiv(sp->geometry_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        printf ("geometry shader %s failed with error:\n", geometry_filename);
        glGetShaderInfoLog(sp->geometry_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", geometry_filename);
        // printf("%s\n", geometric_src);
    }

    glAttachShader(sp->shader_program, sp->vertex_shader);
    glAttachShader(sp->shader_program, sp->geometry_shader);
    glAttachShader(sp->shader_program, sp->fragment_shader);
    glLinkProgram(sp->shader_program);
    glGetProgramiv(sp->shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        printf ("Error linking shaders \\%s\\ and \\%s\\ and \\%s\\\n", vertex_filename, fragment_filename, geometry_filename);
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
        glDeleteShader(sp->geometry_shader);
        glDeleteProgram(sp->shader_program);
    }

    free(vertex_src);
    free(frag_src);
}

void seshader_deinit(SE_Shader *shader) {
    if (shader->loaded_successfully) {
        glDeleteShader(shader->vertex_shader);
        glDeleteShader(shader->fragment_shader);
        if (shader->has_geometry) glDeleteShader(shader->geometry_shader);
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

void seshader_set_uniform_vec4 (SE_Shader *shader, const char *uniform_name, Vec4 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniform4f(var_loc, value.x, value.y, value.z, value.w);
}

void seshader_set_uniform_vec2 (SE_Shader *shader, const char *uniform_name, Vec2 value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniform2f(var_loc, value.x, value.y);
}

void seshader_set_uniform_rgb (SE_Shader *shader, const char *uniform_name, RGB value) {
    GLuint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    seshader_use(shader);
    glUniform3f(var_loc, value.r / 255.0f, value.g / 255.0f, value.b / 255.0f);
}

void seshader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, Mat4 value) {
    glGetError();
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("some error has occured at %s : %i\n", __FILE__, __LINE__);
    }
    seshader_use(shader);
    glUniformMatrix4fv(var_loc, 1, GL_FALSE, (const GLfloat*)&value);
}

void seshader_set_uniform_mat4_array (SE_Shader *shader, const char *uniform_name, Mat4 *value, u32 count) {
    glGetError();
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("some error has occured at %s : %i\n", __FILE__, __LINE__);
    }
    seshader_use(shader);
    glUniformMatrix4fv(var_loc, count, GL_FALSE, (const GLfloat*)value);
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