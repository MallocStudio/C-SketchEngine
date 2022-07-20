#ifndef SE_RENDERER_3D_UTIL
#define SE_RENDERER_3D_UTIL
//! ONLY MEANT TO BE INCLUDED IN serenderer.c AND NO WHERE ELSE.

///
///     INCLUDES
///

#include "serenderer.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "sestring.h"

#include "seinput.h" // for camera
#include "stdio.h" // for file management

///
///     DEFINES
///

#define default_normal_filepath "core/textures/default_normal.png"
#define default_diffuse_filepath "core/textures/default_diffuse.png"
#define default_specular_filepath "core/textures/default_specular.png"

#define shader_filename_lit_vsd "core/shaders/3D/lit.vsd"
#define shader_filename_lit_fsd "core/shaders/3D/lit_better.fsd"

#define shader_filename_lit_header_vsd "core/shaders/3D/lit_header.vsd"
#define shader_filename_lit_footer_vsd "core/shaders/3D/lit_footer.vsd"
#define shader_filename_lit_header_fsd "core/shaders/3D/lit_header.fsd"
#define shader_filename_lit_footer_fsd "core/shaders/3D/lit_footer.fsd"

#define shader_filename_shadow_calc_directional_vsd "core/shaders/3D/shadow_calc.vsd"
#define shader_filename_shadow_calc_directional_fsd "core/shaders/3D/shadow_calc.fsd"
#define shader_filename_shadow_calc_directional_skinned_mesh_vsd "core/shaders/3D/shadow_calc_skinned_mesh.vsd"
#define shader_filename_shadow_calc_omnidir_vsd "core/shaders/3D/shadow_omni_calc.vsd"
#define shader_filename_shadow_calc_omnidir_fsd "core/shaders/3D/shadow_omni_calc.fsd"
#define shader_filename_shadow_calc_omnidir_gsd "core/shaders/3D/shadow_omni_calc.gsd"
#define shader_filename_lines_vsd "core/shaders/3D/lines.vsd"
#define shader_filename_lines_fsd "core/shaders/3D/lines.fsd"
#define shader_filename_outline_vsd "core/shaders/3D/outline.vsd"
#define shader_filename_outline_fsd "core/shaders/3D/outline.fsd"
#define shader_filename_sprite_vsd "core/shaders/3D/sprite.vsd"
#define shader_filename_sprite_fsd "core/shaders/3D/sprite.fsd"
#define shader_filename_lit_skinned_vsd "core/shaders/3D/skinned_vertex.vsd"
#define shader_filename_skeleton_vsd "core/shaders/3D/skinned_skeleton_lines.vsd"

static void debug_print_skeleton(const SE_Skeleton *skeleton, const SE_Bone_Node *parent) {
    static i32 call = 0;
    printf("%i: parent id: %i | ", call, parent->bones_info_index);
    call++;
    for (u32 i = 0; i < parent->children_count; ++i) {
        printf("%i | ", parent->children[i]);
    }
    printf("\n");

    for (u32 i = 0; i < parent->children_count; ++i) {
        debug_print_skeleton(skeleton, &skeleton->bone_nodes[parent->children[i]]);
    }
}

static void sedefault_mesh(SE_Mesh *mesh) {
    mesh->next_mesh_index = -1;
    mesh->type = SE_MESH_TYPE_NORMAL;
    mesh->skeleton = NULL;
    mesh->should_cast_shadow = true;
}

static void se_mesh_generate_skinned // same as se_mesh_generate but for skinned vertices
(SE_Mesh *mesh, u32 vert_count, const SE_Skinned_Vertex *vertices, u32 index_count, u32 *indices) {
    // generate buffers
    glGenBuffers(1, &mesh->vbo);
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->ibo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Skinned_Vertex) * vert_count, vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);

    se_assert(mesh->type == SE_MESH_TYPE_SKINNED && "mesh type was something other than skinned but we tried to generate one");

        // enable position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.position));
        // enable normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.normal));
        // enable uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.texture_coord));
        // enable tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.tangent));
        // enable bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, vert.bitangent));
        // enable bone ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, bone_ids));
        // enable bone weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(SE_Skinned_Vertex), (void*)offsetof(SE_Skinned_Vertex, bone_weights));

    mesh->element_count = index_count;
    mesh->indexed = true;
    // mesh->aabb = (AABB3D) {0}; //se_mesh_calc_aabb(vertices, vert_count);
    mesh->aabb = se_mesh_calc_aabb_skinned(vertices, vert_count);

    // unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

    /// Returns the final transformation of a given SE_Bone_Node based on its parents
static Mat4 recursive_calc_bone_transform(const SE_Skeleton *skeleton, const SE_Bone_Node *node) {
    se_assert(node->bones_info_index >= 0 && node->bones_info_index < skeleton->bone_count);
    if (node->parent >= 0) { // if we have a parent go up the tree
        Mat4 parent_transform = recursive_calc_bone_transform(skeleton, &skeleton->bone_nodes[node->parent]);
        return mat4_mul(parent_transform, node->local_transform);
    }
    return node->local_transform;
}

static Mat4 seskeleton_get_bone_final_transform(const SE_Skeleton *skeleton, i32 bone_node_index) {
    se_assert(bone_node_index >= 0 && bone_node_index < skeleton->bone_node_count);
    return recursive_calc_bone_transform(skeleton, &skeleton->bone_nodes[bone_node_index]);
}

static void recursive_generate_skeleton_verts
(const SE_Skeleton *skeleton, SE_Skinned_Vertex *verts, u32 *vert_count, u32 *indices, u32 *index_count, const SE_Bone_Node *node, Mat4 parent_transform) {
    Mat4 final_transform = node->local_transform;
    final_transform = mat4_mul(parent_transform, node->local_transform);

    verts[*vert_count].vert.position = mat4_get_translation(final_transform);
    verts[*vert_count].bone_ids[0] = node->bones_info_index;
    (*vert_count)++;

    se_assert(node->bones_info_index >= 0);
    for (u32 i = 0; i < node->children_count; ++i) {
            // draw a line between the node and the child
            // we can assign a node's id as the index because we're adding
            // one vertex for each node. If this doesn't work consider changing
            // verts[*vert_count] to verts[node->id] to be more explicit
        indices[*index_count] = node->bones_info_index;
        (*index_count)++;
        indices[*index_count] = skeleton->bone_nodes[node->children[i]].bones_info_index;
        (*index_count)++;
        recursive_generate_skeleton_verts(skeleton, verts, vert_count, indices, index_count, &skeleton->bone_nodes[node->children[i]], final_transform);
    }
}

static void recursive_generate_static_skeleton_verts
(const SE_Skeleton *skeleton, SE_Vertex3D *verts, u32 *vert_count, u32 *indices, u32 *index_count, const SE_Bone_Node *node, Mat4 parent_transform) {
    Mat4 final_transform = node->local_transform;
    final_transform = mat4_mul(parent_transform, node->local_transform);

    verts[*vert_count].position = mat4_get_translation(final_transform);
    (*vert_count)++;

    se_assert(node->bones_info_index >= 0);
    for (u32 i = 0; i < node->children_count; ++i) {
            // draw a line between the node and the child
            // we can assign a node's id as the index because we're adding
            // one vertex for each node. If this doesn't work consider changing
            // verts[*vert_count] to verts[node->id] to be more explicit
        indices[*index_count] = node->bones_info_index;
        (*index_count)++;
        indices[*index_count] = skeleton->bone_nodes[node->children[i]].bones_info_index;
        (*index_count)++;
        recursive_generate_static_skeleton_verts(skeleton, verts, vert_count, indices, index_count, &skeleton->bone_nodes[node->children[i]], final_transform);
    }
}

static void recursive_generate_skeleton_verts_as_points
(const SE_Skeleton *skeleton, SE_Skinned_Vertex *verts, u32 *vert_count, u32 *indices, u32 *index_count, const SE_Bone_Node *node, Mat4 parent_transform) {
        // the problem here is that as we come back up the recursion stack our index_count and vert_count go back to their old values
        // instead of retaining their values.

    Mat4 final_transform = node->local_transform;
    final_transform = mat4_mul(parent_transform, node->local_transform);

        // draw a point at the given bone position
    verts[*vert_count].vert.position = mat4_get_translation(final_transform);
    (*vert_count)++;
    indices[*index_count] = node->bones_info_index;
    (*index_count)++;

    se_assert(node->bones_info_index >= 0);
    for (u32 i = 0; i < node->children_count; ++i) {
        recursive_generate_skeleton_verts_as_points(skeleton, verts, vert_count, indices, index_count, &skeleton->bone_nodes[node->children[i]], final_transform);
    }
}

static void copy_ai_matrix_to_mat4(struct aiMatrix4x4 aiMat, Mat4 *mat4) {
    mat4->data[0] = aiMat.a1; mat4->data[4] = aiMat.a2; mat4->data[8]  = aiMat.a3; mat4->data[12] = aiMat.a4;
    mat4->data[1] = aiMat.b1; mat4->data[5] = aiMat.b2; mat4->data[9]  = aiMat.b3; mat4->data[13] = aiMat.b4;
    mat4->data[2] = aiMat.c1; mat4->data[6] = aiMat.c2; mat4->data[10] = aiMat.c3; mat4->data[14] = aiMat.c4;
    mat4->data[3] = aiMat.d1; mat4->data[7] = aiMat.d2; mat4->data[11] = aiMat.d3; mat4->data[15] = aiMat.d4;
}

#if 0 // no reason to implement this yet.
static void recursive_read_animated_bones(const struct aiAnimation *animation, SE_Skeleton *skeleton) {
        // read the channels / animated bones and their keyframes
    for (u32 i = 0; i < animation->mNumChannels; ++i) {
            // NOTE(Matin): If we were not able to find the bone in skeleton for some reason, add it.
            // Sometimes when animations in fbx are loaded separately they contain additional bones not
            // found in the mesh fbx file itself. Might want to investigate this.
        if () {

        }
    }
}
#endif

static void recursive_read_bone_heirarchy
(SE_Skeleton *skeleton, i32 parent_id, Mat4 parent_model_space_transform, const struct aiNode *src) {
    se_assert(src);

        // See if this node is a bone node by comparing names with the bones
        // found in the mesh (as loaded to "skeleton" previously)
    b8 found_in_bones = false;
    b8 already_exists = false;
    SE_String src_name;
    se_string_init(&src_name, src->mName.data);
    for (u32 i = 0; i < skeleton->bone_count; ++i) {
        if (se_string_compare(&skeleton->bones_info[i].name, &src_name)) {
            found_in_bones = true;
            break;
        }
    }

    if (found_in_bones) {
            // now look to see if we've already added this to bone nodes or not
        for (u32 i = 0; i < skeleton->bone_node_count; ++i) {
            if (se_string_compare(&skeleton->bone_nodes[i].name, &src_name)) {
                already_exists = true;  // we already have this in here,
                break;
            }
        }
    }

    se_string_deinit(&src_name);

        // if this node is a bone node AND it has not already been added to bone nodes, add it to the skeleton's node heirarchy
    SE_Bone_Node *new_bone_node = NULL;
    Mat4 model_space_transform_of_this_node;
    if (found_in_bones && !already_exists) {
        new_bone_node = &skeleton->bone_nodes[skeleton->bone_node_count];
        new_bone_node->bones_info_index = skeleton->bone_node_count;
        skeleton->bone_node_count++;

        new_bone_node->parent = parent_id;
        se_string_init(&new_bone_node->name, src->mName.data);
        copy_ai_matrix_to_mat4(src->mTransformation, &new_bone_node->local_transform);
            //- inverse model space transform of the bone
        model_space_transform_of_this_node = mat4_mul(parent_model_space_transform, new_bone_node->local_transform);
        new_bone_node->inverse_neutral_transform = mat4_inverse(model_space_transform_of_this_node);

            //- link this node to its parent
        if (parent_id >= 0) {
            skeleton->bone_nodes[parent_id].children[skeleton->bone_nodes[parent_id].children_count] = new_bone_node->bones_info_index;
            skeleton->bone_nodes[parent_id].children_count++;
        }

#if 1 // @debug
        printf("CREATING A NEW BONE NODE WITH ID %i\n", new_bone_node->bones_info_index);
#endif
    }

    for (u32 i = 0; i < src->mNumChildren; ++i) {
        if (new_bone_node == NULL) {
            recursive_read_bone_heirarchy(skeleton, parent_id, parent_model_space_transform, src->mChildren[i]);
        } else {
            recursive_read_bone_heirarchy(skeleton, new_bone_node->bones_info_index, model_space_transform_of_this_node, src->mChildren[i]);
        }
    }
}

    /// Extract the bone info of the given assimp mesh and ADD it to the given skeleton
static void seload_skeleton_additively
(const struct aiMesh *ai_mesh, SE_Skeleton *skeleton, SE_Skinned_Vertex *verts, u32 verts_count, const struct aiScene *scene) {
    static i32 dsajhdsdfs = 0;
    printf("reading additively for %i times\n", dsajhdsdfs);
    dsajhdsdfs++;
    {   //- extract bone info for each vertex
            // copy the bone data to skeleton
        for (i32 bone_index = 0; bone_index < ai_mesh->mNumBones; ++bone_index) {
                // add bone to skeleton
            i32 bone_id = -1;
            SE_String ai_mesh_bone_name;
            se_string_init(&ai_mesh_bone_name,ai_mesh->mBones[bone_index]->mName.data);

            b8 bone_found = false;
            for (u32 i = 0; i < skeleton->bone_count; ++i) {
                if (se_string_compare(&skeleton->bones_info[i].name, &ai_mesh_bone_name)) {
                        //- Found the bone
                    bone_found = true;
                    bone_id = skeleton->bones_info[i].id;
                    break;
                }
            }

            se_string_deinit(&ai_mesh_bone_name);

            if (!bone_found) {
                    //- copy the bone info over
                SE_Bone_Info *bone = &skeleton->bones_info[bone_index];
                bone->id = skeleton->bone_count;
                copy_ai_matrix_to_mat4(ai_mesh->mBones[bone_index]->mOffsetMatrix, &bone->offset);
                se_string_init(&bone->name, ai_mesh->mBones[bone_index]->mName.data); // @leak we never deinit this string

                bone_id = bone->id;
                skeleton->bone_count++;
            }
            se_assert(bone_id != -1);

            for (i32 weight_index = 0; weight_index < ai_mesh->mBones[bone_index]->mNumWeights; ++weight_index) {
                i32 vertex_id = ai_mesh->mBones[bone_index]->mWeights[weight_index].mVertexId;
                f32 weight = ai_mesh->mBones[bone_index]->mWeights[weight_index].mWeight;
                se_assert(vertex_id <= verts_count);

                for (i32 xxx = 0; xxx < SE_MAX_BONE_WEIGHTS; ++xxx) {
                    if (verts[vertex_id].bone_ids    [xxx] < 0) {
                        verts[vertex_id].bone_ids    [xxx] = bone_id;
                        verts[vertex_id].bone_weights[xxx] = weight;
                        break;
                    }
                }
            }
        }
        se_assert(skeleton->bone_count == ai_mesh->mNumBones);
    }


    {   //- bone node heirarchy
        recursive_read_bone_heirarchy(skeleton, -1, mat4_identity(), scene->mRootNode);
        se_assert(skeleton->bone_count == skeleton->bone_node_count); // @debug
        {   // @debug
            for (u32 i = 0; i < skeleton->bone_node_count; ++i) {
                se_assert(skeleton->bone_nodes[i].bones_info_index >= 0 && skeleton->bone_nodes[i].bones_info_index < skeleton->bone_count);
            }
        }
    }
}


    /// Populates the given bone with animation data
static void bone_animations_init(SE_Bone_Animations *bone, const struct aiNodeAnim *channel) {
    // bone->bone_node_index = bone_node_index;
    bone->position_count  = channel->mNumPositionKeys;
    bone->rotation_count  = channel->mNumRotationKeys;
    bone->scale_count     = channel->mNumScalingKeys;
    se_string_init(&bone->name, channel->mNodeName.data);

        // allocate memory for the arrays
    bone->positions = malloc(sizeof(Vec3) * bone->position_count);
    bone->rotations = malloc(sizeof(Quat) * bone->rotation_count);
    bone->scales    = malloc(sizeof(Vec3) * bone->scale_count);
    bone->position_time_stamps = malloc(sizeof(f32) * bone->position_count);
    bone->rotation_time_stamps = malloc(sizeof(f32) * bone->rotation_count);
    bone->scale_time_stamps    = malloc(sizeof(f32) * bone->scale_count);

        // copy the data over
    for (u32 i = 0; i < bone->position_count; ++i) {
        bone->positions[i] = (Vec3) {
            .x = channel->mPositionKeys[i].mValue.x,
            .y = channel->mPositionKeys[i].mValue.y,
            .z = channel->mPositionKeys[i].mValue.z
        };
        bone->position_time_stamps[i] = (f32) channel->mPositionKeys[i].mTime;
    }

    for (u32 i = 0; i < bone->rotation_count; ++i) {
        bone->rotations[i] = (Quat) {
            .x = channel->mRotationKeys[i].mValue.x,
            .y = channel->mRotationKeys[i].mValue.y,
            .z = channel->mRotationKeys[i].mValue.z,
            .w = channel->mRotationKeys[i].mValue.w
        };
        bone->rotation_time_stamps[i] = (f32) channel->mRotationKeys[i].mTime;
    }

    for (u32 i = 0; i < bone->scale_count; ++i) {
        bone->scales[i] = (Vec3) {
            .x = channel->mScalingKeys[i].mValue.x,
            .y = channel->mScalingKeys[i].mValue.y,
            .z = channel->mScalingKeys[i].mValue.z
        };
        bone->scale_time_stamps[i] = (f32) channel->mScalingKeys[i].mTime;
    }
}

static u32 add_animation_to_skeleton(SE_Skeleton *skeleton) {
    u32 anim = skeleton->animations_count;
    skeleton->animations_count++;

    skeleton->animations[anim] = NEW(SE_Skeletal_Animation);
    memset(skeleton->animations[anim], 0, sizeof(SE_Skeletal_Animation));
    return anim;
}

static void load_animation(SE_Skeleton *skeleton, const struct aiScene *scene) {
    for (u32 i = 0; i < scene->mNumAnimations; ++i) {
            // add the animation to skeleton
        u32 anim_index = add_animation_to_skeleton(skeleton);
        SE_Skeletal_Animation *anim = skeleton->animations[anim_index];
            // update the data of animation
        anim->duration = scene->mAnimations[i]->mDuration;
        anim->ticks_per_second = scene->mAnimations[i]->mTicksPerSecond;
        se_string_init(&anim->name, scene->mAnimations[i]->mName.data); // @leak we need to free anim->name (deinit)

            // load the data of each animated bone
        anim->animated_bones_count = scene->mAnimations[i]->mNumChannels;
        anim->animated_bones = malloc(sizeof(SE_Bone_Animations) * anim->animated_bones_count);
        for (u32 c = 0; c < scene->mAnimations[i]->mNumChannels; ++c) {
            SE_Bone_Animations *animated_bone = &anim->animated_bones[c];
            bone_animations_init(animated_bone, scene->mAnimations[i]->mChannels[c]);
        }
    }
}

static void ai_scene_to_mesh_save_data
(const struct aiScene *scene, SE_Save_Data_Meshes *save_data, const char *filepath) {
    save_data->meshes_count = scene->mNumMeshes;
    save_data->meshes = malloc(sizeof(SE_Mesh_Raw_Data) * save_data->meshes_count);

    SE_Skeleton *skeleton = NULL;
    b8 is_skeleton_generated = false;
        // go through every mesh in the scene and add it to "save_data"
    for (u32 i = 0; i < scene->mNumMeshes; ++i) {
        struct aiMesh *ai_mesh = scene->mMeshes[i];
        SE_Mesh_Raw_Data *mesh = &save_data->meshes[i];
        memset(mesh, 0, sizeof(SE_Mesh_Raw_Data));
        {   //- Vertices
            if (ai_mesh->mNumBones > 0) {
                //- skinned mesh

                if (!is_skeleton_generated) { // generate the skeleton only once
                    is_skeleton_generated = true;
                    skeleton = NEW (SE_Skeleton);
                    memset(skeleton, 0, sizeof(SE_Skeleton));
                }

                mesh->skinned_verts = malloc(sizeof(SE_Skinned_Vertex) * ai_mesh->mNumVertices);
                mesh->type = SE_MESH_TYPE_SKINNED;

                for (u32 i = 0; i < ai_mesh->mNumVertices; ++i) {
                    SE_Skinned_Vertex vertex = {0};
                    for (i32 j = 0; j < SE_MAX_BONE_WEIGHTS; ++j) {
                            // NOTE(Matin): we set vertex.bone_ids and vertex.bone_weights later on during the
                            // extraction of bone vertex ids and bone weights from the ai_mesh further down...
                        vertex.bone_ids[j] = -1;
                        vertex.bone_weights[j] = 0.0f;
                    }
                        // pos
                    vertex.vert.position.x = ai_mesh->mVertices[i].x;
                    vertex.vert.position.y = ai_mesh->mVertices[i].y;
                    vertex.vert.position.z = ai_mesh->mVertices[i].z;

                        // normals
                    vertex.vert.normal.x = ai_mesh->mNormals[i].x;
                    vertex.vert.normal.y = ai_mesh->mNormals[i].y;
                    vertex.vert.normal.z = ai_mesh->mNormals[i].z;

                        // tangents // @incomplete we assume we have tangent and bi-tangent (because we've passed in a flag to calculate those) investigate
                    vertex.vert.tangent.x = ai_mesh->mTangents[i].x;
                    vertex.vert.tangent.y = ai_mesh->mTangents[i].y;
                    vertex.vert.tangent.z = ai_mesh->mTangents[i].z;

                        // bi-tangents
                    vertex.vert.bitangent.x = ai_mesh->mBitangents[i].x;
                    vertex.vert.bitangent.y = ai_mesh->mBitangents[i].y;
                    vertex.vert.bitangent.z = ai_mesh->mBitangents[i].z;

                        // uvs
                    if (ai_mesh->mTextureCoords[0] != NULL) { // if this mesh has uv mapping
                        vertex.vert.texture_coord.x = ai_mesh->mTextureCoords[0][i].x;
                        vertex.vert.texture_coord.y = ai_mesh->mTextureCoords[0][i].y;
                    } else {
                        vertex.vert.texture_coord = v2f(0, 0);
                    }

                    mesh->skinned_verts[mesh->vert_count] = vertex;
                    mesh->vert_count++;
                }
            } else {
                //- normal mesh
                mesh->verts = malloc(sizeof(SE_Vertex3D) * ai_mesh->mNumVertices);
                mesh->type = SE_MESH_TYPE_NORMAL;

                    //- vertices
                for (u32 i = 0; i < ai_mesh->mNumVertices; ++i) {
                    SE_Vertex3D vertex = {0};

                    // -- pos
                    vertex.position.x = ai_mesh->mVertices[i].x;
                    vertex.position.y = ai_mesh->mVertices[i].y;
                    vertex.position.z = ai_mesh->mVertices[i].z;

                    // -- normals
                    vertex.normal.x = ai_mesh->mNormals[i].x;
                    vertex.normal.y = ai_mesh->mNormals[i].y;
                    vertex.normal.z = ai_mesh->mNormals[i].z;

                    // -- tangents
                    if (ai_mesh->mTangents) {
                        vertex.tangent.x = ai_mesh->mTangents[i].x;
                        vertex.tangent.y = ai_mesh->mTangents[i].y;
                        vertex.tangent.z = ai_mesh->mTangents[i].z;
                    }

                    // -- bi-tangents
                    if (ai_mesh->mBitangents) {
                        vertex.bitangent.x = ai_mesh->mBitangents[i].x;
                        vertex.bitangent.y = ai_mesh->mBitangents[i].y;
                        vertex.bitangent.z = ai_mesh->mBitangents[i].z;
                    }

                    // -- uvs
                    if (ai_mesh->mTextureCoords[0] != NULL) { // if this mesh has uv mapping
                        vertex.texture_coord.x = ai_mesh->mTextureCoords[0][i].x;
                        vertex.texture_coord.y = ai_mesh->mTextureCoords[0][i].y;
                    } else {
                        vertex.texture_coord = v2f(0, 0);
                    }

                    mesh->verts[mesh->vert_count] = vertex;
                    mesh->vert_count++;
                }
            }
        }

        mesh->indices = malloc(sizeof(u32) * ai_mesh->mNumFaces * 3);
            //- indices
        for (u32 i = 0; i < ai_mesh->mNumFaces; ++i) {
            // ! we triangulate on import, so every face has three vertices
            mesh->indices[mesh->index_count+0] = ai_mesh->mFaces[i].mIndices[0];
            mesh->indices[mesh->index_count+1] = ai_mesh->mFaces[i].mIndices[1];
            mesh->indices[mesh->index_count+2] = ai_mesh->mFaces[i].mIndices[2];
            mesh->index_count += 3;
        }


        {   //- materials
            // find the directory part of filepath
            SE_String filepath_string;
            se_string_init(&filepath_string, filepath);

            SE_String dir;
            se_string_init(&dir, "");

            u32 slash_index = se_string_lastof(&filepath_string, '/');
            if (slash_index == SESTRING_MAX_SIZE) {
                se_string_append(&dir, "/");
            } else if (slash_index == 0) {
                se_string_append(&dir, ".");
            } else {
                se_string_append_length(&dir, filepath, slash_index);
                se_string_append(&dir, "/");
            }

            // now add the texture path to directory
            const struct aiMaterial *ai_material = scene->mMaterials[ai_mesh->mMaterialIndex];

            SE_String diffuse_path;
            SE_String specular_path;
            SE_String normal_path;

            se_string_init(&diffuse_path, dir.buffer);
            se_string_init(&specular_path, dir.buffer);
            se_string_init(&normal_path, dir.buffer);

            struct aiString *ai_texture_path_diffuse  = NEW(struct aiString);
            struct aiString *ai_texture_path_specular = NEW(struct aiString);
            struct aiString *ai_texture_path_normal   = NEW(struct aiString);

            b8 has_diffuse  = true;
            b8 has_specular = true;
            b8 has_normal   = true;

            if (AI_SUCCESS != aiGetMaterialTexture(ai_material, aiTextureType_DIFFUSE , 0, ai_texture_path_diffuse, NULL, NULL, NULL, NULL, NULL, NULL)) {
                has_diffuse = false;
                printf("WARNING: ASSIMP unable to load dffiuse\t for mesh: '%s', report: %s\n", filepath, aiGetErrorString());
            }

            if (AI_SUCCESS != aiGetMaterialTexture(ai_material, aiTextureType_SPECULAR, 0, ai_texture_path_specular, NULL, NULL, NULL, NULL, NULL, NULL)) {
                has_specular = false;
                printf("WARNING: ASSIMP unable to load specular\t for mesh: '%s', report: %s\n", filepath, aiGetErrorString());
            }

            if (AI_SUCCESS != aiGetMaterialTexture(ai_material, aiTextureType_NORMALS , 0, ai_texture_path_normal, NULL, NULL, NULL, NULL, NULL, NULL)) {
                has_normal = false;
                printf("WARNING: ASSIMP unable to load normals\t for mesh: '%s', report: %s\n", filepath, aiGetErrorString());
            }

            struct aiColor4D ai_colour;
            // if (AI_SUCCESS == aiGetMaterialColor(ai_material, AI_MATKEY_BASE_COLOR, &ai_colour)) { // for pbr
            if (AI_SUCCESS == aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_DIFFUSE, &ai_colour)) {
                mesh->base_diffuse = (Vec4) {
                    ai_colour.r, ai_colour.g, ai_colour.b, ai_colour.a
                };
            } else {
                printf("WARNING: ASSIMP unable to load colour diffuse\t for mesh: '%s', report: %s\n", filepath, aiGetErrorString());
            }

            /* diffuse */
            if (has_diffuse) {
                se_string_append(&diffuse_path, ai_texture_path_diffuse->data);
                se_string_init(&mesh->texture_diffuse_filepath, diffuse_path.buffer);
            }
            free(ai_texture_path_diffuse);

            /* specular */
            if (has_specular) {
                se_string_append(&specular_path, ai_texture_path_specular->data);
                se_string_init(&mesh->texture_specular_filepath, specular_path.buffer);
            }
            free(ai_texture_path_specular);

            /* normal */
            if (has_normal) {
                se_string_append(&normal_path, ai_texture_path_normal->data);
                se_string_init(&mesh->texture_normal_filepath, normal_path.buffer);
            }
            free(ai_texture_path_normal);

            se_string_deinit(&diffuse_path);
            se_string_deinit(&specular_path);
            se_string_deinit(&normal_path);

            se_string_deinit(&dir);
        }

        {   //- skeleton
            if (mesh->type == SE_MESH_TYPE_SKINNED) {
                seload_skeleton_additively( ai_mesh,
                                            skeleton,
                                            mesh->skinned_verts,
                                            mesh->vert_count,
                                            scene);
                mesh->skeleton_data = skeleton;
            }
        }

        {   //- animations
            if (skeleton != NULL && scene->mNumAnimations > 0) {
                load_animation(skeleton, scene);
            }
        }

        {   //- shadow casting
            if (mesh->type == SE_MESH_TYPE_NORMAL || mesh->type == SE_MESH_TYPE_SKINNED) {
                mesh->should_cast_shadow = true;
            }
        }
    }
}

static void skeleton_deep_copy
(SE_Skeleton *dest, const SE_Skeleton *src) {
        //- Bone Info
    dest->bone_count = src->bone_count;
    for (u32 i = 0; i < dest->bone_count; ++i) {
        dest->bones_info[i].id = src->bones_info[i].id;
        dest->bones_info[i].offset = src->bones_info[i].offset;
        se_string_init(&dest->bones_info[i].name, src->bones_info[i].name.buffer);
    }

        //- Bone Nodes
    dest->bone_node_count = src->bone_node_count;
    for (u32 i = 0; i < dest->bone_node_count; ++i) {
        se_string_init(&dest->bone_nodes[i].name, src->bone_nodes[i].name.buffer);
        dest->bone_nodes[i].bones_info_index = src->bone_nodes[i].bones_info_index;

        dest->bone_nodes[i].children_count = src->bone_nodes[i].children_count;
        memcpy( dest->bone_nodes[i].children,
                src->bone_nodes[i].children,
                sizeof(i32) * dest->bone_nodes[i].children_count);

        dest->bone_nodes[i].parent = src->bone_nodes[i].parent;
        dest->bone_nodes[i].local_transform = src->bone_nodes[i].local_transform;
        dest->bone_nodes[i].inverse_neutral_transform = src->bone_nodes[i].inverse_neutral_transform;
    }

        //- Animations
    dest->animations_count = src->animations_count;
    dest->current_animation = src->current_animation;
    for (u32 i = 0; i < dest->animations_count; ++i) {
        dest->animations[i] = malloc(sizeof(SE_Skeletal_Animation));
        memset(dest->animations[i], 0, sizeof(SE_Skeletal_Animation));

        se_string_init(&dest->animations[i]->name, src->animations[i]->name.buffer);

        dest->animations[i]->animated_bones_count = src->animations[i]->animated_bones_count;
        dest->animations[i]->animated_bones = malloc(sizeof(SE_Bone_Animations) *
                                                dest->animations[i]->animated_bones_count);
        memset( dest->animations[i]->animated_bones, 0,
                sizeof(SE_Bone_Animations) *
                dest->animations[i]->animated_bones_count);

        for (u32 j = 0; j < dest->animations[i]->animated_bones_count; ++j) {
            SE_Bone_Animations *src_animated_bone = &src->animations[i]->animated_bones[j];
            SE_Bone_Animations *dest_animated_bone = &dest->animations[i]->animated_bones[j];

            se_string_init(&dest_animated_bone->name, src_animated_bone->name.buffer);

            dest_animated_bone->position_count = src_animated_bone->position_count;
            dest_animated_bone->rotation_count = src_animated_bone->rotation_count;
            dest_animated_bone->scale_count    = src_animated_bone->scale_count;

            dest_animated_bone->positions = malloc(sizeof(Vec3) * dest_animated_bone->position_count);
            dest_animated_bone->rotations = malloc(sizeof(Quat) * dest_animated_bone->rotation_count);
            dest_animated_bone->scales    = malloc(sizeof(Vec3) * dest_animated_bone->scale_count);

            dest_animated_bone->position_time_stamps = malloc(sizeof(f32) * dest_animated_bone->position_count);
            dest_animated_bone->rotation_time_stamps = malloc(sizeof(f32) * dest_animated_bone->rotation_count);
            dest_animated_bone->scale_time_stamps    = malloc(sizeof(f32) * dest_animated_bone->scale_count);

            memcpy( dest_animated_bone->positions, src_animated_bone->positions, sizeof(Vec3) *
                    dest_animated_bone->position_count);
            memcpy( dest_animated_bone->rotations, src_animated_bone->rotations, sizeof(Quat) *
                    dest_animated_bone->rotation_count);
            memcpy( dest_animated_bone->scales, src_animated_bone->scales, sizeof(Vec3) *
                    dest_animated_bone->scale_count);

            memcpy( dest_animated_bone->position_time_stamps, src_animated_bone->position_time_stamps,
                    sizeof(f32) * dest_animated_bone->position_count);
            memcpy( dest_animated_bone->rotation_time_stamps, src_animated_bone->rotation_time_stamps,
                    sizeof(f32) * dest_animated_bone->rotation_count);
            memcpy( dest_animated_bone->scale_time_stamps, src_animated_bone->scale_time_stamps,
                    sizeof(f32) * dest_animated_bone->scale_count);

        }
        dest->animations[i]->duration = src->animations[i]->duration;
        dest->animations[i]->ticks_per_second = src->animations[i]->ticks_per_second;
    }

        //- Final Pose
    memcpy(dest->final_pose, src->final_pose, sizeof(Mat4) * SE_SKELETON_BONES_CAPACITY);
}

/// Assumes that the "file" is opened. This procedure does not handle closing the file.
/// Writes the given skeleton to the disk in binary mode
static void write_skeleton_to_disk_binary
(const SE_Skeleton *skeleton, FILE *file) {
        //- Bone Info
    fwrite(&skeleton->bone_count, sizeof(u32), 1, file);
    for (u32 i = 0; i < skeleton->bone_count; ++i) {
        fwrite(&skeleton->bones_info[i].id, sizeof(i32), 1, file);
        fwrite(&skeleton->bones_info[i].offset, sizeof(Mat4), 1, file);
        se_string_write_to_disk_binary(&skeleton->bones_info[i].name, file);
    }

        //- Bone Nodes
    fwrite(&skeleton->bone_node_count, sizeof(u32), 1, file);
    for (u32 i = 0; i < skeleton->bone_node_count; ++i) {
        se_string_write_to_disk_binary(&skeleton->bone_nodes[i].name, file);
        fwrite(&skeleton->bone_nodes[i].bones_info_index, sizeof(i32), 1, file);
        fwrite(&skeleton->bone_nodes[i].children_count, sizeof(u32), 1, file);
        fwrite(skeleton->bone_nodes[i].children, sizeof(i32), skeleton->bone_nodes[i].children_count, file);
        fwrite(&skeleton->bone_nodes[i].parent, sizeof(i32), 1, file);
        fwrite(&skeleton->bone_nodes[i].local_transform, sizeof(Mat4), 1, file);
        fwrite(&skeleton->bone_nodes[i].inverse_neutral_transform, sizeof(Mat4), 1, file);
    }

        //- Animations
    fwrite(&skeleton->animations_count, sizeof(u32), 1, file);
    for (u32 i = 0; i < skeleton->animations_count; ++i) {
        se_string_write_to_disk_binary(&skeleton->animations[i]->name, file);

        fwrite(&skeleton->animations[i]->animated_bones_count, sizeof(u32), 1, file);
        for (u32 j = 0; j < skeleton->animations[i]->animated_bones_count; ++j) {
            SE_Bone_Animations *animated_bone = &skeleton->animations[i]->animated_bones[j];

            se_string_write_to_disk_binary(&animated_bone->name, file);

            fwrite(&animated_bone->position_count, sizeof(u32), 1, file);
            fwrite(&animated_bone->rotation_count, sizeof(u32), 1, file);
            fwrite(&animated_bone->scale_count, sizeof(u32), 1, file);

            fwrite(animated_bone->positions, sizeof(Vec3), animated_bone->position_count, file);
            fwrite(animated_bone->rotations, sizeof(Quat), animated_bone->rotation_count, file);
            fwrite(animated_bone->scales, sizeof(Vec3), animated_bone->scale_count, file);

            fwrite(animated_bone->position_time_stamps, sizeof(f32), animated_bone->position_count, file);
            fwrite(animated_bone->rotation_time_stamps, sizeof(f32), animated_bone->rotation_count, file);
            fwrite(animated_bone->scale_time_stamps, sizeof(f32), animated_bone->scale_count, file);
        }

        fwrite(&skeleton->animations[i]->duration, sizeof(f32), 1, file);
        fwrite(&skeleton->animations[i]->ticks_per_second, sizeof(f32), 1, file);
    }
        //- Final Pose
    fwrite(skeleton->final_pose, sizeof(Mat4), SE_SKELETON_BONES_CAPACITY, file);
}

/// Assumes that the "file" is opened. This procedure does not handle closing the file.
/// Reads the given skeleton from the disk in binary mode
static void read_skeleton_from_disk_binary
(SE_Skeleton *skeleton, FILE *file) {
        //- Bone Info
    fread(&skeleton->bone_count, sizeof(u32), 1, file);
    for (u32 i = 0; i < skeleton->bone_count; ++i) {
        fread(&skeleton->bones_info[i].id, sizeof(i32), 1, file);
        fread(&skeleton->bones_info[i].offset, sizeof(Mat4), 1, file);
        se_string_read_from_disk_binary(&skeleton->bones_info[i].name, file);
    }

        //- Bone Nodes
    fread(&skeleton->bone_node_count, sizeof(u32), 1, file);
    for (u32 i = 0; i < skeleton->bone_node_count; ++i) {
        se_string_read_from_disk_binary(&skeleton->bone_nodes[i].name, file);
        fread(&skeleton->bone_nodes[i].bones_info_index, sizeof(i32), 1, file);
        fread(&skeleton->bone_nodes[i].children_count, sizeof(u32), 1, file);
        fread(skeleton->bone_nodes[i].children, sizeof(i32), skeleton->bone_nodes[i].children_count, file);
        fread(&skeleton->bone_nodes[i].parent, sizeof(i32), 1, file);
        fread(&skeleton->bone_nodes[i].local_transform, sizeof(Mat4), 1, file);
        fread(&skeleton->bone_nodes[i].inverse_neutral_transform, sizeof(Mat4), 1, file);
    }

        //- Animations
    fread(&skeleton->animations_count, sizeof(u32), 1, file);
    for (u32 i = 0; i < skeleton->animations_count; ++i) {
        skeleton->animations[i] = malloc(sizeof(SE_Skeletal_Animation));
        memset(skeleton->animations[i], 0, sizeof(SE_Skeletal_Animation));

        se_string_read_from_disk_binary(&skeleton->animations[i]->name, file);

        fread(&skeleton->animations[i]->animated_bones_count, sizeof(u32), 1, file);
        skeleton->animations[i]->animated_bones = malloc(
                    sizeof(SE_Bone_Animations) *
                    skeleton->animations[i]->animated_bones_count);
        memset(skeleton->animations[i]->animated_bones, 0,
                    sizeof(SE_Bone_Animations) *
                    skeleton->animations[i]->animated_bones_count);

        for (u32 j = 0; j < skeleton->animations[i]->animated_bones_count; ++j) {
            SE_Bone_Animations *animated_bone = &skeleton->animations[i]->animated_bones[j];

            se_string_read_from_disk_binary(&animated_bone->name, file);

            fread(&animated_bone->position_count, sizeof(u32), 1, file);
            fread(&animated_bone->rotation_count, sizeof(u32), 1, file);
            fread(&animated_bone->scale_count, sizeof(u32), 1, file);

            animated_bone->positions = malloc(sizeof(Vec3) * animated_bone->position_count);
            animated_bone->rotations = malloc(sizeof(Quat) * animated_bone->rotation_count);
            animated_bone->scales    = malloc(sizeof(Vec3) * animated_bone->scale_count);

            animated_bone->position_time_stamps = malloc(sizeof(f32) * animated_bone->position_count);
            animated_bone->rotation_time_stamps = malloc(sizeof(f32) * animated_bone->rotation_count);
            animated_bone->scale_time_stamps = malloc(sizeof(f32) * animated_bone->scale_count);

            fread(animated_bone->positions, sizeof(Vec3), animated_bone->position_count, file);
            fread(animated_bone->rotations, sizeof(Quat), animated_bone->rotation_count, file);
            fread(animated_bone->scales, sizeof(Vec3), animated_bone->scale_count, file);

            fread(animated_bone->position_time_stamps, sizeof(f32), animated_bone->position_count, file);
            fread(animated_bone->rotation_time_stamps, sizeof(f32), animated_bone->rotation_count, file);
            fread(animated_bone->scale_time_stamps, sizeof(f32), animated_bone->scale_count, file);
        }

        fread(&skeleton->animations[i]->duration, sizeof(f32), 1, file);
        fread(&skeleton->animations[i]->ticks_per_second, sizeof(f32), 1, file);
    }
        //- Final Pose
    fread(skeleton->final_pose, sizeof(Mat4), SE_SKELETON_BONES_CAPACITY, file);
}

//// ANIMATION BONES ////

    /// Gets the normalised value of 'amount' for lerp and slerp
static f32 get_scale_factor(f32 last_time_stamp, f32 next_time_stamp, f32 animation_time) {
    f32 mid_way_length = animation_time - last_time_stamp;
    f32 frame_delta    = next_time_stamp - last_time_stamp;
    return mid_way_length / frame_delta;
}

    /// Returns the interpolated translation of the bone at the given animation time
static Mat4 interpolate_bone_pos(SE_Bone_Animations *bone, f32 animation_time) {
    if (bone->position_count == 0) return mat4_identity();
    if (bone->position_count == 1) {
        return mat4_translation(bone->positions[0]);
    }

        // find index 0 and 1
    i32 index_0;
    for (u32 i = 0; i < bone->position_count - 1; ++i) {
        if (animation_time < bone->position_time_stamps[i + 1]) {
            index_0 = i;
            break;
        }
    }
    i32 index_1 = index_0 + 1;

        // interpolate
    f32 scale_factor = get_scale_factor(bone->position_time_stamps[index_0], bone->position_time_stamps[index_1], animation_time);
    Vec3 interpolated_pos = vec3_lerp(bone->positions[index_0], bone->positions[index_1], scale_factor);
    return mat4_translation(interpolated_pos);
}

    /// Returns the interpolated rotation of the bone at the given animation time
static Mat4 interpolate_bone_rot(SE_Bone_Animations *bone, f32 animation_time) {
    if (bone->rotation_count == 0) return mat4_identity();
    if (bone->rotation_count == 1) {
        Quat rot = quat_normalize(bone->rotations[0]);
        return quat_to_mat4(rot);
    }

        // find index 0 and 1
    i32 index_0;
    for (u32 i = 0; i < bone->rotation_count - 1; ++i) {
        if (animation_time < bone->rotation_time_stamps[i + 1]) {
            index_0 = i;
            break;
        }
    }
    i32 index_1 = index_0 + 1;

        // interpolate
    f32 scale_factor = get_scale_factor(bone->rotation_time_stamps[index_0], bone->rotation_time_stamps[index_1], animation_time);
    Quat interpolated_rot = quat_slerp(bone->rotations[index_0], bone->rotations[index_1], scale_factor);
    interpolated_rot = quat_normalize(interpolated_rot);
    // return quat_to_mat4(interpolated_rot);
    return quat_to_rotation_matrix(interpolated_rot, v3f(0,0,0));
}

    /// Returns the interpolated scale of the bone at the given animation time
static Mat4 interpolate_bone_scale(SE_Bone_Animations *bone, f32 animation_time) {
    if (bone->scale_count == 0) return mat4_identity();
    if (bone->scale_count == 1) {
        return mat4_scale(bone->scales[0]);
    }

        // find index 0 and 1
    i32 index_0;
    for (u32 i = 0; i < bone->scale_count - 1; ++i) {
        if (animation_time < bone->scale_time_stamps[i + 1]) {
            index_0 = i;
            break;
        }
    }
    i32 index_1 = index_0 + 1;

        // interpolate
    f32 scale_factor = get_scale_factor(bone->scale_time_stamps[index_0], bone->scale_time_stamps[index_1], animation_time);
    Vec3 interpolated_scale = vec3_lerp(bone->scales[index_0], bone->scales[index_1], scale_factor);
    return mat4_scale(interpolated_scale);
}

static Mat4 get_interpolated_bone_transform(SE_Bone_Animations *bone, f32 animation_time) {
    Mat4 translation = interpolate_bone_pos   (bone, animation_time);
    Mat4 rotation    = interpolate_bone_rot   (bone, animation_time);
    Mat4 scale       = interpolate_bone_scale (bone, animation_time);

    Mat4 result = scale;
    result = mat4_mul(result, rotation);
    result = mat4_mul(result, translation);
    return result;
}

static void bone_animations_deinit(SE_Bone_Animations *bone) {
    // bone->bone_node_index = -1;
    se_string_deinit(&bone->name);
    free(bone->positions);
    free(bone->rotations);
    free(bone->scales);
    free(bone->position_time_stamps);
    free(bone->rotation_time_stamps);
    free(bone->scale_time_stamps);
}

static void recursive_calculate_bone_pose // calculate the pose of the given bone based on the animation, do the same for its children
(SE_Skeleton *skeleton, const SE_Skeletal_Animation *animation, f32 animation_time, const SE_Bone_Node *node, Mat4 parent_transform) {
    se_assert(node->bones_info_index >= 0);

    SE_Bone_Animations *animated_bone = NULL;
    for (u32 i = 0; i < animation->animated_bones_count; ++i) {
        if (se_string_compare(&animation->animated_bones[i].name, &node->name)) { // @performance: change this from a string compare to a hash compare
            animated_bone = &animation->animated_bones[i];
            break;
        }
    }

    Mat4 final_node_transform = node->local_transform;
        //- the node transform with its parents taken into account
    if (animated_bone != NULL) {
        final_node_transform = get_interpolated_bone_transform(animated_bone, animation_time);
        final_node_transform = mat4_mul(final_node_transform, parent_transform);
    }

    se_assert(node->bones_info_index >= 0 && node->bones_info_index < skeleton->bone_count);
    i32 index = skeleton->bones_info[node->bones_info_index].id;
    Mat4 offset = skeleton->bones_info[node->bones_info_index].offset;
        // inverse neutral pose
    // final_node_transform = mat4_mul(node->inverse_neutral_transform, final_node_transform);
    skeleton->final_pose[index] = mat4_mul(offset, final_node_transform);

        // repeat for children
    for (u32 i = 0; i < node->children_count; ++i) {
        recursive_calculate_bone_pose(skeleton, animation, animation_time, &skeleton->bone_nodes[node->children[i]], final_node_transform);
    }
}

static void recursive_calc_skeleton_pose_without_animation(SE_Skeleton *skeleton) {
    for (u32 i = 0; i < SE_SKELETON_BONES_CAPACITY; ++i) {
        skeleton->final_pose[i] = seskeleton_get_bone_final_transform(skeleton, i);
    }
}

static void util_serender3d_render_set_material_uniforms_lit(SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform) {
    SE_Shader *shader = &renderer->shader_lit;
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    // the good old days when debugging:
    // material->texture_diffuse.width = 100;
    /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
    se_shader_set_uniform_mat4(shader, "model_matrix", transform);
    se_shader_set_uniform_vec3(shader, "camera_pos", renderer->current_camera->position);
    se_shader_set_uniform_mat4(shader, "light_space_matrix", renderer->light_space_matrix);

    /* material uniforms */
    se_shader_set_uniform_f32 (shader, "material.shininess", 0.1f);
    se_shader_set_uniform_i32 (shader, "material.diffuse", 0);
    se_shader_set_uniform_i32 (shader, "material.specular", 1);
    se_shader_set_uniform_i32 (shader, "material.normal", 2);
    se_shader_set_uniform_vec4(shader, "material.base_diffuse", material->base_diffuse);

    // directional light uniforms
    se_shader_set_uniform_vec3(shader, "dir_light.direction", renderer->light_directional.direction);
    se_shader_set_uniform_rgb (shader, "dir_light.ambient", renderer->light_directional.ambient);
    se_shader_set_uniform_rgb (shader, "dir_light.diffuse", renderer->light_directional.diffuse);
    se_shader_set_uniform_rgb (shader, "dir_light.specular", (RGB) {0, 0, 0});
    se_shader_set_uniform_f32 (shader, "dir_light.intensity", renderer->light_directional.intensity);
    se_shader_set_uniform_i32 (shader, "shadow_map", 3);

    // point light uniforms
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].position", i);
        se_shader_set_uniform_vec3(shader, buf, renderer->point_lights[i].position);
        SDL_snprintf(buf, 100, "point_lights[%i].ambient", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].ambient);
        SDL_snprintf(buf, 100, "point_lights[%i].diffuse", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].diffuse);
        SDL_snprintf(buf, 100, "point_lights[%i].specular", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].specular);
        SDL_snprintf(buf, 100, "point_lights[%i].constant", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].constant);
        SDL_snprintf(buf, 100, "point_lights[%i].linear", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].linear);
        SDL_snprintf(buf, 100, "point_lights[%i].quadratic", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].quadratic);
        SDL_snprintf(buf, 100, "point_lights[%i].far_plane", i);
        se_shader_set_uniform_f32 (shader, buf, 25.0f); // @temp magic value set to the projection far plane when calculating the shadow maps (cube texture)
    }
    se_shader_set_uniform_i32 (shader, "num_of_point_lights", renderer->point_lights_count);

    /* textures */
    // Note that by defaut meshes point to SE_DEFAULT_MATERIAL_INDEX, so by default it'll have
    // the default textures.
    if (material->texture_diffuse.loaded) {
        se_texture_bind(&material->texture_diffuse, 0);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_diffuse, 0);
    }

    if (material->texture_specular.loaded) {
        se_texture_bind(&material->texture_specular, 1);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_specular, 1);
    }

    if (material->texture_normal.loaded) {
        se_texture_bind(&material->texture_normal, 2);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_normal, 2);
    }

        // - Directional Shadow Map
    glActiveTexture(GL_TEXTURE0 + 3); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.texture);

        //- Omnidirectional Shadow Map
    for (u32 i = 0; i < SERENDERER3D_MAX_POINT_LIGHTS; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].shadow_map", i);
        se_shader_set_uniform_i32 (shader, buf, 4+i);
    }
        // ! NOTE: Might want to consider merging the below for loop with the above. I'm not sure which one
        // ! has what kind of a performance impact.
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        glActiveTexture(GL_TEXTURE0 + 4+i); // shadow map
        glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->point_lights[i].depth_cube_map);
    }
}

static void serender3d_render_set_material_uniforms_lines(SE_Renderer3D *renderer, Mat4 transform) {
    SE_Shader *shader = &renderer->shader_lines;
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
}

static void serender3d_render_set_material_uniforms_sprite(SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform) {
    SE_Shader *shader = &renderer->shader_sprite;
    se_shader_use(shader);

    /* always look at the camera */
    // Mat4 look_at_camera = mat4_lookat(mat4_get_translation(transform), renderer->current_camera->position, renderer->current_camera->up);
    // // transform = mat4_mul(transform, look_at_camera);
    // transform = mat4_mul(mat4_translation(mat4_get_translation(transform)), look_at_camera);
    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);

    /* material */
    se_shader_set_uniform_vec4(shader, "base_diffuse", material->base_diffuse);
    se_shader_set_uniform_i32(shader, "sprite_texture", 0);

    /* textures */
    if (material->sprite.texture.loaded) {
        se_texture_bind(&material->sprite.texture, 0);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_diffuse, 0);
    }
}

static void load_animation_from_file(SE_Skeleton *skeleton, const char *model_filepath) {
        // load scene from file
    const struct aiScene *scene = aiImportFile(model_filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (scene == NULL) {
        printf("ERROR: could not mesh from %s (%s)\n", model_filepath, aiGetErrorString());
        return;
    }

    load_animation(skeleton, scene);

    aiReleaseImport(scene);
}

static void
set_material_uniforms_skinned
(SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform, Mat4 *final_pose) {
    SE_Shader *shader = &renderer->shader_skinned_mesh;
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

     /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
    se_shader_set_uniform_mat4(shader, "model_matrix", transform);
    se_shader_set_uniform_vec3(shader, "camera_pos", renderer->current_camera->position);
    se_shader_set_uniform_mat4(shader, "light_space_matrix", renderer->light_space_matrix);

    /* material uniforms */
    se_shader_set_uniform_f32 (shader, "material.shininess", 0.1f);
    se_shader_set_uniform_i32 (shader, "material.diffuse", 0);
    se_shader_set_uniform_i32 (shader, "material.specular", 1);
    se_shader_set_uniform_i32 (shader, "material.normal", 2);
    se_shader_set_uniform_vec4(shader, "material.base_diffuse", material->base_diffuse);

    // directional light uniforms
    se_shader_set_uniform_vec3(shader, "dir_light.direction", renderer->light_directional.direction);
    se_shader_set_uniform_rgb (shader, "dir_light.ambient", renderer->light_directional.ambient);
    se_shader_set_uniform_rgb (shader, "dir_light.diffuse", renderer->light_directional.diffuse);
    se_shader_set_uniform_rgb (shader, "dir_light.specular", (RGB) {0, 0, 0});
    se_shader_set_uniform_f32 (shader, "dir_light.intensity", renderer->light_directional.intensity);
    se_shader_set_uniform_i32 (shader, "shadow_map", 3);

    // point light uniforms
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].position", i);
        se_shader_set_uniform_vec3(shader, buf, renderer->point_lights[i].position);
        SDL_snprintf(buf, 100, "point_lights[%i].ambient", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].ambient);
        SDL_snprintf(buf, 100, "point_lights[%i].diffuse", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].diffuse);
        SDL_snprintf(buf, 100, "point_lights[%i].specular", i);
        se_shader_set_uniform_rgb (shader, buf, renderer->point_lights[i].specular);
        SDL_snprintf(buf, 100, "point_lights[%i].constant", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].constant);
        SDL_snprintf(buf, 100, "point_lights[%i].linear", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].linear);
        SDL_snprintf(buf, 100, "point_lights[%i].quadratic", i);
        se_shader_set_uniform_f32 (shader, buf, renderer->point_lights[i].quadratic);
        SDL_snprintf(buf, 100, "point_lights[%i].far_plane", i);
        se_shader_set_uniform_f32 (shader, buf, 25.0f); // @temp magic value set to the projection far plane when calculating the shadow maps (cube texture)
    }
    se_shader_set_uniform_i32 (shader, "num_of_point_lights", renderer->point_lights_count);

    /* textures */
    // Note that by defaut meshes point to SE_DEFAULT_MATERIAL_INDEX, so by default it'll have
    // the default textures.
    if (material->texture_diffuse.loaded) {
        se_texture_bind(&material->texture_diffuse, 0);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_diffuse, 0);
    }

    if (material->texture_specular.loaded) {
        se_texture_bind(&material->texture_specular, 1);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_specular, 1);
    }

    if (material->texture_normal.loaded) {
        se_texture_bind(&material->texture_normal, 2);
    } else {
        se_texture_bind(&renderer->user_materials[SE_DEFAULT_MATERIAL_INDEX]->texture_normal, 2);
    }

    glActiveTexture(GL_TEXTURE0 + 3); // shadow map
    glBindTexture(GL_TEXTURE_2D, renderer->shadow_render_target.texture);

       //- Omnidirectional Shadow Map
    for (u32 i = 0; i < SERENDERER3D_MAX_POINT_LIGHTS; ++i) {
        char buf[100];
        SDL_snprintf(buf, 100, "point_lights[%i].shadow_map", i);
        se_shader_set_uniform_i32 (shader, buf, 4+i);
    }
        // ! NOTE: Might want to consider merging the below for loop with the above. I'm not sure which one
        // ! has what kind of a performance impact.
    for (u32 i = 0; i < renderer->point_lights_count; ++i) {
        glActiveTexture(GL_TEXTURE0 + 4+i); // shadow map
        glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->point_lights[i].depth_cube_map);
    }

    se_shader_set_uniform_mat4_array(shader, "bones", final_pose, SE_SKELETON_BONES_CAPACITY);
}

static void
set_material_uniforms_skinned_skeleton
(SE_Renderer3D *renderer, const SE_Material *material, Mat4 transform, Mat4 *final_pose) {
    SE_Shader *shader = &renderer->shader_skinned_mesh_skeleton;
    se_shader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

     /* vertex */
    se_shader_set_uniform_mat4(shader, "projection_view_model", pvm);
    se_shader_set_uniform_mat4(shader, "model_matrix", transform);

    /* material uniforms */
    se_shader_set_uniform_vec3 (shader, "base_diffuse", v3f(1, 0, 0));
    se_shader_set_uniform_mat4_array(shader, "bones", final_pose, SE_SKELETON_BONES_CAPACITY);
}

static void recursive_render_directional_shadow_map_for_mesh
(SE_Renderer3D *renderer, u32 mesh_index, Mat4 model_mat, Mat4 light_space_mat) {
    SE_Mesh *mesh = renderer->user_meshes[mesh_index];

    if (mesh->type == SE_MESH_TYPE_NORMAL) {
        se_shader_use(&renderer->shader_shadow_calc);
        se_shader_set_uniform_mat4(&renderer->shader_shadow_calc, "light_space_matrix", light_space_mat);
        se_shader_set_uniform_mat4(&renderer->shader_shadow_calc, "model", model_mat);
    } else
    if (mesh->type == SE_MESH_TYPE_SKINNED) {
        se_shader_use(&renderer->shader_shadow_calc_skinned_mesh);
        se_shader_set_uniform_mat4(&renderer->shader_shadow_calc_skinned_mesh, "light_space_matrix", light_space_mat);
        se_shader_set_uniform_mat4(&renderer->shader_shadow_calc_skinned_mesh, "model", model_mat);
        se_shader_set_uniform_mat4_array(&renderer->shader_shadow_calc_skinned_mesh, "bones", mesh->skeleton->final_pose, SE_SKELETON_BONES_CAPACITY);
    }

    glBindVertexArray(mesh->vao);
    if (mesh->indexed) {
        glDrawElements(GL_TRIANGLES, mesh->element_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->element_count);
    }

    if (mesh->next_mesh_index >= 0) {
        recursive_render_directional_shadow_map_for_mesh(renderer, mesh->next_mesh_index, model_mat, light_space_mat);
    }
}

#endif // SE_RENDERER_3D_UTIL