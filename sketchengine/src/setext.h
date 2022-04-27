// https://learnopengl.com/In-Practice/Text-Rendering
#ifndef SETEXT_H
#define SETEXT_H

#include "sedefines.h"
#include "FreeType/freetype.h"
// #include "serenderer.h"
#include "serenderer.h"

// #define DEFAULT_FONT_PATH "assets/fonts/Ya'ahowu/Yaahowu.ttf"
#define DEFAULT_FONT_PATH "assets/fonts/josefin-sans-font/JosefinSansRegular-x3LYV.ttf"
#define SETEXT_SUCCESS 0 // must be zero
#define SETEXT_ERROR 1

#define SE_TEXT_NUM_OF_GLYPHS 256 // ASCII
typedef struct SE_Text_Character {
    unsigned char character;
    u32 texture_id; // ID handle of the glyph texture
    i32 width;      // size of glyph
    i32 height;     // size of glyph
    i32 bearing_x;  // offset from baseline to the left of glyph
    i32 bearing_y;  // offset from baseline to the top of glyph
    u32 advance;    // offset to advance to next glyph
} SE_Text_Character;

typedef struct SE_Text_Stored_Letter {
    f32 vertices[6][4];
    f32 x, y, scale;
    Vec3 color; // @TODO change to rgb

    GLuint texture_ids;
} SE_Text_Stored_Letter;

typedef enum SETEXT_FLAGS {
    SETEXT_FLAG_CENTER = 0,
} SETEXT_FLAGS;

#define SE_TEXT_RENDERER_MAX_STRINGS 1024
typedef struct SE_Text_Renderer {
    FT_Library library;
    FT_Face face; // the font
    SE_Shader *shader_program;

    u32 VAO, VBO;
    bool initialised;

    SE_Text_Character characters[SE_TEXT_NUM_OF_GLYPHS];

    Mat4 shader_projection_matrix;

    i32 strings_count;
    SE_Text_Stored_Letter generated_letters[SE_TEXT_RENDERER_MAX_STRINGS]; // the generated glyphs along with their data
} SE_Text_Renderer;

/// Initialises freetype.
/// SE_Text_Renderer will be used by the rest of the procedures.
/// To deinitialise the text renderer call setext_deinit to free all the
/// memory used by setext and freetype.
/// Returns SETEXT_ERROR if something went wrong.
i32 setext_init (SE_Text_Renderer *txt, Rect viewport);

/// Initialises freetype.
/// SE_Text_Renderer will be used by the rest of the procedures.
/// To deinitialise the text renderer call setext_deinit to free all the
/// memory used by setext and freetype.
/// ALSO Loads the given font. Note that setting either width or height to
/// zero, dynamically calculates the proper aspect ratio based on the other parameter.
/// Returns SETEXT_ERROR if something went wrong.
i32 setext_init_from (SE_Text_Renderer *txt, Rect viewport, const char *font_path, i32 width, i32 height);

/// Deinitialises setext and frees the memory used by freetype and setext
void setext_deinit(SE_Text_Renderer *txt);

/// Loads a font.
/// Setting either width or height to zero, dynamically calculates the proper aspect ratio
/// based on the other parameter.
/// Returns 0 on success
i32 setext_load_font(SE_Text_Renderer *txt, const char *font_path, i32 width, i32 height);

/// prints the characters hashmap into the console
i32 setext_print_loaded_characters(SE_Text_Renderer *txt);

/// add the given string at the given position with colour and scale to txt. use setext_render() to render the texts.
/// REMEMBER TO CALL SETEXT_RENDER() at the end, otherwise, we will corrupt memory because we increment txt->strings_count each time we call this procedure
i32 setext_render_text(SE_Text_Renderer *txt, const char *string, f32 x, f32 y, f32 scale, Vec3 color);

i32 setext_render_text_rect(SE_Text_Renderer *txt, const char *string, Rect rect, Vec3 color);

/// render all the stored glyphs at once and "clear" the stored array of glyphs.
/// Note that it does not actually draw an array of vertices at once yet. We go through each "String"
/// and set the shader color to that.
/// @TODO change this behaviour : store an array of colours that are synced with txt->strings.
i32 setext_render(SE_Text_Renderer *txt);

/// updates the viewport properties
/// viewport rect: mat4_ortho: {viewport.x, viewport.w, viewport.h, viewport.y}
void setext_set_viewport(SE_Text_Renderer *txt, Rect viewport);

/// return the area of the given string in the given text renderer's font (characters)
Vec2 setext_size_string(SE_Text_Renderer *txt, const char *string);
#endif // SETEXT_H