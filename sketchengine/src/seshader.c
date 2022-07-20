#include "seshader.h"
#include <stdio.h> // for loading file as string
#include "sestring.h"

void se_shader_init_from_string(SE_Shader *sp, const char *vertex_src, const char *frag_src, const char* vertex_shader_name, const char *fragment_shader_name) {
    sp->loaded_successfully = true; // set to false later on if errors occure
    sp->has_geometry = false;

    sp->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    sp->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    sp->shader_program = glCreateProgram();

    GLchar error_log[512];
    GLint success = 0;

    glShaderSource(sp->vertex_shader, 1, &vertex_src, NULL);
    glCompileShader(sp->vertex_shader);

    glGetShaderiv(sp->vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // something failed with the vertex shader compilation
        printf ("vertex shader %s failed with error:\n", vertex_shader_name);
        glGetShaderInfoLog(sp->vertex_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", vertex_shader_name);
        // printf("%s\n", vertex_src);
    }

    glShaderSource(sp->fragment_shader, 1, &frag_src, NULL);
    glCompileShader(sp->fragment_shader);

    glGetShaderiv(sp->fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        printf ("fragment shader %s failed with error:\n", fragment_shader_name);
        glGetShaderInfoLog(sp->fragment_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", fragment_shader_name);
        // printf("%s\n", frag_src);
    }

    glAttachShader(sp->shader_program, sp->vertex_shader);
    glAttachShader(sp->shader_program, sp->fragment_shader);
    glLinkProgram(sp->shader_program);
    glGetProgramiv(sp->shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        printf ("Error linking shaders \\%s\\ and \\%s\\\n", vertex_shader_name, fragment_shader_name);
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
}

void se_shader_deinit(SE_Shader *shader) {
    if (shader->loaded_successfully) {
        glDeleteShader(shader->vertex_shader);
        glDeleteShader(shader->fragment_shader);
        if (shader->has_geometry) glDeleteShader(shader->geometry_shader);
        glDeleteProgram(shader->shader_program);
    }
}

void se_shader_use(const SE_Shader *shader) {
    glUseProgram(shader->shader_program);
}

GLint se_shader_get_uniform_loc(SE_Shader *shader, const char *uniform_name) {
    return glGetUniformLocation(shader->shader_program, uniform_name);
}

void se_shader_set_uniform_f32  (SE_Shader *shader, const char *uniform_name, f32 value) {
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    // if ((i32)var_loc == -1) printf("Something's boned (%s)\n", uniform_name);
    se_shader_use(shader);
    glUniform1f(var_loc, value);
}

void se_shader_set_uniform_i32  (SE_Shader *shader, const char *uniform_name, i32 value) {
    glGetError(); // @debug

    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);

        // @debug
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("some error has occured at %s : %i\n", __FILE__, __LINE__);
    }
        // @debug
    if (var_loc == -1) {
        printf("Something's boned (%s)\n", uniform_name);
    }
    se_shader_use(shader);
    glUniform1i(var_loc, value);
}

void se_shader_set_uniform_vec3 (SE_Shader *shader, const char *uniform_name, Vec3 value) {
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    se_shader_use(shader);
    glUniform3f(var_loc, value.x, value.y, value.z);
}

void se_shader_set_uniform_vec4 (SE_Shader *shader, const char *uniform_name, Vec4 value) {
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    se_shader_use(shader);
    glUniform4f(var_loc, value.x, value.y, value.z, value.w);
}

void se_shader_set_uniform_vec2 (SE_Shader *shader, const char *uniform_name, Vec2 value) {
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    se_shader_use(shader);
    glUniform2f(var_loc, value.x, value.y);
}

void se_shader_set_uniform_rgb (SE_Shader *shader, const char *uniform_name, RGB value) {
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    se_shader_use(shader);
    glUniform3f(var_loc, value.r / 255.0f, value.g / 255.0f, value.b / 255.0f);
}

void se_shader_set_uniform_rgba (SE_Shader *shader, const char *uniform_name, RGBA value) {
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    se_shader_use(shader);
    glUniform4f(var_loc, value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f);
}

void se_shader_set_uniform_mat4 (SE_Shader *shader, const char *uniform_name, Mat4 value) {
    glGetError();
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("some error has occured at %s : %i\n", __FILE__, __LINE__);
    }
    se_shader_use(shader);
    glUniformMatrix4fv(var_loc, 1, GL_FALSE, (const GLfloat*)&value);
}

void se_shader_set_uniform_mat4_array (SE_Shader *shader, const char *uniform_name, Mat4 *value, u32 count) {
    glGetError();
    GLint var_loc = glGetUniformLocation(shader->shader_program, uniform_name);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("some error has occured at %s : %i\n", __FILE__, __LINE__);
    }
    se_shader_use(shader);
    // for (u32 i = 0; i < count; ++i) {
    //     // NOTE(Matin): we're not certain that this is supported on all drivers, not sure if this is part of the spec
    //     // glUniformMatrix4fv(var_loc + i, 1, GL_FALSE, (const GLfloat*)(&value + i));
    // }
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
                printf("file (%s) reading error at %s: %i\n", file_name, __FILE__,__LINE__);
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

char *se_combine_files_to_string(const char **files, u32 count) {
    if (count == 0 || files == NULL) return NULL;

    SE_String string;
    se_string_init(&string, "");

    for (u32 i = 0; i < count; ++i) {
        char *src = se_load_file_as_string(files[i]);
        se_string_append(&string, src);
        free(src);
    }

    char *result = malloc(sizeof(char) * string.size + 1);
    memcpy(result, string.buffer, sizeof(char) * string.size + 1);

    se_string_deinit(&string);
    return result;
}

void se_shader_init_from_files (SE_Shader *sp,
                                const char **vertex_files,
                                u32 vertex_count,
                                const char **fragment_files,
                                u32 fragment_count,
                                const char **geometry_files,
                                u32 geometry_count) {
    // create a shader from the given files
    sp->loaded_successfully = true;
    if (geometry_count > 0) sp->has_geometry = true;
    else                    sp->has_geometry = false;

    // create opengl resources
    sp->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    sp->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (sp->has_geometry) sp->geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    sp->shader_program = glCreateProgram();

    // load sources
    char   *vertex_src = se_combine_files_to_string(vertex_files,   vertex_count);
    char *fragment_src = se_combine_files_to_string(fragment_files, fragment_count);

    char *geometry_src;
    if (sp->has_geometry) geometry_src = se_combine_files_to_string(geometry_files, geometry_count);

    // compile and link
    GLchar error_log[512];
    GLint success = 0;

    // vertex
    glShaderSource(sp->vertex_shader, 1, &vertex_src, NULL);
    glCompileShader(sp->vertex_shader);

    glGetShaderiv(sp->vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // something failed with the vertex shader compilation
        printf ("vertex shader failed with error:\n");
        glGetShaderInfoLog(sp->vertex_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        printf ("source was:\n%s\n", vertex_src);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", vertex_files[0]);
    }

    // fragment
    glShaderSource(sp->fragment_shader, 1, &fragment_src, NULL);
    glCompileShader(sp->fragment_shader);

    glGetShaderiv(sp->fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        printf ("fragment shader failed with error:\n");
        glGetShaderInfoLog(sp->fragment_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        printf ("source was:\n%s\n", fragment_src);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", fragment_files[0]);
    }

    if (sp->has_geometry) { // geometry
        glShaderSource(sp->geometry_shader, 1, &geometry_src, NULL);
        glCompileShader(sp->geometry_shader);

        glGetShaderiv(sp->geometry_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            printf ("geometry shader failed with error:\n");
            glGetShaderInfoLog(sp->geometry_shader, 512, NULL, error_log);
            printf("%s\n", error_log);
            printf ("source was:\n%s\n", geometry_src);
            sp->loaded_successfully = false;
        } else {
            printf ("\\%s\\ compiled successfully.\n", geometry_files[0]);
        }
    }

    glAttachShader(sp->shader_program, sp->vertex_shader);
    if (sp->has_geometry) glAttachShader(sp->shader_program, sp->geometry_shader);
    glAttachShader(sp->shader_program, sp->fragment_shader);
    glLinkProgram(sp->shader_program);
    glGetProgramiv(sp->shader_program, GL_LINK_STATUS, &success);

    if (!success) { // error linking
        if (sp->has_geometry) {
            printf ("Error linking shaders \\ %s \\ %s \\ %s \n", vertex_files[0], fragment_files[0], geometry_files[0]);
        } else {
            printf ("Error linking shaders \\ %s \\ %s \n", vertex_files[0], fragment_files[0]);
        }

        glGetProgramInfoLog(sp->shader_program, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    }

    // print result
    if (sp->loaded_successfully) {
        printf ("Shaders compiled and linked successfully.\n");
    } else {
        // if there was a problem, tell OpenGL that we don't need those resources after all
        glDeleteShader(sp->vertex_shader);
        glDeleteShader(sp->fragment_shader);
        if (sp->has_geometry) glDeleteShader(sp->geometry_shader);
        glDeleteProgram(sp->shader_program);
    }

    free(vertex_src);
    free(fragment_src);
    if (sp->has_geometry) free(geometry_src);
}