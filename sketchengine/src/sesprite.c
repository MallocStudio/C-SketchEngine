#include "sesprite.h"
#include "GL/glew.h"
#include "stb_image.h"
#include <stdio.h> // for saving file to disk

///
/// TEXTURE
///

void se_texture_load(SE_Texture *texture, const char *filepath, SE_TEXTURE_LOAD_CONFIG config_flags) {
    texture->loaded = true;

    ubyte *image_data = stbi_load(filepath, &texture->width, &texture->height, &texture->channel_count, 0);
    if (image_data != NULL) {
        se_texture_load_data(texture, image_data, config_flags);
    } else {
        printf("ERROR: cannot load %s (%s)\n", filepath, stbi_failure_reason());
        texture->loaded = false;
    }
}

void se_texture_load_data(SE_Texture *texture, ubyte *image_data, SE_TEXTURE_LOAD_CONFIG config_flags) {
    texture->loaded = true;
    glGenTextures(1, &texture->id);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    if (texture->channel_count == 3) {
        GLint internal_format = GL_RGB;

        if (config_flags & SE_TEXTURE_LOAD_CONFIG_CONVERT_TO_LINEAR_SPACE) {
            internal_format = GL_SRGB;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    } else if (texture->channel_count == 4) {
        GLint internal_format = GL_RGBA;

        if (config_flags & SE_TEXTURE_LOAD_CONFIG_CONVERT_TO_LINEAR_SPACE) {
                internal_format = GL_SRGB_ALPHA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
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

void se_texture_unload(SE_Texture *texture) {
    if (texture->loaded) {
        glDeleteTextures(1, &texture->id);
    }
}

void se_texture_bind(const SE_Texture *texture, u32 index) { // @TODO change index to an enum of different texture types that map to an index internally
    se_assert(texture->loaded == true && "texture was not loaded so we can't bind");
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void se_texture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

///
/// IMAGE
///

void se_image_load_empty(SE_Image *image, i32 width, i32 height, i32 channel_count) {
    if (image == NULL || width == 0 || height == 0 || channel_count == 0) return;
    image->loaded = true;
    image->width = width;
    image->height = height;
    image->channel_count = channel_count;
    image->data = (ubyte*) malloc(sizeof(ubyte) * width * height * channel_count);
}

void se_image_load_ext(SE_Image *image, const char *filepath, i32 channels_to_load) {
    image->loaded = true;
    image->data = NULL;
    image->data = stbi_load(filepath, &image->width, &image->height, &image->channel_count, channels_to_load);
    if (image->data != NULL) {
        // seimage_load_data(image, image->data);
        // image loaded successfully
        if (channels_to_load < image->channel_count) image->channel_count = channels_to_load;
    } else {
        printf("ERROR: cannot load %s (%s)\n", filepath, stbi_failure_reason());
        image->loaded = false;
    }
}

void se_image_load(SE_Image *image, const char *filepath) {
    se_image_load_ext(image, filepath, 0);
}

void se_image_unload(SE_Image *image) {
    image->loaded = false;
    stbi_image_free(image->data); // this is just free so works with empty image
    image->data = NULL;
    image->height = 0;
    image->width = 0;
    image->channel_count = 0;
}

void se_image_to_texture(const SE_Image *image, SE_Texture *texture) {
    if (image->loaded) {
        texture->channel_count = image->channel_count;
        texture->height = image->height;
        texture->width  = image->width;
        se_texture_load_data(texture, image->data, false);
    }
}

void se_image_blit(SE_Image *dst, const SE_Image *src, i32 x, i32 y) {
    se_image_blit_data(dst, src->data, src->width, src->height, x, y);
}

void se_image_blit_data(SE_Image *dst, ubyte *data, i32 data_width, i32 data_height, i32 x, i32 y) {
    if (data != NULL && dst->loaded) {
        for (u32 j = 0; j < data_height; j++) {
            u32 dst_y = j + y;
            if (dst_y < 0 || dst_y >= dst->height) continue;
            for (u32 i = 0; i < data_width; i++) {
                u32 dst_x = i + x;
                if (dst_x < 0 || dst_x >= dst->width)  continue;
                dst->data[dst_x + dst_y * dst->width] = data[i + j * data_width];
            }
        }
    }
}

// void seimage_to_disk(const SE_Image *image, const char *filepath) {
//     if (!image->loaded) {
//         printf("ERROR: could not save image to file (%s) because image was not loaded\n", filepath);
//         return;
//     }

//     FILE *file = fopen(filepath, "w"); // output only
//     if (file != NULL) {
//         for (u32 j = 0; j < image->height; ++j) {
//             for (u32 i = 0; i < image->width; ++i) {
//                 fprintf(file, "%u", (unsigned int) image->data[i + j * image->width]);
//             }
//         }
//     } else {
//         printf("ERROR: could not save image to file (%s) because there was an issue with openning the file\n", filepath);
//         // maybe because the file does not exist and we have to create it
//     }
// }

///
/// TEXTURE ATLAS
///

void se_texture_atlas_load(SE_Texture_Atlas *texture_atlas, const char *filepath, u32 columns, u32 rows) {
    se_texture_load(&texture_atlas->texture, filepath, true); // @incomplete add a parameter for convert_to_linear_space of texture load
    if (texture_atlas->texture.loaded) {
        texture_atlas->columns = columns;
        texture_atlas->rows = rows;
    }
}

void se_texture_atlas_unload(SE_Texture_Atlas *texture_atlas) {
    se_texture_unload(&texture_atlas->texture);
    texture_atlas->columns = 0;
    texture_atlas->rows = 0;
}

void se_texture_atlas_bind(SE_Texture_Atlas *texture_atlas) {
    se_texture_bind(&texture_atlas->texture, 0);
}

void se_texture_atlas_unbind() {
    se_texture_unbind();
}

/// SPRITES

b8 se_sprite_load(SE_Sprite *sprite, const char *filepath) {
    se_texture_load(&sprite->texture, filepath, true);
    sprite->frame = 0;
    sprite->columns = 0;
    sprite->rows = 0;
    return sprite->texture.loaded;
}

void se_sprite_unload(SE_Sprite *sprite) {
    se_texture_unload(&sprite->texture);
}