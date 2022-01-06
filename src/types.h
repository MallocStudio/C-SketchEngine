#ifndef TYPES_H
#define TYPES_H

/// sdl2 types
#include "SDL.h"
#include "SDL_ttf.h"

/// primitive types
typedef float     f32;
typedef int       i32;
typedef uint32_t  u32;
typedef Uint8     u8;
typedef enum bool {
    false = 0,
    true  = 1,
} bool;
typedef SDL_Rect  Rect;
typedef SDL_FRect RectF;
typedef SDL_Point Vec2i;

/// color types
typedef struct {
    f32 r, g, b, a; // from 0 - 1
} RGBA;
SDL_Color rgba_to_sdl_color(const RGBA *rgba);

/// text and fonts
typedef struct Text { // ! MUST use the setters to generate the text again
    TTF_Font    *font;    // the font
    SDL_Surface *surface; // the sdl surface (on the CPU side)
    SDL_Texture *texture; // the texture generated by SDL (on the GPU side)
    char        *buffer;    // the text
    RGBA        color;   // the color of the text
    SDL_Renderer *renderer; // the renderer that renders this text. this does not get freed after uninit
    bool dynamic;   // whether to use dynamic methods to render this text (should we rearrange each glyph based on the passed in rect?)
} Text;
/// create a Text on the data. also generates the texture. text should not be Null
void init_text(Text *text, SDL_Renderer *renderer, const char *data, TTF_Font *font, const RGBA color);
/// init a dynamic test
void init_text_dynamic(Text *text, SDL_Renderer *renderer, const char *data, TTF_Font *font, RGBA color, SDL_Surface *surface);
/// frees text. text becomes and invalid pointer after
void uninit_text(Text *text);
/// render text
void render_text(SDL_Renderer *renderer, Text *text, int x, int y);
/// render text within the given rect
void render_text_rect(SDL_Renderer *renderer, Text *text, Rect rect);
/// set the color of the text // ! heavy operation as it creates a new surface and texture
void set_text_color(Text *text, RGBA color);
/// returns the rect of the texture based on font. note that the pos will be the ones passed in
Rect get_text_rect(Text *text, int pos_x, int pos_y);

/// pre generated glyphs to be used to arrange new words
#define GLYPHS_UNCODE_MIN 32
#define GLYPHS_UNCODE_MAX 126
#define GLYPHS_AMOUNT GLYPHS_UNCODE_MAX - GLYPHS_UNCODE_MIN
#define UNICODE_TO_GLYPH_INDEX(unicode) unicode - GLYPHS_UNCODE_MIN
#define GLYPH_INDEX_TO_UNICODE(glyph_index) glyph_index + GLYPHS_UNCODE_MIN
typedef struct Glyphs {
    SDL_Surface *glyph[GLYPHS_AMOUNT]; // 126 - 32 (Unicode glyphs) = 94
    TTF_Font *font; // not owned, the font that was used to generate the glyphs
    RGBA      rgba; // the color used to generate the texture
} Glyphs;
/// generate all the glyph textures based on the font
void init_glyphs (SDL_Renderer *renderer, Glyphs *glyphs, TTF_Font* font, RGBA rgba);
/// free all the textures generated by the glyphs
void uninit_glyphs (Glyphs *glyphs);
/// inits the given text based on the given glyphs
void glyphs_generate_text (Text *result, SDL_Renderer *renderer, Glyphs *glyphs, const char *string);
#endif // TYPES_H