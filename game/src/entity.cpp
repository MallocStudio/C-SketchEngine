#include "entity.hpp"

Entities::Entities() {
    this->set_to_default();
}

Entities::~Entities() {
    this->clear();
}

void Entities::update_transforms() {
    for (u32 index = 0; index < this->count; ++index) {
        this->transform[index] = mat4_identity();
        Vec3 pos   = this->position[index];
        Vec3 rot   = this->oriantation[index];
        Vec3 scale = this->scale[index];

        this->transform[index] = mat4_mul(this->transform[index], mat4_scale(scale));
        this->transform[index] = mat4_mul(this->transform[index], mat4_euler_xyz(rot.x, rot.y, rot.z));
        this->transform[index] = mat4_mul(this->transform[index], mat4_translation(pos));
    }
}

void Entities::clear() {
    this->count = 0;
    for (u32 i = 0; i < ENTITIES_MAX; ++i) {
            //- Name
        if (this->has_name[i]) {
            sestring_deinit(&this->name[i]);
        }
    }
}

void Entities::set_to_default() {
    this->count = 0;
    for (u32 i = 0; i < ENTITIES_MAX; ++i) {
        this->has_mesh           [i] = false;
        this->should_render_mesh [i] = true;
        this->mesh_index         [i] = 0;
        this->oriantation        [i] = v3f(0,0,0);
        this->position           [i] = v3f(0,0,0);
        this->scale              [i] = v3f(1,1,1);
        this->has_name           [i] = false;
    }
}