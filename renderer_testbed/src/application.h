#ifndef APPLICATION_H
#define APPLICATION_H

#include "entity.h"
#include "sestring.h"

typedef struct Application_Panel {
    f32 light_intensity;
    RGBA colour_test;
    Vec2 light_direction;
    SE_String input_text;
    f32 left, right, bottom, top, near, far;
} Application_Panel;

SEINLINE panel_init(Application_Panel *panel) {
    panel->light_intensity = 1.0f;
    panel->colour_test = RGBA_RED;
    panel->light_direction = (Vec2) {0, -1};
    sestring_init(&panel->input_text, "enter text"); // @leak
    panel->left   = 0;
    panel->right  = 1;
    panel->bottom = 0;
    panel->top    = 1;
    panel->near   = 0;
    panel->far    = 1;
}

#define ENTITIES_MAX_COUNT 10
typedef struct Application {
    SE_Input input;
    SE_Camera3D camera;

    SE_Renderer3D renderer;

    SDL_Window *window;
    bool should_quit;

    u32 entity_count;
    Entity entities[ENTITIES_MAX_COUNT];
} Application;

void app_init(Application *app, SDL_Window *window);
void app_deinit(Application *app);
void app_update(Application *app);
void app_render(Application *app);
u32 app_add_entity(Application *app);

#endif // APPLICATION_H