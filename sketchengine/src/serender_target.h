#ifndef SERENDER_TARGET_H
#define SERENDER_TARGET_H

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"

///
/// Render target
///

typedef struct SE_Render_Target {
    Rect viewport;
    b8 has_depth; // set during init
    GLuint frame_buffer;
    GLuint texture; // rendered texture
    GLuint depth_buffer;
} SE_Render_Target;

typedef struct SE_Render_Target_Config {
    // size of the generated texture
    Vec2 size;
    // whether we have a colour pass
    b8 has_colour;
    // whether we have a depth pass
    b8 has_depth;
    // options : GL_RED, GL_BLUE, GL_GREEN, GL_RGB etc...
    GLint  internal_format;
    GLenum format;
    // options: GL_UNSIGNED_BYTE, GL_FLOAT, etc...
    GLenum type;

    /* texture parameter */
    GLint wrap_s; // GL_CLAMP_TO_EDGE,
    GLint wrap_t; // GL_CLAMP_TO_EDGE,
    GLint min_filter; // GL_LINEAR, GL_NEAREST
    GLint mag_filter; // GL_LINEAR, GL_NEAREST
} SE_Render_Target_Config;

void serender_target_init_ext(SE_Render_Target *render_target, SE_Render_Target_Config config);
void serender_target_init(SE_Render_Target *render_target, const Rect viewport, const b8 has_colour, const b8 has_depth);
void se_render_target_init_hdr(SE_Render_Target *render_target, const Rect viewport);
void serender_target_deinit(SE_Render_Target *render_target);
void serender_target_use(SE_Render_Target *render_target);

#endif // SERENDER_TARGET_H