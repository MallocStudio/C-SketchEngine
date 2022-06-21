#pragma once

#include "entity.hpp"

struct Level {
public:
    Level();
    ~Level();

    bool load(const char *filepath);
    bool save(const char *filepath);

        /// Add an entity to the entities array and return its index.
    u32 add_entity();

        // @incomplete
    // void add_action();
    // void undo_action();
    // void redo_action();

        /// The array of entities. This is where all the entity data lives
    Entities entities;
};