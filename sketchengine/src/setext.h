// https://learnopengl.com/In-Practice/Text-Rendering
#ifndef SETEXT_H
#define SETEXT_H

#include "defines.h"
#include "FreeType/freetype.h"
#include "serenderer.h"

#define DEFAULT_FONT_PATH "assets/fonts/Ya'ahowu/Yaahowu.ttf"
// #define DEFAULT_FONT_PATH "assets/fonts/josefin-sans-font/JosefinSansRegular-x3LYV.ttf"
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

typedef struct SE_Text_Renderer {
    FT_Library library;
    FT_Face face; // the font
    SEGL_Shader_Program *shader_program;

    u32 VAO, VBO;
    bool initialised;
    
    SE_Text_Character characters[SE_TEXT_NUM_OF_GLYPHS];
} SE_Text_Renderer;

/// Initialises freetype and returns a pointer to a SE_Text_Renderer.
/// SE_Text_Renderer will be used by the rest of the procedures.
/// The returned pointer is handled by setext and should not be
/// changed or freed by the user manually. Instead call setext_deinit
/// to free all the memory used by setext and freetype
/// Returns NULL when an error has occured
SE_Text_Renderer* setext_init();

/// Deinitialises setext and frees the memory used by freetype and setext
void setext_deinit(SE_Text_Renderer *txt);

/// Loads a font.
/// Setting either width or height to zero, dynamically calculates the proper aspect ratio
/// based on the other parameter.
/// Returns 0 on success
i32 setext_load_font(SE_Text_Renderer *txt, const char *font_path, i32 width, i32 height);

/// prints the characters hashmap into the console
i32 setext_print_loaded_characters(SE_Text_Renderer *txt);

i32 setext_render_text(SE_Text_Renderer *txt, const char *string, f32 x, f32 y, f32 scale, Vec3 color, Mat4 shader_projection);
#endif // SETEXT_H