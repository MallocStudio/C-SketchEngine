#ifndef LAZYFOO_RENDERER_H
#define LAZYFOO_RENDERER_H

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include <stdlib.h>
#include "defines.h"

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


#endif // LAZYFOO_RENDERER_H