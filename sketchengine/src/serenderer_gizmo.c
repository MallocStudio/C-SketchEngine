#include "serenderer_gizmo.h"

void se_gizmo_renderer_init(SE_Gizmo_Renderer *renderer, SE_Camera3D *current_camera) {
    memset(renderer, 0, sizeof(SE_Gizmo_Renderer)); // default everything to zero
    renderer->current_camera = current_camera;

        //- Shaders
    seshader_init_from(&renderer->shader_mesh, "core/shaders/gizmo.vsd", "core/shaders/gizmo.fsd");
    seshader_init_from(&renderer->shader_sprite, "core/shaders/gizmo.vsd", "core/shaders/gizmo.fsd");
}

void se_gizmo_renderer_deinit(SE_Gizmo_Renderer *renderer) {
        //- Shaders
    seshader_deinit(&renderer->shader_mesh);
    seshader_deinit(&renderer->shader_sprite);

    for (u32 i = 0; i < renderer->shapes_count; ++i) {
        glDeleteVertexArrays(1, &renderer->shapes[i].vao);
        glDeleteBuffers(1, &renderer->shapes[i].vbo);
        glDeleteBuffers(1, &renderer->shapes[i].ibo);
    }
    renderer->shapes_count = 0;
}

static void setup_mesh_shader(SE_Gizmo_Renderer *renderer, Mat4 transform) {
    SE_Shader *shader = &renderer->shader_mesh;
    seshader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    seshader_set_uniform_mat4(shader, "projection_view_model", pvm);
}

static void setup_sprite_shader(SE_Gizmo_Renderer *renderer, Mat4 transform) {
        //@incomplete
    SE_Shader *shader = &renderer->shader_sprite;
    seshader_use(shader);

    Mat4 pvm = mat4_mul(transform, renderer->current_camera->view);
    pvm = mat4_mul(pvm, renderer->current_camera->projection);

    seshader_set_uniform_mat4(shader, "projection_view_model", pvm);
}

void se_gizmo_render_index(SE_Gizmo_Renderer *renderer, u32 shape_index, Mat4 transform) {
    se_assert(shape_index < renderer->shapes_count);
    se_gizmo_render(renderer, &renderer->shapes[shape_index], transform);
}

void se_gizmo_render(SE_Gizmo_Renderer *renderer, SE_Gizmo_Shape *shape, Mat4 transform) {
        //- Default GL State
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend mode

        //- Setup
    i32 primitive = GL_TRIANGLES;
    if (shape->type == SE_GIZMO_TYPE_MESH) {
        primitive = GL_TRIANGLES;
        setup_mesh_shader(renderer, transform);
    } else
    if (shape->type == SE_GIZMO_TYPE_LINE) {
        primitive = GL_LINES;
        glLineWidth(shape->line_width);
        setup_mesh_shader(renderer, transform);
    } else
    if (shape->type == SE_GIZMO_TYPE_POINT) {
        primitive = GL_POINTS;
        glPointSize(shape->point_size);
        setup_mesh_shader(renderer, transform);
    } else
    if (shape->type == SE_GIZMO_TYPE_SPRITE) {
        primitive = GL_TRIANGLES;
        setup_sprite_shader(renderer, transform);
    }

        //- Draw call
    glBindVertexArray(shape->vao);
    if (shape->indexed) {
        glDrawElements(primitive, shape->vert_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(primitive, 0, shape->vert_count);
    }

        //- reset
    glLineWidth(1.0f);
    glPointSize(1.0f);
    glBindVertexArray(0);
}

static AABB3D calc_aabb(const SE_Gizmo_Vertex *verts, u32 verts_count) {
    f32 xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;

    for (u32 i = 0; i < verts_count; ++i) {
        Vec3 vert_pos = verts[i].position;
        if (xmin > vert_pos.x) xmin = vert_pos.x;
        if (ymin > vert_pos.y) ymin = vert_pos.y;
        if (zmin > vert_pos.z) zmin = vert_pos.z;

        if (xmax < vert_pos.x) xmax = vert_pos.x;
        if (ymax < vert_pos.y) ymax = vert_pos.y;
        if (zmax < vert_pos.z) zmax = vert_pos.z;
    }

    AABB3D result = {(Vec3) {xmin, ymin, zmin}, (Vec3) {xmax, ymax, zmax}};
    return result;
}

static void generate_shape(SE_Gizmo_Shape *shape, SE_GIZMO_TYPES type, SE_Gizmo_Vertex *verts, u32 verts_count, u32 *indices, u32 index_count) {
        //- generate buffers
    glGenBuffers(1, &shape->vbo);
    glGenVertexArrays(1, &shape->vao);
    glGenBuffers(1, &shape->ibo);

    glBindVertexArray(shape->vao);
    glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->ibo);

        //- fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(SE_Gizmo_Vertex) * verts_count, verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);

        //- enable position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SE_Gizmo_Vertex), (void*)offsetof(SE_Gizmo_Vertex, position));
        //- enable colour
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, TYPEOF_RGBA_OPENGL, GL_TRUE, sizeof(SE_Gizmo_Vertex), (void*)offsetof(SE_Gizmo_Vertex, colour));


    shape->vert_count = index_count;
    shape->indexed = true;
    shape->aabb = calc_aabb(verts, verts_count);
    shape->type = type;

        //- unselect
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static u32 add_shape(SE_Gizmo_Renderer *renderer) {
    se_assert(renderer->shapes_count < _SEGIZMO_RENDERER_MAX_SHAPES);
    u32 result = renderer->shapes_count;
    renderer->shapes_count++;
    return result;
}

u32 se_gizmo_add_coordniates(SE_Gizmo_Renderer *renderer) {
    u32 result = add_shape(renderer);
    SE_Gizmo_Shape *shape = &renderer->shapes[result];

    shape->line_width = 3;

    Vec3 pos_o = vec3_zero();
    Vec3 pos_x = vec3_right();
    Vec3 pos_y = vec3_up();
    Vec3 pos_z = vec3_forward();

    SE_Gizmo_Vertex verts[6] = {
        {.position = pos_o}, // x
        {.position = pos_x},
        {.position = pos_o}, // y
        {.position = pos_y},
        {.position = pos_o}, // z
        {.position = pos_z}
    };

    verts[0].colour = RGBA_RED;
    verts[1].colour = RGBA_RED;
    verts[2].colour = RGBA_GREEN;
    verts[3].colour = RGBA_GREEN;
    verts[4].colour = RGBA_BLUE;
    verts[5].colour = RGBA_BLUE;

    u32 indices[6] = {
        0, 1, // x
        2, 3, // y
        4, 5, // z
    };

    generate_shape(shape, SE_GIZMO_TYPE_LINE, verts, 6, indices, 6);
    return result;
}

u32 se_gizmo_add_aabb(SE_Gizmo_Renderer *renderer, Vec3 min, Vec3 max, f32 line_width) {
    u32 result = add_shape(renderer);
    SE_Gizmo_Shape *shape = &renderer->shapes[result];

    shape->line_width = line_width;

    SE_Gizmo_Vertex verts[8] = {
        {.position = (Vec3) {min.x, min.y, min.z}}, // front bottom left - 0
        {.position = (Vec3) {max.x, min.y, min.z}}, // front bottom right - 1
        {.position = (Vec3) {max.x, max.y, min.z}}, // front top right - 2
        {.position = (Vec3) {min.x, max.y, min.z}}, // front top left - 3

        {.position = (Vec3) {min.x, min.y, max.z}}, // behind bottom left - 4
        {.position = (Vec3) {max.x, min.y, max.z}}, // behind bottom right - 5
        {.position = (Vec3) {max.x, max.y, max.z}}, // behind top right - 6
        {.position = (Vec3) {min.x, max.y, max.z}}, // behind top left - 7
    };

    verts[0].colour = RGBA_WHITE;
    verts[1].colour = RGBA_WHITE;
    verts[2].colour = RGBA_WHITE;
    verts[3].colour = RGBA_WHITE;
    verts[4].colour = RGBA_WHITE;
    verts[5].colour = RGBA_WHITE;
    verts[6].colour = RGBA_WHITE;
    verts[7].colour = RGBA_WHITE;

    u32 indices[24] = {
        0, 1,
        0, 4,
        0, 3,
        5, 1,
        5, 6,
        5, 4,
        2, 1,
        2, 3,
        2, 6,
        7, 3,
        7, 6,
        7, 4
    };

    generate_shape(shape, SE_GIZMO_TYPE_LINE, verts, 8, indices, 24);
    return result;
}
