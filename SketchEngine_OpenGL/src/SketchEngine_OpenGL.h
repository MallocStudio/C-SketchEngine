#ifndef SKETCHENGINE_OPENGL
#define SKETCHENGINE_OPENGL
#include "GL/glew.h"
#include <stdlib.h>
typedef int i32;
// typedef Uint32 u32;
// typedef Uint16 u16;
typedef float f32;
typedef enum bool {
    false, true
} bool;

typedef struct SE_GL_Context {
    // -- Graphics program
    GLuint gProgramID;
    GLint gVertexPos2DLocation;
    GLuint gVBO;
    GLuint gIBO;
} SE_GL_Context; // Sketch Engine GL Context

bool initGL(SE_GL_Context *ctx);
void render(SE_GL_Context *ctx);

#endif // SKETCHENGINE_OPENGL