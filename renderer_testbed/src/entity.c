#include "entity.h"

void entity_render(const Entity *entity, const SE_Renderer3D *renderer) {
    serender3d_render_mesh(renderer, entity->mesh_index, entity->transform);
}