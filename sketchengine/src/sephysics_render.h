#ifndef SEPHYSICS_RENDER_H
#define SEPHYSICS_RENDER_H

#include "serenderer.h"
#include "sephysics_defines.h"

void se_render_aabb(SEGL_Line_Renderer *lines, const SE_AABB *aabb);
void se_render_circle(SEGL_Line_Renderer *lines, const SE_Circle *circle);

#endif // SEPHYSICS_RENDER_H