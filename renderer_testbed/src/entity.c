#include "entity.h"

Mat4 entity_get_transform(const Entity *entity) {
    Mat4 result = mat4_identity();
    Vec3 pos = entity->position;
    Vec3 rot = entity->oriantation;
    Vec3 scale = entity->scale;

    result = mat4_mul(result, mat4_scale(scale));
    result = mat4_mul(result, mat4_euler_xyz(rot.x, rot.y, rot.z));
    result = mat4_mul(result, mat4_translation(pos));
    return result;
}

void entity_render(const Entity *entity, const SE_Renderer3D *renderer) {
    if (entity->has_mesh) {
        serender_mesh_index(renderer, entity->mesh_index, entity_get_transform(entity));
    }
}