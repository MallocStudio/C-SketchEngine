#include "sesprite.h"
#include "GL/glew.h"
#include "stb_image.h"

///
/// TEXTURE
///

void setexture_load(SE_Texture *texture, const char *filepath) {
    texture->loaded = true;

    ubyte *image_data = stbi_load(filepath, &texture->width, &texture->height, &texture->channel_count, 0);
    if (image_data != NULL) {
        setexture_load_data(texture, image_data);
    } else {
        printf("ERROR: cannot load %s (%s)\n", filepath, stbi_failure_reason());
        texture->loaded = false;
    }
}

void setexture_load_data(SE_Texture *texture, ubyte *image_data) {
    glGenTextures(1, &texture->id);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    if (texture->channel_count == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    } else if (texture->channel_count == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    } else {
        printf("ERROR: cannot load texture, because we don't support %i channels\n", texture->channel_count);
        texture->loaded = false;
    }

    // @TODO
    // glGenerateMipmap(GL_TEXTURE_2D);

    // Instead of generating mipmaps we can set the texture param to not use mipmaps. We have to do one of these or our texture won't appear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image_data);
}

void setexture_unload(SE_Texture *texture) {
    if (texture->loaded) {
        glDeleteTextures(1, &texture->id);
    }
}

void setexture_bind(const SE_Texture *texture, u32 index) { // @TODO change index to an enum of different texture types that map to an index internally
    SDL_assert(texture->loaded == true && "texture was not loaded so we can't bind");
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void setexture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

///
/// TEXTURE ATLAS
///

void setexture_atlas_load(SE_Texture_Atlas *texture_atlas, const char *filepath, u32 columns, u32 rows) {
    setexture_load(&texture_atlas->texture, filepath);
    if (texture_atlas->texture.loaded) {
        texture_atlas->columns = columns;
        texture_atlas->rows = rows;
    }
}

void setexture_atlas_unload(SE_Texture_Atlas *texture_atlas) {
    setexture_unload(&texture_atlas->texture);
    texture_atlas->columns = 0;
    texture_atlas->rows = 0;
}

void setexture_atlas_bind(SE_Texture_Atlas *texture_atlas) {
    setexture_bind(&texture_atlas->texture, 0);
}

void setexture_atlas_unbind() {
    setexture_unbind();
}

/// SPRITES

bool sesprite_load(SE_Sprite *sprite, const char *filepath) {
    return false;
}

void sesprite_deinit(SE_Sprite *sprite) {

}