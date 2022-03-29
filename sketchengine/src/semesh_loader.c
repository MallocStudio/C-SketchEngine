#include "semesh_loader.h"

#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include <stdio.h>

void semesh_load(SE_Mesh *mesh, const char *filepath) {
    // reset mesh
    memset(mesh, 0, sizeof(mesh));

    // load mesh from file
    const struct aiScene *scene = aiImportFile(filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (scene == NULL) {
        printf("ERROR: could not load load mesh from %s\n", filepath);
        return;
    }
    struct aiMesh *ai_mesh = scene->mMeshes[0];

    u32 verts_count = 0;
    u32 index_count = 0;
    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * ai_mesh->mNumVertices);
    u32       *indices = malloc(sizeof(u32) * ai_mesh->mNumFaces * 3);

    // -- vertices

    for (u32 i = 0; i < ai_mesh->mNumVertices; ++i) {
        SE_Vertex3D vertex = {0};
        vertex.position.x = ai_mesh->mVertices[i].x;
        vertex.position.y = ai_mesh->mVertices[i].y;
        vertex.position.z = ai_mesh->mVertices[i].z;
        vertex.position.w = 1;

        vertex.rgba = RGBA_WHITE;

        vertex.normal.x = ai_mesh->mNormals[i].x;
        vertex.normal.y = ai_mesh->mNormals[i].y;
        vertex.normal.z = ai_mesh->mNormals[i].z;
        vertex.normal.w = 0;

        if (ai_mesh->mTextureCoords[0] != NULL) { // if this mesh has uv mapping
            vertex.texture_coord.x = ai_mesh->mTextureCoords[0][i].x;
            vertex.texture_coord.y = ai_mesh->mTextureCoords[0][i].y;
        }

        verts[verts_count] = vertex;
        verts_count++;
    }

    // -- indices

    for (u32 i = 0; i < ai_mesh->mNumFaces; ++i) {
        // ! we triangulate on import, so every face has three vertices
        indices[index_count+0] = ai_mesh->mFaces[i].mIndices[0];
        indices[index_count+1] = ai_mesh->mFaces[i].mIndices[1];
        indices[index_count+2] = ai_mesh->mFaces[i].mIndices[2];
        index_count += 3;
    }

    semesh_generate(mesh, verts_count, verts, index_count, indices);

    // -- materials

    struct aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];

    struct aiString *texturepath = NULL

    if (AI_SUCCESS == aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, texturepath, NULL, NULL, NULL, NULL, NULL, NULL)) {
        setexture_load(&mesh->material.map_Ka, texturepath->data);
    }

    aiReleaseImport(scene);
}