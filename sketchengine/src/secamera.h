#ifndef SE_CAMERA_H
#define SE_CAMERA_H

#include "sedefines.h"
#include "semath.h"

typedef struct SE_Camera3D {
    // @note that these two matrices are updated by se_camera3d_update_projection()
    Mat4 projection; // projection transform
    Mat4 view;       // view transform

    Vec3 position;
    f32 yaw;
    f32 pitch;
    Vec3 up;
} SE_Camera3D;

void se_camera3d_init(SE_Camera3D *cam);
Vec3 se_camera3d_get_front(const SE_Camera3D *cam);
Mat4 se_camera3d_get_view(const SE_Camera3D *cam);
    /// updates the given camera's view and projection
void se_camera3d_update_projection(SE_Camera3D *cam, i32 window_w, i32 window_h);
void se_camera3d_input(SE_Camera3D *camera, struct SE_Input *seinput, f32 delta_time);

void se_camera3d_get_raycast(SE_Camera3D *camera, SDL_Window *window, Vec3 *raycast_dir, Vec3 *raycast_origin);

#endif // SE_CAMERA_H