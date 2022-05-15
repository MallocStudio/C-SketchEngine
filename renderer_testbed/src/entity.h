#ifndef ENTITY_H
#define ENTITY_H
#include "sketchengine.h"

typedef struct Entity {
    /* mesh */
    bool has_mesh;
    u32 mesh_index;

    /* sprite */
    bool has_sprite;
    SE_Sprite sprite;

    Vec3 oriantation;
    Vec3 position;
    Vec3 scale;
} Entity;

Mat4 entity_get_transform(const Entity *entity);
void entity_render(const Entity *entity, const SE_Renderer3D *renderer);

#endif //ENTITY_H