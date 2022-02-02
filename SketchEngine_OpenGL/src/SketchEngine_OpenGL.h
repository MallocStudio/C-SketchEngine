#ifndef SKETCHENGINE_OPENGL
#define SKETCHENGINE_OPENGL
#include "GL/glew.h"
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "raymath.h"


/// debugging for SDL2
void print_sdl_error();

#define new(type) ( type *) malloc (sizeof( type ))
#define ERROR_ON_NOTZERO_SDL(x, additional_message) if( x != 0) {printf("(%s)\n", additional_message); print_sdl_error();}
#define ERROR_ON_NULL_SDL(x, additional_message) if( x == NULL) {printf("(%s)\n", additional_message); print_sdl_error();}

typedef int i32;
typedef Uint32 u32;
// typedef Uint16 u16;
typedef float f32;
typedef Matrix  mat4;
typedef Vector2 vec2;
typedef Vector3 vec3;
typedef Vector4 vec4;

vec4 mat4_mul_vec4 (const mat4 *m, const vec4 *v);

typedef enum bool {
    false, true
} bool;

typedef struct SEGL_Camera {
    f32 height;
    vec2 center;
    f32 aspect_ratio;
    f32 speed;
} SEGL_Camera;
void segl_camera_init(SEGL_Camera *cam);
mat4 segl_get_camera_transform(SEGL_Camera *cam);

/// A copy of what Finn did
typedef struct Shader_Program {
    // Remember, OpenGL manages its resources itself and gives you handles to them.
    // That's what all this 'GLuint' business is about.
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    bool loaded_successfully;
} Shader_Program;
void segl_shader_program_init(Shader_Program *shader_program);
/// creates GL resources and links the given shaders
void segl_shader_program_init_from (Shader_Program *shader_program, const char *vertex_filename, const char *fragment_filename);
/// unloads GL resources
void segl_shader_program_deinit(Shader_Program *sp);
void segl_shader_program_use_shader(Shader_Program *sp); // @check do we need this?
GLuint segl_shader_program_get_uniform_loc  (Shader_Program *sp, const char *var_name);
void   segl_shader_program_set_uniform_f32  (Shader_Program *sp, const char *var_name, f32 value);
void   segl_shader_program_set_uniform_vec3 (Shader_Program *sp, const char *var_name, vec3 value);
void   segl_shader_program_set_uniform_mat4 (Shader_Program *sp, const char *var_name, mat4 value);
/// returns a pointer to a string on the heap.
//! Needs to be freed by the called
const char* load_file_as_string(const char *file_name);

/// --------------------
/// the lazy foo example
/// --------------------
typedef struct SEGL_Context {
    // -- Graphics program
    GLuint gProgramID;
    GLint gVertexPos2DLocation;
    GLuint gVBO;
    GLuint gIBO;
    SDL_Window *window;
} SEGL_Context; // Sketch Engine GL Context
/// some example functions
bool initGL(SEGL_Context *ctx);
void render(SEGL_Context *ctx);

/// -----------
/// custom test
/// -----------
typedef struct Finn_Game {
    Shader_Program *shader_program;
    SEGL_Camera *camera;
    SDL_Window *window;
} Finn_Game;
void finn_game_init(Finn_Game *game);
void finn_game_deinit(Finn_Game *game);
void finn_game_update(Finn_Game *game);
void finn_game_render(Finn_Game *game);


#endif // SKETCHENGINE_OPENGL