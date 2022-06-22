#pragma once

#include "sketchengine.h"

#define ENTITIES_MAX 100
    /// SOA
struct Entities {
public:
    u32 count;

        //- Transforms
    Vec3 oriantation[ENTITIES_MAX];
    Vec3 position[ENTITIES_MAX];
    Vec3 scale[ENTITIES_MAX];
    Mat4 transform[ENTITIES_MAX];   // ! does not need to be saved and loaded from files. Because it gets calculated after update_transforms is called.

        //- Mesh
    bool has_mesh[ENTITIES_MAX];
    bool should_render_mesh[ENTITIES_MAX];
    u32 mesh_index[ENTITIES_MAX];

        //- Name
    bool has_name[ENTITIES_MAX];
    SE_String name[ENTITIES_MAX];

    /// ---------
    /// PROCEDURES
    /// ---------

    Entities();
    ~Entities();

        /// Sets all entity data to their default values
        /// NOTE that this is called in the constructor
    void set_to_default();
        /// Clear out every entity data.
        /// Note that this is called in the destructor
    void clear();

        /// Calculate the transform matrix of the entity
    void update_transforms();
};