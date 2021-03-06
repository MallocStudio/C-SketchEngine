#include "entity.hpp"

static AABB3D transform_aabb3d(AABB3D aabb, Vec3 pos, Vec3 scale) {
    aabb.min = vec3_mul(aabb.min, scale);
    aabb.max = vec3_mul(aabb.max, scale);
    aabb.min = vec3_add(aabb.min, pos);
    aabb.max = vec3_add(aabb.max, pos);
    return aabb;
}

Entities::Entities() {
    this->set_to_default();
}

Entities::~Entities() {
    this->clear();
}

void Entities::update(SE_Renderer3D *renderer, f32 delta_time) {
    for (u32 i = 0; i < this->count; ++i) {
            // @temp
        // if (this->has_shader[i]) {
        //     static f32 time = 0;
        //     time += delta_time;
        //     this->position[i].y = 0.5f * se_math_sin(time) + 1;
        // }
            //- Transforms
        this->transform[i] = mat4_identity();
        Vec3 pos   = this->position[i];
        Vec3 rot   = this->oriantation[i];
        Vec3 scale = this->scale[i];

        this->transform[i] = mat4_mul(this->transform[i], mat4_scale(scale));
        this->transform[i] = mat4_mul(this->transform[i], mat4_euler_xyz(rot.x, rot.y, rot.z));
        this->transform[i] = mat4_mul(this->transform[i], mat4_translation(pos));

            //- AABB
        if (this->has_mesh[i]) {
            this->aabb[i] = renderer->user_meshes[this->mesh_index[i]]->aabb;
        }

        this->aabb_transformed[i] = transform_aabb3d(this->aabb[i], this->position[i], this->scale[i]);

            //- Update Point Light Pos
        if (this->has_light[i]) {
            renderer->point_lights[this->light_index[i]].position = this->position[i];
        }

            //- Custom Shader
        // if (this->has_shader[i]) {
        //     SE_Shader *shader = renderer->user_shaders[this->shader_index[i]];
        //     se_shader_use(shader);
        //     static f32 time = 0;
        //     time += delta_time * 4;
        //     se_shader_set_uniform_f32(shader, "time", time);
        // }
    }
}

void Entities::render(SE_Renderer3D *renderer) {
    // opaque pass
    for (u32 i = 0; i < this->count; ++i) {
        if (this->has_mesh[i] && this->should_render_mesh[i]) {
            if (renderer->user_materials[renderer->user_meshes[this->mesh_index[i]]->material_index]->type == SE_MATERIAL_TYPE_TRANSPARENT) continue;
            se_render_mesh_index(renderer, this->mesh_index[i], this->transform[i], false);
        }
    }
    // transparent pass
    for (u32 i = 0; i < this->count; ++i) {
        if (this->has_mesh[i] && this->should_render_mesh[i]) {
            if (renderer->user_materials[renderer->user_meshes[this->mesh_index[i]]->material_index]->type != SE_MATERIAL_TYPE_TRANSPARENT) continue;
            se_render_mesh_index(renderer, this->mesh_index[i], this->transform[i], true);
        }
    }
}

void Entities::clear() {
    this->count = 0;
    for (u32 i = 0; i < ENTITIES_MAX; ++i) {
            //- Name
        if (this->has_name[i]) {
            se_string_deinit(&this->name[i]);
        }
    }
}

void Entities::set_to_default() {
    this->count = 0;
    for (u32 i = 0; i < ENTITIES_MAX; ++i) {
        this->has_mesh           [i] = false;
        this->should_render_mesh [i] = true;
        this->mesh_index         [i] = -1;  // ! this must be default to -1. We rely on it @se_render_directional_shadow_map()
        this->oriantation        [i] = v3f(0,0,0);
        this->position           [i] = v3f(0,0,0);
        this->scale              [i] = v3f(1,1,1);
        this->aabb               [i] = aabb3d_one();
        this->has_name           [i] = false;
        this->has_light          [i] = false;
        this->light_index        [i] = -1;
    }
}

///
///     PLAYER
///

Player::Player(Entities *entities, u32 index, f32 cell_size) {
    m_entities = entities;
    m_index = index;
    m_cell_size = cell_size;
}

void Player::move(Vec3 direction) {
    m_entities->position[m_index] = vec3_add(m_entities->position[m_index], direction);
}