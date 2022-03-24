#ifndef SEUI_H
#define SEUI_H

#define SE_OPENGL // compatibility for other graphics APIs
#ifdef SE_OPENGL

#include "defines.h"
#include "semath_defines.h"
#include "serenderer_opengl.h"

#endif // SE_OPENGL

/// each context is made out of multiple panels. It is the panels that hold and arrange widgets
typedef struct SEUI_Panel {

} SEUI_Panel;

typedef struct SEUI_Context {
    SEUI_Panel *current_panel; // the panel we are adding widgets to right now
    SEUI_Panel *panels; // a list of all panels inside of this context
} SEUI_Context;

void seui_init(SEUI_Context *ctx);
void seui_deinit(SEUI_Context *ctx);

/// begin a panel
void seui_panel_begin(SEUI_Context *ctx, SEUI_Panel *panel);

#endif // SEUI_H