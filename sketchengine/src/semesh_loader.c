#include "semesh_loader.h"

#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include <stdio.h>
#include "sestring.h"

void semesh_construct(SE_Mesh *mesh, const struct aiMesh *ai_mesh, const char *filepath, const struct aiScene *scene, const char *shader_vsd, const char *shader_fsd) {
    // reset mesh
    memset(mesh, 0, sizeof(SE_Mesh));

    seshader_init_from(&mesh->material.shader, shader_vsd, shader_fsd);

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

    { // -- extract images from model

        if (scene->mNumMaterials > 0) {
            // find the directory part of filepath
            SE_String filepath_string;
            sestring_init(&filepath_string, filepath);

            SE_String dir;
            sestring_init(&dir, "");

            u32 slash_index = sestring_lastof(&filepath_string, '/');
            if (slash_index == SESTRING_MAX_SIZE) {
                sestring_append(&dir, "/");
            } else if (slash_index == 0) {
                sestring_append(&dir, ".");
            } else {
                sestring_append_length(&dir, filepath, slash_index);
                sestring_append(&dir, "/");
            }

            // now add the texture path to directory
            const struct aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
            struct aiString *texture_path = new(struct aiString);
            aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, texture_path, NULL, NULL, NULL, NULL, NULL, NULL);

            sestring_append(&dir, texture_path->data);

            setexture_load(&mesh->material.texture_diffuse, dir.buffer);

            free(texture_path);
            sestring_deinit(&filepath_string);
            sestring_deinit(&dir);
        }
    }
}

void semesh_load(SE_Mesh *mesh, const char *filepath, const char *shader_vsd, const char *shader_fsd) {
    // load mesh from file
    const struct aiScene *scene = aiImportFile(filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (scene == NULL) {
        printf("ERROR: could not load load mesh from %s\n", filepath);
        return;
    }

    for (u32 i = 0; i < scene->mNumMeshes; ++i) {
        struct aiMesh *ai_mesh = scene->mMeshes[i];
        if (i == 0) {
            semesh_construct(mesh, ai_mesh, filepath, scene, shader_vsd, shader_fsd);
        } else {
            u32 current = 0;
            SE_Mesh *current_mesh = NULL;
            while (current < i) {
                current_mesh = mesh->next_mesh;
                current++;
            }
            current_mesh = new(SE_Mesh);
            semesh_construct(current_mesh , ai_mesh, filepath, scene, shader_vsd, shader_fsd);
        }
    }

    aiReleaseImport(scene);
}