#include "assets.hpp"
#include <iostream> // used for writing save files
#include <fstream>  // used for writing save files

#define LEVEL_SAVE_DATA_VERSION 1

bool Assets::save_renderer3D(SE_Renderer3D *renderer, const char *filepath) {

    return true;
}

bool Assets::load_renderer3D(SE_Renderer3D *renderer, const char *filepath) {
    return true;
}

bool Assets::save_level(Level *level, const char *filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        SE_ERROR("could not open file to save:");
        SE_ERROR(filepath);
        return false;
    }

    {   //- Entities
            // version
        file << LEVEL_SAVE_DATA_VERSION << std::endl;

            // count
        file << level->entities.count << std::endl;

        for (u32 i = 0; i < level->entities.count; ++i) {
                //- name
            file << level->entities.has_name[i] << std::endl;
            if (level->entities.has_name[i]) {
                if (se_string_replace_space_with_underscore(&level->entities.name[i])) {
                    SE_WARNING("We are saving out entity's name and we found space characters. We replaced them with underscores ( _ ) because we cannot have spaces in names that are saved to files");
                }
                file << level->entities.name[i].buffer << std::endl;
            }

                //- transforms
            file << level->entities.oriantation[i].x << " ";
            file << level->entities.oriantation[i].y << " ";
            file << level->entities.oriantation[i].z << std::endl;

            file << level->entities.position[i].x << " ";
            file << level->entities.position[i].y << " ";
            file << level->entities.position[i].z << std::endl;

            file << level->entities.scale[i].x << " ";
            file << level->entities.scale[i].y << " ";
            file << level->entities.scale[i].z << std::endl;

                //- AABB
            file << level->entities.aabb[i].min.x << " ";
            file << level->entities.aabb[i].min.y << " ";
            file << level->entities.aabb[i].min.z << std::endl;

            file << level->entities.aabb[i].max.x << " ";
            file << level->entities.aabb[i].max.y << " ";
            file << level->entities.aabb[i].max.z << std::endl;

                //- mesh data
            file << level->entities.has_mesh[i] << std::endl;
            if (level->entities.has_mesh[i]) {
                file << level->entities.should_render_mesh[i] << std::endl;
                file << level->entities.mesh_index[i] << std::endl;
            }

                //- light data
            file << level->entities.has_light[i] << std::endl;
            if (level->entities.has_light[i]) {
                file << level->entities.light_index[i] << std::endl;
            }
        }
    }

    file.close();
    return true;
}

bool Assets::load_level(Level *level, const char *filepath) {
    level->entities.count = 0;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        SE_ERROR("could not open file:");
        SE_ERROR(filepath);
        return false;
    }

    {   //- Entities
            // version
        u32 version;
        file >> version;

            // count
        u32 count;
        file >> count;

        for (u32 i = 0; i < count; ++i) {
            u32 entity = level->add_entity();
                //- name
            bool has_name;
            file >> has_name;
            level->entities.has_name[entity] = has_name;

            if (has_name) {
                char name[1024];
                file >> name;
                se_string_init(&level->entities.name[entity], name);
            }

                //- transforms
            file >> level->entities.oriantation[i].x;
            file >> level->entities.oriantation[i].y;
            file >> level->entities.oriantation[i].z;

            file >> level->entities.position[i].x;
            file >> level->entities.position[i].y;
            file >> level->entities.position[i].z;

            file >> level->entities.scale[i].x;
            file >> level->entities.scale[i].y;
            file >> level->entities.scale[i].z;

                //- AABB
            file >> level->entities.aabb[i].min.x;
            file >> level->entities.aabb[i].min.y;
            file >> level->entities.aabb[i].min.z;

            file >> level->entities.aabb[i].max.x;
            file >> level->entities.aabb[i].max.y;
            file >> level->entities.aabb[i].max.z;

                //- mesh data
            file >> level->entities.has_mesh[i];
            if (level->entities.has_mesh[i]) {
                file >> level->entities.should_render_mesh[i];
                file >> level->entities.mesh_index[i];
            }

                //- light data
            file >> level->entities.has_light[i];
            if (level->entities.has_light[i]) {
                file >> level->entities.light_index[i];
            }
        }
    }

    file.close();
    return true;
}
