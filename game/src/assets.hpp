#pragma once

///
/// This handles saving and loading assets to and from the disk.
/// The way asset management works is with a bunch of procedures.
/// For example, you pass in the SE_Renderer3D as a parameter and
/// the procedure handles the saving and loading of its assets.
///

#include "sketchengine.h"
#include "level.hpp"

namespace Assets {
    bool save_renderer3D(SE_Renderer3D *renderer, const char *filepath);
    bool load_renderer3D(SE_Renderer3D *renderer, const char *filepath);
    bool save_level(Level *level, const char *filepath);
    bool load_level(Level *level, const char *filepath);
};