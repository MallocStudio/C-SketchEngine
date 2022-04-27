#include "serender_target.h"
///
/// Render Targets
///

static void serender_target_reset(SE_Render_Target *render_target) {
    render_target->has_depth = false;
    render_target->viewport = (Rect) {0, 0, 0, 0};
    render_target->frame_buffer = 0;
    render_target->texture = 0;
    render_target->depth_buffer = 0;
}

void serender_target_init(SE_Render_Target *render_target, const Rect viewport, const bool has_depth) {
    serender_target_reset(render_target);

    render_target->viewport = viewport;
    render_target->has_depth = has_depth;

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &render_target->frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_target->frame_buffer);

    // the texture we're going to be rendering to
    glGenTextures(1, &render_target->texture);

    glBindTexture(GL_TEXTURE_2D, render_target->texture);
    // Give an empty image to opengl (the last '0')
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.w, viewport.h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // poor filtering required
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // the depth buffer
    if (has_depth) {
        glGenTextures(1, &render_target->depth_buffer);
        glBindTexture(GL_TEXTURE_2D, render_target->depth_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewport.w, viewport.h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        f32 border_colour[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);
    }

    // -- configure our frame buffer
    // set texture as our colour attachment #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render_target->texture, 0);
    if (has_depth) glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, render_target->depth_buffer, 0);
    // set the list of draw buffers
    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers); // 1 is the size of draw_buffers

    // check for errors
    SDL_assert_always(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void serender_target_deinit(SE_Render_Target *render_target) {
    if (render_target->has_depth) {
        glDeleteRenderbuffers(1, &render_target->depth_buffer);
    }
    glDeleteTextures(1, &render_target->texture);
    glDeleteFramebuffers(1, &render_target->frame_buffer);

    serender_target_reset(render_target);
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