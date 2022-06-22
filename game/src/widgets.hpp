#pragma once

#include "entity.hpp"

struct Widget_Entity {
public:
    i32 entity = -1;
    Entities *entities = nullptr; // ! not owned

    Widget_Entity()  = default;
    ~Widget_Entity() = default;

    void construct_panel(SE_UI *ctx);
    void toggle_visibility(SE_UI *ctx);

private:
    i32 panel_index = -1;
};