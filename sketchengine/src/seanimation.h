#ifndef SE_ANIMATION_H
#define SE_ANIMATION_H

#include "sedefines.h"
#include "semath.h"
#include "sestring.h"

#define SE_ANIMATION_JOINT_MAX_CHILDREN 10
typedef struct SE_Animation_Joint {
    u32 children_count;
    SE_Animation_Joint children[SE_ANIMATION_JOINT_MAX_CHILDREN];
    i32 index; // id
    Mat4 animated_transform; // model space (in relation to model's origin)

    // note that bind refers to the original pose with no animation applied
    Mat4 local_bind_transform; // bone space
    Mat4 inverse_bind_transform;

    /* meta data */
    SE_String name;
} SE_Animation_Joint;

SEINLINE void se_set_joint(SE_Animation_Joint *joint, i32 index, SE_String name, Mat4 local_bind_transform) {
    joint->index = index;
    joint->name = name;
    joint->local_bind_transform = local_bind_transform;
}

SEINLINE void se_add_joint(SE_Animation_Joint *parent, SE_Animation_Joint child) {
    parent->children[parent->children_count] = child;
    parent->children_count++;
}

SEINLINE void se_calculate_inverse_bind_transform(SE_Animation_Joint *joint, Mat4 parent_bind_transform) {
    Mat4 bind_transform = mat4_mul(parent_bind_transform, joint->local_bind_transform);
    joint->inverse_bind_transform = mat4_inverse(bind_transform);
    for (u32 i = 0; i < joint->children_count; ++i) {
        se_calculate_inverse_bind_transform(joint->children[i], bind_transform);
    }
}

typedef struct SE_Animation_Data {
    i32 join_ids[3];
    f32 weights[3];
} SE_Animation_Data;

#endif // SE_ANIMATION_H