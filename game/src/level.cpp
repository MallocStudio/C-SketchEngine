#include "level.hpp"
#include <iostream> // used for writing save files
#include <fstream>  // used for writing save files

#define LEVEL_SAVE_DATA_VERSION 1

Level::Level() {
    // in entities constructor everything is set to default
}

Level::~Level() {
    this->clear();
}

void Level::clear() {
        // free memory if required
    this->entities.clear();
        // set entity data to their default value
    this->entities.set_to_default();
}

bool Level::load(const char *filepath) {
    entities.count = 0;

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
            u32 entity = this->add_entity();
                //- name
            bool has_name;
            file >> has_name;
            entities.has_name[entity] = has_name;

            if (has_name) {
                char name[1024];
                file >> name;
                sestring_init(&entities.name[entity], name);
            }

                //- transforms
            file >> entities.oriantation[i].x;
            file >> entities.oriantation[i].y;
            file >> entities.oriantation[i].z;

            file >> entities.position[i].x;
            file >> entities.position[i].y;
            file >> entities.position[i].z;

            file >> entities.scale[i].x;
            file >> entities.scale[i].y;
            file >> entities.scale[i].z;

                //- AABB
            file >> entities.aabb[i].min.x;
            file >> entities.aabb[i].min.y;
            file >> entities.aabb[i].min.z;

            file >> entities.aabb[i].max.x;
            file >> entities.aabb[i].max.y;
            file >> entities.aabb[i].max.z;

                //- mesh data
            file >> entities.has_mesh[i];
            if (entities.has_mesh[i]) {
                file >> entities.should_render_mesh[i];
                file >> entities.mesh_index[i];
            }

                //- light data
            file >> entities.has_light[i];
            if (entities.has_light[i]) {
                file >> entities.light_index[i];
            }
        }
    }

    file.close();
    return false;
}

bool Level::save(const char *filepath) {
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
        file << entities.count << std::endl;

        for (u32 i = 0; i < entities.count; ++i) {
                //- name
            file << entities.has_name[i] << std::endl;
            if (entities.has_name[i]) {
                if (sestring_replace_space_with_underscore(&entities.name[i])) {
                    SE_WARNING("We are saving out entity's name and we found space characters. We replaced them with underscores ( _ ) because we cannot have spaces in names that are saved to files");
                }
                file << entities.name[i].buffer << std::endl;
            }

                //- transforms
            file << entities.oriantation[i].x << " ";
            file << entities.oriantation[i].y << " ";
            file << entities.oriantation[i].z << std::endl;

            file << entities.position[i].x << " ";
            file << entities.position[i].y << " ";
            file << entities.position[i].z << std::endl;

            file << entities.scale[i].x << " ";
            file << entities.scale[i].y << " ";
            file << entities.scale[i].z << std::endl;

                //- AABB
            file << entities.aabb[i].min.x << " ";
            file << entities.aabb[i].min.y << " ";
            file << entities.aabb[i].min.z << std::endl;

            file << entities.aabb[i].max.x << " ";
            file << entities.aabb[i].max.y << " ";
            file << entities.aabb[i].max.z << std::endl;

                //- mesh data
            file << entities.has_mesh[i] << std::endl;
            if (entities.has_mesh[i]) {
                file << entities.should_render_mesh[i] << std::endl;
                file << entities.mesh_index[i] << std::endl;
            }

                //- light data
            file << entities.has_light[i] << std::endl;
            if (entities.has_light[i]) {
                file << entities.light_index[i] << std::endl;
            }
        }
    }

    file.close();
    return true;
}

u32 Level::add_entity() {
    u32 result = this->entities.count;
    this->entities.count++;
    return result;
}
