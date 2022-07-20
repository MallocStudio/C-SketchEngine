#pragma once

#include "entity.hpp"

struct Level {
public:
    Level();
    ~Level();

        /// Clear Entities (free memory) and set their values to default
    void clear();

        /// Add an entity to the entities array and return its index.
    u32 add_entity();
    u32 add_player();

        // @incomplete
    // void add_action();
    // void undo_action();
    // void redo_action();

        /// The array of entities. This is where all the entity data lives
    Entities entities;
    SE_Camera3D main_camera_settings;
    Player *m_player;
    f32 cell_size;
};