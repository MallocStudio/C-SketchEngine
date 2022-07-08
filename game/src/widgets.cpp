#include "widgets.hpp"
#include <string>

void Widget_Entity::construct_panel(SE_UI *ctx, SE_Renderer3D *mesh_renderer) {
    if (seui_panel(ctx, "entity data")) {
        panel_index = ctx->current_panel->index;
        ctx->current_panel->docked_dir = 1; // left
            //- Entity
        if (this->entity >= 0) {
                // entity variables to display
            SE_String *name = nullptr;
            if (entities->has_name[entity]) {
                name = &entities->name[entity];
            }

            Vec3 *pos   = &entities->position[entity];
            Vec3 *rot   = &entities->oriantation[entity];
            Vec3 *scale = &entities->scale[entity];
            u32 *mesh_index = &entities->mesh_index[entity];

                //- Name
            if (name) {
                seui_panel_row(ctx, 32, 2);
                seui_label(ctx, "name:");
                seui_label(ctx, name->buffer);
            }

            {   //- Entity index
                SE_String entity_index_string;
                se_string_init_i32(&entity_index_string, entity);

                seui_panel_row(ctx, 32, 2);
                seui_label(ctx, "index:");
                // seui_label(ctx, entity_index_string.buffer);

                seui_selector(ctx, &entity, 0, entities->count-1);

                se_string_deinit(&entity_index_string);
            }

            {   //- Mesh

                seui_panel_row(ctx, 32, 2);
                seui_label(ctx, "mesh index:");
                // seui_selector(ctx, (i32*)mesh_index, 0, mesh_renderer->user_meshes_count-1);

                SE_String mesh_index_string;
                se_string_init_i32(&mesh_index_string, *mesh_index);
                seui_label(ctx, mesh_index_string.buffer);
                se_string_deinit(&mesh_index_string);
            }

            {   //- Pos, Rot, Scale
                rot->x = SEMATH_RAD2DEG(rot->x);
                rot->y = SEMATH_RAD2DEG(rot->y);
                rot->z = SEMATH_RAD2DEG(rot->z);

                seui_label_vec3(ctx, "position", pos, true);
                seui_label_vec3(ctx, "oriantation", rot, true);
                seui_label_vec3(ctx, "scale", scale, true);

                rot->x = SEMATH_DEG2RAD(rot->x);
                rot->y = SEMATH_DEG2RAD(rot->y);
                rot->z = SEMATH_DEG2RAD(rot->z);
            }
        } else {
            {   //- Entity index
                SE_String entity_index_string;
                se_string_init_i32(&entity_index_string, entity);

                seui_panel_row(ctx, 32, 2);
                seui_label(ctx, "entity index:");

                seui_selector(ctx, &entity, -1, entities->count-1);

                se_string_deinit(&entity_index_string);
            }
        }

            //- Light Data
        {
            seui_panel_row(ctx, 64, 2);
            seui_label(ctx, "light dir:");
            seui_slider2d(ctx, &light_dir);
            mesh_renderer->light_directional.direction.x = light_dir.x;
            mesh_renderer->light_directional.direction.y = light_dir.y;
            mesh_renderer->light_directional.direction.z = 0;
        }
    }
}

void Widget_Entity::toggle_visibility(SE_UI *ctx) {
    if (panel_index >= 0) {
        ctx->panels[panel_index].is_closed = !ctx->panels[panel_index].is_closed;
    }
}