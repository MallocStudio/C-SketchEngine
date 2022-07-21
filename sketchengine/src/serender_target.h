#ifndef SERENDER_TARGET_H
#define SERENDER_TARGET_H

#include "sedefines.h"
#include "GL/glew.h"
#include "semath.h"

///
/// Render target
///

#if 0 // old method // @remove
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
#endif

typedef struct SE_Render_Target_Config {
        // colour texture formats
    // options: GL_RED, GL_BLUE, GL_GREEN, GL_RGB, GL_RGB16F ...
    GLint internal_format;
    GLenum format;
    // options: GL_UNSIGNED_BYTE, GL_FLOAT ...
    GLenum type;

        // texture parameters
    GLint wrap; // GL_CLAMP_TO_EDGE ...
    GLint filter; // GL_LINEAR, GL_NEAREST
} SE_Render_Target_Config;

#define SE_RENDER_TARGET_MAX_TEXTURE 8
typedef struct SE_Render_Target {
    Vec2 texture_size;
    GLuint frame_buffer;
    // Rect viewport;
    // GLuint texture; // rendered texture
    // SE_Render_Target_Config config; // the config used to generate the texture and render target
    u32 colour_buffers_count;
    GLuint colour_buffers[SE_RENDER_TARGET_MAX_TEXTURE];
    b8 has_depth;
    GLuint depth_buffer;
} SE_Render_Target;


// void serender_target_init_ext(SE_Render_Target *render_target, SE_Render_Target_Config config);
// void serender_target_init(SE_Render_Target *render_target, const Rect viewport, const b8 has_colour, const b8 has_depth);
// void se_render_target_init_hdr(SE_Render_Target *render_target, const Rect viewport);

void serender_target_init_ext(SE_Render_Target *render_target, Vec2 size, u32 colour_count, b8 has_depth, SE_Render_Target_Config config);
void serender_target_init(SE_Render_Target *render_target, Vec2 size, u32 colour_count, b8 has_depth);
void se_render_target_init_hdr(SE_Render_Target *render_target, Vec2 size, u32 colour_count, b8 has_depth);

void serender_target_deinit(SE_Render_Target *render_target);
void serender_target_use(SE_Render_Target *render_target);
// void se_render_target_resize(SE_Render_Target *render_target, Rect new_viewport);

#endif // SERENDER_TARGET_H