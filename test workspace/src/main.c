/// SDL Inlcudes
#define SDL_MAIN_HANDLED // gets rid of linking errors
#include <SDL2/SDL.h>
#include <gl/glew.h>
#include <SDL2/SDL_opengl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <string.h>

// -- Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;

typedef enum bool {
    false = 0,
    true = 1,
} bool;
typedef int i32;
typedef Uint32 u32;

// -- Shader loading utility programs
void print_program_log (GLuint program) {
    // @incomplete checkout http://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
}

void print_shader_log  (GLuint shader) {
    // @incomplete checkout http://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
}

bool initGL() {
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
    gProgramID = glCreateProgram();

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
        glAttachShader(gProgramID, vertex_shader);

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
            glAttachShader (gProgramID, fragment_shader);

            // -- Link program
            glLinkProgram (gProgramID);

            // -- Check for errors
            GLint program_success = GL_TRUE;
            glGetProgramiv (gProgramID, GL_LINK_STATUS, &program_success);

            if (program_success != GL_TRUE) {
                printf ("Error linking program %d\n", gProgramID);
                print_program_log (gProgramID);
                success = false;
            } else {
                // -- Get vertex attribute location
                gVertexPos2DLocation = glGetAttribLocation (gProgramID, "LVertexPos2D");

                if (gVertexPos2DLocation == -1) {
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
                    glGenBuffers (1, &gVBO);
                    glBindBuffer (GL_ARRAY_BUFFER, gVBO);
                    glBufferData (GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertex_data, GL_STATIC_DRAW);

                    // -- Create IBO
                    glGenBuffers (1, &gIBO);
                    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, gIBO);
                    glBufferData (GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), index_data, GL_STATIC_DRAW);
                }
            }
        }
    }
    return success;
}

void render () {
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
    glUseProgram (gProgramID);

    // -- Enable vertex position
    glEnableVertexAttribArray (gVertexPos2DLocation);

    // -- Set vertex data
    glBindBuffer          (GL_ARRAY_BUFFER, gVBO);
    glVertexAttribPointer (gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);

    // -- Set index data and render
    glBindBuffer   (GL_ELEMENT_ARRAY_BUFFER, gIBO);
    glDrawElements (GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

    // -- Disable vertex position
    glDisableVertexAttribArray (gVertexPos2DLocation);

    // -- Unbind program
    glUseProgram (0);
}

/// debugging for SDL2
#define ERROR_ON_NOTZERO_SDL(x, additional_message) if( x != 0) {printf("(%s)\n", additional_message); print_sdl_error();}
#define ERROR_ON_NULL_SDL(x, additional_message) if( x == NULL) {printf("(%s)\n", additional_message); print_sdl_error();}
void print_sdl_error() {
    const char *error = SDL_GetError();
    if (strlen(error) <= 0) {
        int x = 0; // dummy assignment for breakpoints
    }
    printf("ERROR: %s\n", error);
}

int main (int argc, char *argv[]) {
    /// Setup SDL
    SDL_Window *window;
    // SDL_Renderer *sdl_renderer; // the whole point of this is that we don't have a renderer
    i32 window_width  = 800;
    i32 window_height = 600;

    ERROR_ON_NOTZERO_SDL(SDL_Init(SDL_INIT_EVERYTHING), "setup sdl");

    // // -- sdl_renderer
    // sdl_renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    // ERROR_ON_NULL_SDL(sdl_renderer, "setup sdl_renderer");
    // SDL_RenderSetLogicalSize(sdl_renderer, window_width, window_height);

    /// Setup OpenGL stuff

    // -- Use OpenGL 3.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // -- window
    window = SDL_CreateWindow("My OpenGL Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ERROR_ON_NULL_SDL(window, "init_sdl");
    
    // -- Create context
    SDL_GLContext g_context = SDL_GL_CreateContext(window);
    ERROR_ON_NULL_SDL(g_context, "OpenGL context could not be created!");

    // -- Init GLEW
    bool glew_experimental = GL_TRUE;
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        printf("Error init GLEW! %s\n", glewGetErrorString(glew_error));
    }

    // -- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

    // -- Init OpenGL
    if (!initGL()) {
        printf("Unable to init OpenGL!\n");
    }

    bool quit = false;

    while (!quit) {
        render();
        SDL_GL_SwapWindow(window);
        printf("rendering\n");
    }
    
    // -- Exit SDL
    SDL_DestroyWindow(window);
    // SDL_DestroyRenderer(sdl_renderer);
    SDL_Quit();

    printf("we did something\n");
    return 0;
}

#pragma region // after -- events in main loop 
        // -- keyboard state
        // SDL_PumpEvents();
        // SDL_GetKeyboardState(app.keyboard);
        // if (app.keyboard[SDL_SCANCODE_ESCAPE] == 1) should_close = true;

        // -- update window surface // ! (matink dec 2021) we cannot use window surface if we're using the sdl_renderer according to https://dev.to/noah11012/using-sdl2-2d-accelerated-renderering-1kcb
        // app.surface = sdl.GetWindowSurface(app.window)
        // if app.surface == nil do sdl_print_error("GetWindowSurface:", sdl.GetError())
        // if sdl.UpdateWindowSurface(app.window) < 0 do sdl_print_error("UpdateWindowSurface:", sdl.GetError())
#pragma endregion