#include "SketchEngine_OpenGL.h"

// -- Shader loading utility programs
void print_program_log (GLuint program) {
    // @incomplete checkout http://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
}

void print_shader_log  (GLuint shader) {
    // @incomplete checkout http://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
}

bool initGL(SE_GL_Context *ctx) {
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

void render (SE_GL_Context *ctx) {
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