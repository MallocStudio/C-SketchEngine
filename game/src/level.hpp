#pragma once

#include "entity.hpp"

struct Level {
public:
    Level(){};
    ~Level(){};

    void load(const char *filepath){};
    void save(const char *filepath){};

        // @incomplete
    // void add_action();
    // void undo_action();
    // void redo_action();

};