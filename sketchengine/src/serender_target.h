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
    bool has_depth; // set during init
    GLuint frame_buffer;
    GLuint texture; // rendered texture
    GLuint depth_buffer;
} SE_Render_Target;

void serender_target_init(SE_Render_Target *render_target, const Rect viewport, const bool has_depth);
void serender_target_deinit(SE_Render_Target *render_target);
void serender_target_use(SE_Render_Target *render_target);

#endif // SERENDER_TARGET_H