#include "secamera.h"
#include "seinput.h"

///
/// Camera
///

void secamera3d_init(SE_Camera3D *cam) {
    cam->position = vec3_zero();
    cam->yaw = 0;
    cam->pitch = 0;
    cam->up = vec3_up();
}

Vec3 secamera3d_get_front(const SE_Camera3D *cam) {
    f32 yaw = cam->yaw;
    f32 pitch = cam->pitch;
    Vec3 camera_front = {0};
    camera_front.x = semath_cos(yaw * SEMATH_DEG2RAD_MULTIPLIER) * semath_cos(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    camera_front.y = semath_sin(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    camera_front.z = semath_sin(yaw * SEMATH_DEG2RAD_MULTIPLIER) * semath_cos(pitch * SEMATH_DEG2RAD_MULTIPLIER);
    vec3_normalise(&camera_front);
    return camera_front;
}

Mat4 secamera3d_get_view(const SE_Camera3D *cam) {
    Vec3 camera_front = secamera3d_get_front(cam);
    return mat4_lookat(cam->position, vec3_add(cam->position, camera_front), cam->up);
}

void secamera3d_update_projection(SE_Camera3D *cam, i32 window_w, i32 window_h) {
    cam->view = secamera3d_get_view(cam);
    // f32 near_plane = 0.01f, far_plane = 70.5f;
    // cam->projection = mat4_ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    cam->projection = mat4_perspective(SEMATH_PI * 0.25f, window_w / (f32) window_h, 0.1f, 1000.0f);
}

void secamera3d_input(SE_Camera3D *camera, SE_Input *seinput) {
    { // movement
        i32 r       = seinput_is_key_down(seinput, SDL_SCANCODE_D) == true ? 1 : 0;
        i32 l       = seinput_is_key_down(seinput, SDL_SCANCODE_A) == true ? 1 : 0;
        i32 d       = seinput_is_key_down(seinput, SDL_SCANCODE_S) == true ? 1 : 0;
        i32 u       = seinput_is_key_down(seinput, SDL_SCANCODE_W) == true ? 1 : 0;
        i32 elevate = seinput_is_key_down(seinput, SDL_SCANCODE_E) == true ? 1 : 0;
        i32 dive    = seinput_is_key_down(seinput, SDL_SCANCODE_Q) == true ? 1 : 0;

        Vec3 input = vec3_create(r - l, d - u, elevate - dive);

        f32 camera_speed = 0.2f; // adjust accordingly

        Vec3 movement = {
            -input.x * camera_speed,
            -input.y * camera_speed,
            input.z * camera_speed,
        };

        Vec3 camera_front = secamera3d_get_front(camera);
        Vec3 camera_right = vec3_normalised(vec3_cross(camera->up, camera_front));
        Vec3 camera_up = vec3_normalised(vec3_cross(camera_front, camera_right));

        if (movement.x != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_right, movement.x));
        }
        if (movement.y != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_front, movement.y));
        }
        if (movement.z != 0) {
            camera->position = vec3_add(camera->position, vec3_mul_scalar(camera_up, movement.z));
        }
    }
    { // -- rotate camera
        u32 mouse_state = SDL_GetMouseState(NULL, NULL);
        if (mouse_state & SDL_BUTTON_RMASK) {
            seui_mouse_fps_activate(seinput);
            f32 sensitivity = 0.15f;
            f32 xoffset = seinput->mouse_screen_pos_delta.x * sensitivity;
            f32 yoffset = seinput->mouse_screen_pos_delta.y * sensitivity;

            if (semath_abs(xoffset) > sensitivity) camera->yaw += xoffset;
            if (semath_abs(yoffset) > sensitivity) camera->pitch += yoffset;
            if(camera->pitch > +89.0f) camera->pitch = +89.0f;
            if(camera->pitch < -89.0f) camera->pitch = -89.0f;
        } else {
            seui_mouse_fps_deactivate(seinput);
        }
    }
}