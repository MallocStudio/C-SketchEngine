#ifndef APPLICATION_H
#define APPLICATION_H

#include "entity.h"

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