#ifndef APPLICATION_H
#define APPLICATION_H

#include "sketchengine.h"

typedef struct Application {
    SE_Input input;
    SE_Camera3D camera;

    SE_Mesh mesh;

    SDL_Window *window;
    bool should_quit;
} Application;

void app_init(Application *app, SDL_Window *window);
void app_deinit(Application *app);
void app_update(Application *app);
void app_render(Application *app);

#endif // APPLICATION_H