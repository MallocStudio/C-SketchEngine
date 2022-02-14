// https://learnopengl.com/In-Practice/Text-Rendering
#ifndef SETEXT_H
#define SETEXT_H

#include "defines.h"
#include "FreeType/freetype.h"
#include "hashmap.h"
#include "serenderer.h"

#define DEFAULT_FONT_PATH "assets/fonts/Ya'ahowu/Yaahowu.ttf"
// #define DEFAULT_FONT_PATH "assets/fonts/josefin-sans-font/JosefinSansRegular-x3LYV.ttf"
#define SETEXT_SUCCESS 0 // must be zero
#define SETEXT_ERROR 1

typedef struct SE_Text {
    FT_Library library;
    FT_Face face; // the font

    u32 VAO, VBO;
    bool initialised;
    
    // a map of all generated characters // !using the hashmap library
    // <char, SE_Text_Character>
    struct hashmap *characters; 
} SE_Text;

#define NUM_OF_GLYPHS 128 // ASCII
typedef struct SE_Text_Character {
    u32 texture_id; // ID handle of the glyph texture
    i32 width;     // size of glyph
    i32 height;     // size of glyph
    i32 bearing_x;  // offset from baseline to the left of glyph
    i32 bearing_y;  // offset from baseline to the top of glyph
    u32 advance;    // offset to advance to next glyph
} SE_Text_Character;

/// Initialises freetype and returns a pointer to a SE_Text.
/// SE_Text will be used by the rest of the procedures.
/// The returned pointer is handled by setext and should not be
/// Changed or freed by the user manually. Instead call setext_deinit
/// to free all the memory used by setext and freetype
/// Returns NULL when an error has occured
SE_Text* setext_init();

/// Deinitialises setext and frees the memory used by freetype and setext
void setext_deinit(SE_Text *txt);

/// Loads a font.
/// Setting either width or height to zero, dynamically calculates the proper aspect ratio
/// based on the other parameter.
/// Returns 0 on success
i32 setext_load_font(SE_Text *txt, const char *font_path, i32 width, i32 height);

/// prints the characters hashmap into the console
i32 setext_print_loaded_characters(SE_Text *txt);

i32 setext_render_text(SE_Text *txt, const char *string, i32 x, i32 y);
#endif // SETEXT_H