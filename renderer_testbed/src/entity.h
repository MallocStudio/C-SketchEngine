#ifndef ENTITY_H
#define ENTITY_H
#include "sketchengine.h"

typedef struct Entity {
    Mat4 transform;
    u32 mesh_index;
} Entity;

void entity_render(const Entity *entities, const SE_Renderer3D *renderer);

#endif //ENTITY_H