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

void secamera3d_get_raycast(SE_Camera3D *camera, SDL_Window *window, Vec3 *_raycast_dir, Vec3 *_raycast_origin) {
    i32 window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    Vec3 raycast_dir;
    Vec3 raycast_origin;
#if 0 // old method that didn't work. Might want to look into this because this way is more performant?
    {   //- Get Mouse World Pos
        Mat4 proj_view_matrix = mat4_mul(m_cameras[main_camera].view, m_cameras[main_camera].projection);
        Mat4 deprojection_world = mat4_inverse(proj_view_matrix);
        // deprojection_world = mat4_transposed(deprojection_world);
        Vec2 cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / window_w) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / window_h) * 2.0f - 1.0f;

        Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 0};
        // Vec4 mouse_pos_ndc = {cursor_pos.x, -cursor_pos.y, 0, 1};
        Vec4 mouse_pos_world = mat4_mul_vec4(deprojection_world, mouse_pos_ndc);
        cursor_pos.x = mouse_pos_world.x;
        cursor_pos.y = mouse_pos_world.y;

        raycast_origin.x = mouse_pos_world.x * 10;
        raycast_origin.z = mouse_pos_world.y * 10;
        raycast_origin.y = m_cameras[main_camera].position.z;
    }
#endif
    {   //- Get Mouse World Pos
        Mat4 invert_proj = mat4_inverse(camera->projection);
        Vec2 cursor_pos = get_mouse_pos(NULL, NULL);
        cursor_pos.x = (cursor_pos.x / window_w) * 2.0f - 1.0f;
        cursor_pos.y = (cursor_pos.y / window_h) * 2.0f - 1.0f;

        Vec4 clip_coord = {cursor_pos.x, -cursor_pos.y, -1, 1};

        Vec4 eye_coord = mat4_mul_vec4(invert_proj, clip_coord);
        eye_coord.z = -1;
        eye_coord.w = 0;

        Mat4 invert_view = mat4_inverse(camera->view);
        Vec4 world_pos = mat4_mul_vec4(invert_view, eye_coord);
        Vec3 ray = v3f(world_pos.x, world_pos.y, world_pos.z);
        vec3_normalise(&ray);

        raycast_dir = ray;
        raycast_origin.x = camera->position.x;
        raycast_origin.y = camera->position.y;
        raycast_origin.z = camera->position.z;
    }

    *_raycast_dir    = raycast_dir;
    *_raycast_origin = raycast_origin;
}