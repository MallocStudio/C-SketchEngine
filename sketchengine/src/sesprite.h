#ifndef SE_SPRITE_H
#define SE_SPRITE_H

#include "sedefines.h"
#include "semath_defines.h"

///
/// TEXTURE
///

typedef struct SE_Texture {
    u32 id;
    i32 width;
    i32 height;
    i32 channel_count;
    bool loaded;
} SE_Texture;

void setexture_load(SE_Texture *texture, const char *filepath);
void setexture_load_data(SE_Texture *texture, ubyte *data);
void setexture_unload(SE_Texture *texture);
void setexture_bind(const SE_Texture *texture, u32 index);
void setexture_unbind();

///
/// TEXTURE ATLAS
///

typedef struct SE_Texture_Atlas {
    SE_Texture texture;
    u32 columns; // number of columns
    u32 rows;    // number of rows
} SE_Texture_Atlas;

void setexture_atlas_load(SE_Texture_Atlas *texture_atlas, const char *filepath, u32 columns, u32 rows);
void setexture_atlas_unload(SE_Texture_Atlas *texture_atlas);
void setexture_atlas_bind(SE_Texture_Atlas *texture_atlas);
void setexture_atlas_unbind();


typedef struct SE_Sprite {
    /* image data */
    Vec2 size;
    /* rendering data */
    u32 vao;
    u32 vbo;
    bool loaded;
} SE_Sprite;

/// returns true on success, false on failure.
bool sesprite_load(SE_Sprite *sprite, const char *filepath);
/// must be called if sesprite_load is called to clean up opengl resources.
void sesprite_deinit(SE_Sprite *sprite);
#endif // SE_SPRITE_H