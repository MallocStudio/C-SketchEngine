#include "level.hpp"

Level::Level() {
    // in entities constructor everything is set to default
    cell_size = 1.0f;
    m_player = nullptr;
}

Level::~Level() {
    this->clear();
    delete m_player;
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

u32 Level::get_player() {
    u32 player_index;
    if (m_player == nullptr) {
        player_index = this->add_entity();
        m_player = new Player(&entities, player_index, cell_size);
    } else {
        SE_WARNING("Tried to add a player but a player has already been defined in this level");
        player_index = m_player->m_index;
    }
    return player_index;
}