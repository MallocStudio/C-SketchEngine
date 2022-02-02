#include "SketchEngine_OpenGL.h"
#include <stdio.h>

void print_sdl_error() {
    const char *error = SDL_GetError();
    if (strlen(error) <= 0) {
        int x = 0; // dummy assignment for breakpoints
    }
    printf("ERROR: %s\n", error);
}

/// get the mouse position (relative to the window). Optionally pass bools to get mouse state
vec2 get_mouse_pos(bool *lpressed, bool *rpressed) {
    i32 x, y;
    u32 state = SDL_GetMouseState(&x, &y);
    if (lpressed != NULL) {
        *lpressed = false;
        if (state & SDL_BUTTON_LEFT) *lpressed = true;
    }
    if (rpressed != NULL) {
        *rpressed = false;
        if (state & SDL_BUTTON_RIGHT) *rpressed = true;
    }
    return (vec2){x, y};
}

/// -----------------
/// lazy foo tutorial
/// -----------------
// -- Shader loading utility programs
void print_program_log (GLuint program) {
    // @incomplete checkout http://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
}

void print_shader_log  (GLuint shader) {
    // @incomplete checkout http://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
}

bool initGL(SEGL_Context *ctx) {
    /*bool success = true;
    GLenum error = GL_NO_ERROR;

    // -- Init Projection Matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // -- check for errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error init OpenGL! %s\n", gluErrorString(error));
        success = false;
    }

    // -- init Modelview Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // -- check for errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error initializing OpenGL! %s\n", gluErrorString(error));
        success = false;
    }

    // -- Init clear color
    glClearColor(0.f, 0.f, 0.f, 1.f);

    // -- Check for error
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error init OpenGL! %s\n", gluErrorString(error));
        success = false;
    }

    return success;*/
    bool success = true;

    // -- generate program
    ctx->gProgramID = glCreateProgram();

    // -- create vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

    // -- get vertex source
    const GLchar *vertex_shader_source[] = {
        "#version 140\nin vec2 LVertexPos2D; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); }"
    };

    // -- set vertex source
    glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);

    // -- compile vertex source
    glCompileShader(vertex_shader);

    // -- check vertex shader for errors
    GLint v_shader_compiled = GL_FALSE;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_shader_compiled);

    if (v_shader_compiled != GL_TRUE) {
        printf("Unable to compile vertex shader %d\n", vertex_shader);
        print_shader_log(vertex_shader);
        success = false;
    } else {
        // -- Attach vertex shader to program
        glAttachShader(ctx->gProgramID, vertex_shader);

        // -- create fragment shader
        GLuint fragment_shader = glCreateShader (GL_FRAGMENT_SHADER);

        // -- get fragment source
        const GLchar *fragment_shader_source[] = {
            "#version 140\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"
        };

        // -- set fragment source
        glShaderSource (fragment_shader, 1, fragment_shader_source, NULL);

        // -- compile fragment source
        glCompileShader (fragment_shader);

        // -- check fragment shader for errors
        GLint f_shader_compiled = GL_FALSE;
        glGetShaderiv (fragment_shader, GL_COMPILE_STATUS, &f_shader_compiled);

        if (f_shader_compiled != GL_TRUE) {
            printf ("Unable to compile fragment shader %d\n", fragment_shader);
            print_shader_log (fragment_shader);
            success = false;
        } else {
            // -- Attach fragment shader to program
            glAttachShader (ctx->gProgramID, fragment_shader);

            // -- Link program
            glLinkProgram (ctx->gProgramID);

            // -- Check for errors
            GLint program_success = GL_TRUE;
            glGetProgramiv (ctx->gProgramID, GL_LINK_STATUS, &program_success);

            if (program_success != GL_TRUE) {
                printf ("Error linking program %d\n", ctx->gProgramID);
                print_program_log (ctx->gProgramID);
                success = false;
            } else {
                // -- Get vertex attribute location
                ctx->gVertexPos2DLocation = glGetAttribLocation (ctx->gProgramID, "LVertexPos2D");

                if (ctx->gVertexPos2DLocation == -1) {
                    printf ("LVertexPos2D is not a valid glsl program variable!\n");
                    success = false;
                } else {
                    // -- init clear color
                    glClearColor(0.f, 0.f, 0.f, 1.f);

                    // -- VBO data
                    GLfloat vertex_data[] = {
                        -0.5f, -0.5f,
                        +0.5f, -0.5f,
                        +0.5f, +0.5f,
                        -0.5f, +0.5f
                    };

                    // -- IBO data
                    GLuint index_data[] = {0, 1, 2, 3};

                    // -- Create VBO
                    glGenBuffers (1, &ctx->gVBO);
                    glBindBuffer (GL_ARRAY_BUFFER, ctx->gVBO);
                    glBufferData (GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertex_data, GL_STATIC_DRAW);

                    // -- Create IBO
                    glGenBuffers (1, &ctx->gIBO);
                    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ctx->gIBO);
                    glBufferData (GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), index_data, GL_STATIC_DRAW);
                }
            }
        }
    }
    return success;
}

void render (SEGL_Context *ctx) {
   /*
   // Clear color buffer
   glClear(GL_COLOR_BUFFER_BIT);
   
   // render quad
   glBegin (GL_QUADS);
       glVertex2f(-0.5f, -0.5f);
       glVertex2f(+0.5f, -0.5f);
       glVertex2f(+0.5f, +0.5f);
       glVertex2f(-0.5f, +0.5f);
   glEnd();
   
   // ! The important thing to know is that OpenGL uses normalized coordinates. This means they go from -1 to 1
   // ! This means our quad is wider than it is tall. In order to get a coordinate system like SDL's, 
   // ! you're going to have to set the project matrix to an orthographic perspective.
   */
    // -- Clear color buffer
    glClear (GL_COLOR_BUFFER_BIT);

    /// Render quad
    // -- Bind program
    glUseProgram (ctx->gProgramID);

    // -- Enable vertex position
    glEnableVertexAttribArray (ctx->gVertexPos2DLocation);

    // -- Set vertex data
    glBindBuffer          (GL_ARRAY_BUFFER, ctx->gVBO);
    glVertexAttribPointer (ctx->gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);

    // -- Set index data and render
    glBindBuffer   (GL_ELEMENT_ARRAY_BUFFER, ctx->gIBO);
    glDrawElements (GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

    // -- Disable vertex position
    glDisableVertexAttribArray (ctx->gVertexPos2DLocation);

    // -- Unbind program
    glUseProgram (0);
}

/// --------------
/// Finn's example
/// --------------
void update(SEGL_Context *ctx, SEGL_Camera *cam) {
    i32 width, height;
    SDL_GetWindowSize(ctx->window, &width, &height);

    mat4 deprojection = MatrixInvert(segl_get_camera_transform(cam));
    vec2 cursor_pos;
    cursor_pos = get_mouse_pos(NULL, NULL);
    cursor_pos.x = (cursor_pos.x / width ) * 2.0f - 1.0f;
    cursor_pos.y = (cursor_pos.y / height) * 2.0f - 1.0f;

    vec4 mouse_pos_ndc = {cursor_pos.x, cursor_pos.y, 0, 1};
    vec4 mouse_pos_world = mat4_mul_vec4(&deprojection, &mouse_pos_ndc);

    cursor_pos.x = mouse_pos_world.x;
    cursor_pos.y = mouse_pos_world.y;

    // printf("mouse pos: %f, %f\n", cursor_pos.x, cursor_pos.y);
}

void segl_camera_init(SEGL_Camera *cam) {
    cam->aspect_ratio = 16.0f / 9.0f;
    cam->height = 10.0f;
    cam->speed = 0.8f;
}

mat4 segl_get_camera_transform(SEGL_Camera *cam) {
    return MatrixOrtho(
        -cam->aspect_ratio * cam->height * 0.5f + cam->center.x,
        +cam->aspect_ratio * cam->height * 0.5f + cam->center.x,
        -cam->height * 0.5f + cam->center.y,
        +cam->height * 0.5f + cam->center.y,
        -1.0f, 1.0f);
}

vec4 mat4_mul_vec4 (const mat4 *m, const vec4 *v) {
    // @incomplete move to either my own math library, or raymath's source code
    vec4 result;
    result.x = m->m0  * v->x + m->m1  * v->y + m->m2  * v->z + v->w + m->m3  * v->w;
    result.y = m->m4  * v->x + m->m5  * v->y + m->m6  * v->z + v->w + m->m7  * v->w;
    result.z = m->m8  * v->x + m->m9  * v->y + m->m10 * v->z + v->w + m->m11 * v->w;
    result.w = m->m12 * v->x + m->m13 * v->y + m->m14 * v->z + v->w + m->m15 * v->w;
    return result;
}

void segl_shader_program_init(Shader_Program *shader_program) {
    shader_program->vertex_shader = -1;
    shader_program->fragment_shader = -1;
    shader_program->shader_program = -1;
    shader_program->loaded_successfully = false;
}

void segl_shader_program_init_from (Shader_Program *sp, const char *vertex_filename, const char *fragment_filename) {
    sp->loaded_successfully = true;
    
    sp->vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
    sp->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    sp->shader_program = glCreateProgram();

    const char *vertex_src = load_file_as_string(vertex_filename);
    const char *frag_src   = load_file_as_string(fragment_filename);

    glShaderSource(sp->vertex_shader, 1, &vertex_src, NULL);
    glCompileShader(sp->vertex_shader);

    GLchar error_log[512];
    GLint success = 0;

    glGetShaderiv(sp->vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // something failed with the vertex shader compilation
        printf ("vertex shader %s failed with error:\n", vertex_filename);
        glGetShaderInfoLog(sp->vertex_shader, 512, NULL, error_log);
        printf("%s\n", error_log);
        sp->loaded_successfully = false;
    } else {
        printf ("\\%s\\ compiled successfully.\n", vertex_filename);
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

void segl_shader_program_deinit(Shader_Program *sp) {
    if (sp->loaded_successfully) {
        glDeleteShader(sp->vertex_shader);
        glDeleteShader(sp->fragment_shader);
        glDeleteProgram(sp->shader_program);
    }
}

GLuint segl_shader_program_get_uniform_loc (Shader_Program *sp, const char *var_name) {
    return glGetUniformLocation(sp->shader_program, var_name);
}

void segl_shader_program_set_uniform_f32 (Shader_Program *sp, const char *var_name, f32 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniform1f(var_loc, value);
}

void segl_shader_program_set_uniform_vec3 (Shader_Program *sp, const char *var_name, vec3 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniform3f(var_loc, value.x, value.y, value.z);
}

void segl_shader_program_set_uniform_mat4 (Shader_Program *sp, const char *var_name, mat4 value) {
    GLuint var_loc = glGetUniformLocation(sp->shader_program, var_name);
    segl_shader_program_use_shader(sp);
    glUniformMatrix4fv(var_loc, 1, GL_FALSE, &value); // @check (Finn did &value[0][0] with glm::mat4)
}


void segl_shader_program_use_shader(Shader_Program *sp) {
    glUseProgram(sp->shader_program);
}

const char* load_file_as_string(const char *file_name) {
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

/// -----------------
/// Finn Game example
/// -----------------
void finn_game_init(Finn_Game *game) {
    SDL_Window *window;
    i32 window_w = 800;
    i32 window_h = 400;
    
    game->camera = new(SEGL_Camera);
    segl_camera_init(game->camera);

    game->shader_program = new(Shader_Program);
    segl_shader_program_init_from(game->shader_program, "Simple.vsd", "Simple.fsd");
    segl_shader_program_use_shader(game->shader_program);

    glClear(0, 0, 0, 1);
}

void finn_game_deinit(Finn_Game *game) {
    free(game->camera);
    segl_shader_program_deinit(game->shader_program);
    free(game->shader_program);
}

void finn_game_update(Finn_Game *game) {
    // @incomplete
}

void finn_game_render(Finn_Game *game) {
    // @incomplete
}
