/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef SCENE_HPP__
#define SCENE_HPP__

#include "mesh.hpp"
#include "culling.hpp"
#include <vector>
#include <stdint.h>

class Scene
{
    public:
        Scene();
        ~Scene();

        void update(float delta_time, unsigned width, unsigned height);
        void render(unsigned width, unsigned height);
        void move_camera(float delta_x, float delta_y);

        enum CullingMethod {
            CullHiZ = 0,
            CullHiZNoLOD = 1,
            CullNone = -1
        };
        void set_culling_method(CullingMethod method);

        void set_physics_speed(float speed) { physics_speed = speed; }
        float get_physics_speed() const { return physics_speed; }
        void set_show_redundant(bool enable) { show_redundant = enable; }
        bool get_show_redundant() const { return show_redundant; }

    private:
        GLDrawable *box;
        GLDrawable *sphere[SPHERE_LODS];
        std::vector<CullingInterface*> culling_implementations;

        unsigned culling_implementation_index;

        bool show_redundant;
        bool enable_culling;

        void render_spheres(vec3 color_mod);

        void bake_occluder_geometry(std::vector<vec4> &occluder_positions,
                std::vector<uint32_t> &occluder_indices,
                const Mesh &box_mesh, const vec4 *instances, unsigned num_instances);

        GLuint occluder_program;
        GLuint sphere_program;

        GLDrawable quad;
        GLuint quad_program;

        // Allow for readbacks of atomic counter without stalling GPU pipeline.
#define INDIRECT_BUFFERS 4
        struct
        {
            GLuint buffer[INDIRECT_BUFFERS];
            unsigned buffer_index;
            GLuint instance_buffer[SPHERE_LODS];
        } indirect;

        void init_instances();
        GLuint physics_program;
        GLuint occluder_instances_buffer;
        GLuint sphere_instances_buffer;
        unsigned num_occluder_instances;
        unsigned num_sphere_render_lods;
        unsigned num_render_sphere_instances;

        void apply_physics(float delta_time);
        float physics_speed;

        void render_depth_map();

        mat4 projection;
        mat4 view;

        float camera_rotation_y;
        float camera_rotation_x;
        void update_camera(float rotation_y, float rotation_x, unsigned viewport_width, unsigned viewport_height);
};

#endif

