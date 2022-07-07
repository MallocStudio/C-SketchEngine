#include "level.hpp"

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

u32 Level::add_entity() {
    u32 result = this->entities.count;
    this->entities.count++;
    return result;
}
