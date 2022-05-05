#ifndef PANELS_H
#define PANELS_H
/* these are the panels used by the game editor */

#include "sketchengine.h"

typedef struct Panel_Entity {
    u32  entity_id;
    u32  *entity_mesh;  // not owned
    Vec3 *entity_pos;   // not owned
    Vec3 *entity_rot;   // not owned
    Vec3 *entity_scale; // not owned
} Panel_Entity;

#endif // PANELS_H