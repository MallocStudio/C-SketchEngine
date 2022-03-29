#include "semesh_loader.h"
#include "fast_obj.h"
#include <stdio.h>


void semesh_load_obj(SE_Mesh *mesh, const char *filepath) {
    fastObjMesh *obj = fast_obj_read(filepath);

    SDL_assert_always(obj != NULL && "could not load obj");
    SDL_assert_always(obj->object_count == 1 && "we only support one object per obj file");
    SDL_assert_always(obj->material_count <= 1 && "we do not support multiple materials per mesh");

    // We're figuring out what's the vertex buffer size. As in how many unique verts
    // will we have in total.
    u32 verts_size = semath_max(obj->position_count, obj->normal_count);
    verts_size = semath_max(verts_size, obj->texcoord_count);
    u32 indices_size = obj->index_count;

    // @temp temporarily set verts_size to indices_size because we're not using the index buffer properly to optimise duplicate vertices
    verts_size = indices_size;
    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * verts_size);
    u32 *indices = malloc(sizeof(u32) * indices_size);

    // @note Remember that the zero-th index and data is dummy. So we should
    // not send that to the graphics card.

    // -- generate all unique verts
    u32 vert_count  = 0; // The count we'll use to generate our mesh.
    u32 index_count  = 0; // The count we'll use to generate our mesh.
    for (u32 i = 0; i < obj->index_count; ++i) {
        SE_Vertex3D vertex = {0};

        // loop through every index, and match the pos, normal, and uv to one vertex at vert_count,
        // and we'll set the index at indices[index_count] and increase index_count
        u32 index_pos = obj->indices[i].p;
        u32 index_normal = obj->indices[i].n;
        u32 index_uv = obj->indices[i].t;

        // pos
        vertex.position.x = obj->positions[index_pos * 3 + 0];
        vertex.position.y = obj->positions[index_pos * 3 + 1];
        vertex.position.z = obj->positions[index_pos * 3 + 2];
        vertex.position.w = 1;

        // normal
        Vec3 normal;
        normal.x = obj->normals[index_normal * 3 + 0];
        normal.y = obj->normals[index_normal * 3 + 1];
        normal.z = obj->normals[index_normal * 3 + 2];
        vec3_normalise(&normal);
        vertex.normal.x = normal.x;
        vertex.normal.y = normal.y;
        vertex.normal.z = normal.z;
        vertex.normal.w = 0;

        // uv
        vertex.texture_coord.x = obj->texcoords[index_uv * 2 + 0];
        vertex.texture_coord.y = obj->texcoords[index_uv * 2 + 1];

        // todo check if we already have this vertex in our vertex buffer, but we don't do that yet because
        // we're not optimising and using the index buffer properly

        indices[index_count] = index_count; // @temp for now, let's not use index buffering.
        index_count++;

        verts[vert_count] = vertex;
        vert_count++;
    }

    semesh_generate(mesh, vert_count, verts, index_count, indices);

    // -- load the material
    SDL_strlcpy(mesh->material.name, obj->materials[0].name, SEMATERIAL_NAME_SIZE);
    // mesh->materials[i].Ka[0] = obj->materials[i].Ka[0];
    // mesh->materials[i].Ka[1] = obj->materials[i].Ka[1];
    // mesh->materials[i].Ka[2] = obj->materials[i].Ka[2];

    // mesh->materials[i].Kd[0] = obj->materials[i].Kd[0];
    // mesh->materials[i].Kd[1] = obj->materials[i].Kd[1];
    // mesh->materials[i].Kd[2] = obj->materials[i].Kd[2];

    // mesh->materials[i].Ks[0] = obj->materials[i].Ks[0];
    // mesh->materials[i].Ks[1] = obj->materials[i].Ks[1];
    // mesh->materials[i].Ks[2] = obj->materials[i].Ks[2];

    // mesh->materials[i].Ke[0] = obj->materials[i].Ke[0];
    // mesh->materials[i].Ke[1] = obj->materials[i].Ke[1];
    // mesh->materials[i].Ke[2] = obj->materials[i].Ke[2];

    // mesh->materials[i].Kt[0] = obj->materials[i].Kt[0];
    // mesh->materials[i].Kt[1] = obj->materials[i].Kt[1];
    // mesh->materials[i].Kt[2] = obj->materials[i].Kt[2];

    // mesh->materials[i].Ns = obj->materials[i].Ns;
    // mesh->materials[i].Ni = obj->materials[i].Ni;

    // mesh->materials[i].Tf[0] = obj->materials[i].Tf[0];
    // mesh->materials[i].Tf[1] = obj->materials[i].Tf[1];
    // mesh->materials[i].Tf[2] = obj->materials[i].Tf[2];

    // mesh->materials[i].d  = obj->materials[i].d;
    // mesh->materials[i].illum = obj->materials[i].illum;

    // -- diffuse
    if (obj->materials[0].map_Kd.path != NULL) {
        setexture_load(&mesh->material.map_Kd, obj->materials[0].map_Kd.path);
        printf("loaded texture %s\n", obj->materials[0].map_Kd.path);
    }

    free(verts);
    free(indices);

    fast_obj_destroy(obj);
}

#if 0
void semesh_load_obj(SE_Mesh *mesh, const char *filepath) {
    fastObjMesh *obj = fast_obj_read(filepath);

    // We're figuring out what's the vertex buffer size. As in how many unique verts
    // will we have in total.
    u32 verts_size = semath_max(obj->position_count, obj->normal_count);
    verts_size = semath_max(verts_size, obj->texcoord_count);
    u32 indices_size = obj->index_count;

    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * verts_size);
    u32 *indices = malloc(sizeof(u32) * indices_size);

    // @note Remember that the zero-th index and data is dummy. So we should
    // not send that to the graphics card.

    // -- generate and order our indices
    u32 index_count = 0; // The count we'll use to generate our mesh.
    for (u32 i = 0; i < obj->index_count; ++i) {
        u32 obj_index = obj->indices[i].p; // @incomplete I think that this is the problem

        indices[index_count] = obj_index;
        index_count++;
    }

    // -- generate all unique verts
    u32 vert_count  = 0; // The count we'll use to generate our mesh.
    for (u32 i = 0; i < verts_size; ++i) {
        SE_Vertex3D vertex = {0};

        // pos // @incomplete remember how verts_size is not as large as position_count
        vertex.position.x = obj->positions[(i) * 3 + 0];
        vertex.position.y = obj->positions[(i) * 3 + 1];
        vertex.position.z = obj->positions[(i) * 3 + 2];
        vertex.position.w = 1;

        // normal
        Vec3 normal;
        normal.x = obj->normals[(i) * 3 + 0];
        normal.y = obj->normals[(i) * 3 + 1];
        normal.z = obj->normals[(i) * 3 + 2];
        vec3_normalise(&normal);

        vertex.normal.x = normal.x;
        vertex.normal.y = normal.y;
        vertex.normal.z = normal.z;
        vertex.normal.w = 0;
        // uv

        verts[vert_count] = vertex;
        vert_count++;
    }

    semesh_generate(mesh, vert_count, verts, index_count, indices);

    free(verts);
    free(indices);

    fast_obj_destroy(obj);
}
#endif

#if 0
void semesh_load_obj(SE_Mesh *mesh, const char *filepath) {
    // @note that valid indices in the fastObjMesh::indices array start from 1.
    // A dummy position, normal and texture coordinate are added to the corresponding fastObjMesh arrays at
    // element 0 and then an index of 0 is used to indicate that attribute is not present at the vertex.
    // This means that users can avoid the need to test for non-present data if required as the
    // vertices will still reference a valid entry in the mesh arrays.

    fastObjMesh *obj_mesh = fast_obj_read(filepath);

    // find the biggest size, which one of {pos_count, normal_count, uv_count} do we have most of?
    u32 vert_size = semath_max(obj_mesh->position_count - 1, obj_mesh->normal_count - 1);
    vert_size = semath_max(vert_size, obj_mesh->texcoord_count - 1);

    SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * vert_size);
    memset(verts, 0, sizeof(SE_Vertex3D) * vert_size);

    // u32 index_size = obj_mesh->face_count * 3; // in this case, each face has 3 indices
    u32 index_size = obj_mesh->index_count - 1;
    u32 *indices = malloc(sizeof(u32) * index_size);
    memset(indices, 0, sizeof(u32) * index_size);

    // @note that obj_mesh->positions[0, 1, 2] are 0.0000000000000

    // -- cache positions, normals, and uvs so we can copy them over to vert array
    // pos
    Vec4 *positions = malloc(sizeof(Vec4) * obj_mesh->position_count);
    for (u32 i = 0; i < obj_mesh->position_count; i++) {
        positions[i] = (Vec4) {
            obj_mesh->positions[i*3+0],
            obj_mesh->positions[i*3+1],
            obj_mesh->positions[i*3+2],
            1,
        };
    }
    // normals // @note sometimes some faces are black, probabily because we're reading the normals wrong (currently colour is the normal in vertex shader)
    Vec4 *normals = malloc(sizeof(Vec4) * obj_mesh->normal_count);
    for (u32 i = 0; i < obj_mesh->normal_count; i++) {
        normals[i] = (Vec4) {
            obj_mesh->normals[i*3+0],
            obj_mesh->normals[i*3+1],
            obj_mesh->normals[i*3+2],
            0,
        };
    }

    // Lets populate verts
    for (u32 i = 0; i < vert_size; i++) {
        // If obj_mesh->indices[i].p for example is zero, then that data does not exist
        // at that vertex
        u32 index_pos = obj_mesh->indices[i].p;
        u32 index_nor = obj_mesh->indices[i].n;
        u32 index_uv  = obj_mesh->indices[i].t;

        // populate the verts with the correct data by indexing into positions, normals, etc
        // we need to figure out what vertex we need to
        // -- positions
        verts[i].position = positions[index_pos];
        // -- normal
        verts[i].normal = normals[index_nor];
        // -- rgba
        verts[i].rgba = RGBA_WHITE;
    }

    // Lets populate indices
    for (u32 i = 0; i < index_size; i++) {
        // -- add to indices array
        indices[i] = i;//obj_mesh->indices[i].p;
    }

#if 1 // print vertex buffer and index buffer
    printf("=== vertex buffer: %i ===\n", vert_size);
    for (u32 i = 0; i < vert_size; ++i) {
        printf("%i: vertex:\npos {%f, %f, %f, %f}\nnormal: {%f, %f, %f, %f}\n",
        i,
        verts[i].position.x, verts[i].position.y, verts[i].position.z, verts[i].position.w,
        verts[i].normal.x, verts[i].normal.y, verts[i].normal.z, verts[i].normal.w);
    }
    printf("=== index buffer: ===\n", index_size);
    for (u32 i = 0; i < index_size; ++i) {
        printf("index: %i\n", indices[i]);
    }
    printf("=== ============ ===\n");

#endif

    semesh_generate(mesh, vert_size, verts, index_size, indices);

    // -- free cached pos, normals, and texcoords
    free(positions);
    free(normals);
    // free(texcoords);

    free(verts);
    free(indices);
    fast_obj_destroy(obj_mesh);
}
#endif

// void semesh_load_obj(SE_Mesh *mesh, const char *filepath) {
//     fastObjMesh *obj_mesh = fast_obj_read(filepath);

//     u32 vert_count  = 0; // @TODO remove because in the end they are the same as vert_size
//     u32 index_count = 0; // @TODO remove because in the end they are the same as index_size

//     // find the biggest size, which one of {pos_count, normal_count, uv_count} do we have most of?
//     u32 vert_size = semath_max(obj_mesh->position_count, obj_mesh->normal_count);
//     vert_size = semath_max(vert_size, obj_mesh->texcoord_count);

//     SE_Vertex3D *verts = malloc(sizeof(SE_Vertex3D) * vert_size);
//     memset(verts, 0, sizeof(SE_Vertex3D) * vert_size);

//     u32 index_size = obj_mesh->face_count * 3; // in this case, each face has 3 indices
//     u32 *indices = malloc(sizeof(u32) * index_size);
//     memset(indices, 0, sizeof(u32) * index_size);

//     // @TODO somehow figure out how to match each pos , normal , and uv to one vertex
//     // then populate the verts array (unsolved), and populate the indices array (solved)

//     // -- vertices
//     for (u32 i = 0; i < obj_mesh->position_count; i++) {
//         verts[vert_count].position = (Vec4) {
//             obj_mesh->positions[i*3+0],
//             obj_mesh->positions[i*3+1],
//             obj_mesh->positions[i*3+2],
//             1,
//         };
//         verts[vert_count].rgba = RGBA_RED;
//         verts[vert_count].normal = (Vec4) {0, 1, 0, 0}; // @temp

//         // printf("vertex: {%i, %i, %i, %i}\n", verts[vert_count].position.x, verts[vert_count].position.y, verts[vert_count].position.z, verts[vert_count].position.w);
//         vert_count++;
//     }

//     // -- indices
//     //// the following block assumes we only have positions as our verts
//     // for (u32 i = 0; i < obj_mesh->index_count; i++) {
//     //     u32 index = obj_mesh->indices[i].p;
//     //     indices[index_count] = index;
//     //     // printf("index: %i\n", indices[index_count]);
//     //     index_count++;
//     // }
//     /// the following block tries to map pos , normals , and uvs to one vertex that shares them
//     for (u32 i = 0; i < obj_mesh->index_count; i++) {
//         u32 index_pos = obj_mesh->indices[i].p;
//         u32 index_nor = obj_mesh->indices[i].n;
//         u32 index_uv  = obj_mesh->indices[i].t;

//         u32 index = index_pos; // @TODO we can't use the positions from above because our max vert_size is much larger than positions_count. So we have to populate our verts here.
//         { // -- positions
//         // @TODO make sure that index corresponds to the correct place in the vertex array
//             verts[index].position = (Vec4) {

//             };
//         }
//         verts[index].normal = obj_mesh->normals[index_nor];
//         indices[index_count] = index;
//         // printf("index: %i\n", indices[index_count]);
//         index_count++;
//     }

//     semesh_generate(mesh, vert_count, verts, index_count, indices);

//     free(verts);
//     verts = NULL;
//     free(indices);
//     indices = NULL;
//     fast_obj_destroy(obj_mesh);
// }
