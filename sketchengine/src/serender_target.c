#include "serender_target.h"
///
/// Render Targets
///
#if 0
void serender_target_init_ext(SE_Render_Target *render_target, SE_Render_Target_Config config) {
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &render_target->frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_target->frame_buffer);
    render_target->config = config;

    // -- configure our frame buffer
    render_target->viewport = (Rect) {0, 0, config.size.x, config.size.y};

    if (config.has_colour) {
        // the texture we're going to be rendering to
        glGenTextures(1, &render_target->texture);
        glBindTexture(GL_TEXTURE_2D, render_target->texture);
        // Give an empty image to opengl (the last '0')
        glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, config.size.x, config.size.y, 0, config.format, config.type, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap_t);
        f32 border_colour[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);

        // set texture as our colour attachment #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_target->texture, 0);
        // set the list of draw buffers
        GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, draw_buffers); // 1 is the size of draw_buffers
    }

    // the depth buffer
    if (config.has_depth) {
        glGenTextures(1, &render_target->depth_buffer);
        glBindTexture(GL_TEXTURE_2D, render_target->depth_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, config.size.x, config.size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap_t);
        f32 border_colour[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, render_target->depth_buffer, 0);
    }


    // check for errors
    SDL_assert_always(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void serender_target_init(SE_Render_Target *render_target, const Rect viewport, const b8 has_colour, const b8 has_depth) {
    SE_Render_Target_Config config;
    config.size = v2f(viewport.w, viewport.h);
    config.has_colour = has_colour;
    config.has_depth = has_depth;
    config.internal_format = GL_RGB;
    config.format = GL_RGB;
    config.type = GL_UNSIGNED_BYTE;
    config.mag_filter = GL_NEAREST;
    config.min_filter = GL_NEAREST;
    config.wrap_s = GL_CLAMP_TO_BORDER;
    config.wrap_t = GL_CLAMP_TO_BORDER;
    // config.wrap_s = GL_CLAMP_TO_EDGE;
    // config.wrap_t = GL_CLAMP_TO_EDGE;
    serender_target_init_ext(render_target, config);
}

void se_render_target_init_hdr(SE_Render_Target *render_target, const Rect viewport) {
    SE_Render_Target_Config config;
    config.size = v2f(viewport.w, viewport.h);
    config.has_colour = true;
    config.has_depth  = true;
    config.internal_format = GL_RGB16F;
    config.format = GL_RGB;
    config.type = GL_FLOAT;
    config.mag_filter = GL_LINEAR;
    config.min_filter = GL_LINEAR;
    // config.mag_filter = GL_NEAREST;
    // config.min_filter = GL_NEAREST;
    config.wrap_s = GL_CLAMP_TO_EDGE;
    config.wrap_t = GL_CLAMP_TO_EDGE;
    serender_target_init_ext(render_target, config);
}

void se_render_target_resize(SE_Render_Target *render_target, Rect new_viewport) {
    render_target->viewport = new_viewport;
    render_target->config.size.x = new_viewport.w;
    render_target->config.size.y = new_viewport.h;

    const SE_Render_Target_Config config = render_target->config;
    if (render_target->config.has_colour) {
        glBindTexture(GL_TEXTURE_2D, render_target->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, config.size.x,
                    config.size.y, 0, config.format, config.type, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap_t);
        f32 border_colour[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);
    }

    // the depth buffer
    if (render_target->config.has_depth) {
        glBindTexture(GL_TEXTURE_2D, render_target->depth_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, render_target->config.size.x,
                    render_target->config.size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap_t);
        f32 border_colour[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);
    }
}
void serender_target_deinit(SE_Render_Target *render_target) {
    if (render_target->config.has_depth) {
        glDeleteRenderbuffers(1, &render_target->depth_buffer);
    }
    glDeleteTextures(1, &render_target->texture);
    glDeleteFramebuffers(1, &render_target->frame_buffer);
}

void serender_target_use(SE_Render_Target *render_target) {
    if (render_target == NULL) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, render_target->frame_buffer);
        Rect v = render_target->viewport;
        glViewport(v.x, v.y, v.w, v.h);
    }
}
#endif

void serender_target_deinit(SE_Render_Target *render_target) {
    if (render_target->has_depth) {
        glDeleteRenderbuffers(1, &render_target->depth_buffer); // @TODO investigate this call vs glDeleteTextures
    }
    glDeleteTextures(render_target->colour_buffers_count, render_target->colour_buffers);
    glDeleteFramebuffers(1, &render_target->frame_buffer);
}

void serender_target_use(SE_Render_Target *render_target) {
    if (render_target == NULL) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, render_target->frame_buffer);
        glViewport(0, 0, render_target->texture_size.x, render_target->texture_size.y);
    }
}

void serender_target_init_ext(SE_Render_Target *render_target, Vec2 size, u32 colour_count, b8 has_depth, SE_Render_Target_Config config) {
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &render_target->frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_target->frame_buffer);
    render_target->texture_size = size;
    render_target->colour_buffers_count = colour_count;
    render_target->has_depth = has_depth;

    // -- configure our frame buffer
    if (colour_count > 0) {
        // the texture we're going to be rendering to
        glGenTextures(colour_count, render_target->colour_buffers);

        GLenum *draw_buffers = malloc(sizeof(GLenum) * colour_count);
        for (u32 i = 0; i < colour_count; ++i) {
            glBindTexture(GL_TEXTURE_2D, render_target->colour_buffers[i]);
            // Give an empty image to opengl (the last '0')
            glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, size.x, size.y, 0, config.format, config.type, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap);

            f32 border_colour[] = {1.0f, 1.0f, 1.0f, 1.0f}; // (maybe necessary for shadow mapping if I recall correctly?)
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);

            // set texture as our colour attachment #0
            draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, render_target->colour_buffers[i], 0);
        }

        // set the list of draw buffers
        glDrawBuffers(colour_count, draw_buffers); // 1 is the size of draw_buffers
        free(draw_buffers);
    }

    // the depth buffer
    if (has_depth) {
        glGenTextures(1, &render_target->depth_buffer);
        glBindTexture(GL_TEXTURE_2D, render_target->depth_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap);

        f32 border_colour[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, render_target->depth_buffer, 0);
    }

    // check for errors
    SDL_assert_always(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void serender_target_init(SE_Render_Target *render_target, Vec2 size, u32 colour_count, b8 has_depth) {
    SE_Render_Target_Config config;
    config.internal_format = GL_RGB;
    config.format = GL_RGB;
    config.type = GL_UNSIGNED_BYTE;
    config.filter = GL_NEAREST;
    // config.wrap = GL_CLAMP_TO_BORDER;
    config.wrap = GL_CLAMP_TO_EDGE;
    serender_target_init_ext(render_target, size, colour_count, has_depth, config);
}

void se_render_target_init_hdr(SE_Render_Target *render_target, Vec2 size, u32 colour_count, b8 has_depth) {
    SE_Render_Target_Config config;
    config.internal_format = GL_RGBA16F;
    config.format = GL_RGBA;
    config.type = GL_FLOAT;
    config.filter = GL_NEAREST;
    // config.wrap = GL_CLAMP_TO_BORDER;
    config.wrap = GL_CLAMP_TO_EDGE;
    serender_target_init_ext(render_target, size, colour_count, has_depth, config);
}