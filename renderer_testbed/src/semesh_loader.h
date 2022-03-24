#ifndef SEMESH_LOADER_H
#define SEMESH_LOADER_H

#include "serenderer_opengl.h"

/// loads an obj file from filepath and generates a mesh out of that
void semesh_load_obj(SE_Mesh *mesh, const char *filepath);

#endif // SEMESH_LOADER_H