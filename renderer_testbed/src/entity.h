#ifndef ENTITY_H
#define ENTITY_H
#include "sketchengine.h"

typedef struct Entity {
    /* mesh */
    bool has_mesh;
    bool should_render_mesh;
    u32 mesh_index;

    Vec3 oriantation;
    Vec3 position;
    Vec3 scale;
} Entity;

Mat4 entity_get_transform(const Entity *entity);
void entity_render(const Entity *entity, const SE_Renderer3D *renderer);
SEINLINE void entity_default(Entity *entity) {
    entity->has_mesh = false;
    entity->should_render_mesh = true;
    entity->mesh_index = 0;
    entity->oriantation = v3f(0,0,0);
    entity->position = v3f(0,0,0);
    entity->scale = v3f(1,1,1);
}
#endif //ENTITY_H