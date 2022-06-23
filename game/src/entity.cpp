#include "entity.hpp"

Entities::Entities() {
    this->set_to_default();
}

Entities::~Entities() {
    this->clear();
}

void Entities::update(SE_Renderer3D *renderer, f32 delta_time) {
    for (u32 i = 0; i < this->count; ++i) {
            //- Transforms
        this->transform[i] = mat4_identity();
        Vec3 pos   = this->position[i];
        Vec3 rot   = this->oriantation[i];
        Vec3 scale = this->scale[i];

        this->transform[i] = mat4_mul(this->transform[i], mat4_scale(scale));
        this->transform[i] = mat4_mul(this->transform[i], mat4_euler_xyz(rot.x, rot.y, rot.z));
        this->transform[i] = mat4_mul(this->transform[i], mat4_translation(pos));

            //- Update Point Light Pos
        if (this->has_light[i]) {
            renderer->point_lights[this->light_index[i]].position = this->position[i];
        }
    }
}

void Entities::render(SE_Renderer3D *renderer) {
    for (u32 i = 0; i < this->count; ++i) {
        if (this->has_mesh[i] && this->should_render_mesh[i]) {
            serender_mesh_index(renderer, this->mesh_index[i], this->transform[i]);
        }
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
        this->mesh_index         [i] = -1;
        this->oriantation        [i] = v3f(0,0,0);
        this->position           [i] = v3f(0,0,0);
        this->scale              [i] = v3f(1,1,1);
        this->has_name           [i] = false;
        this->has_light          [i] = false;
        this->light_index        [i] = -1;
    }
}