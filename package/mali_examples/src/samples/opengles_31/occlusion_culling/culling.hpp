/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef CULLING_HPP__
#define CULLING_HPP__

#include "common.hpp"
#include "mesh.hpp"
#include <vector>
#include <stdint.h>
#include <stddef.h>
#define SPHERE_LODS 4

// Layout is defined by OpenGL ES 3.1.
// We don't care about the three last elements in this case.
struct IndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint zero[3];
};

class CullingInterface
{
    public:
        virtual ~CullingInterface() {}

        // Sets up occlusion geometry. This is mostly static and should be done at startup of a scene.
        virtual void setup_occluder_geometry(const std::vector<vec4> &positions, const std::vector<uint32_t> &indices) = 0;

        // Sets current view and projection matrices.
        virtual void set_view_projection(const mat4 &projection, const mat4& view, const vec2 &zNearFar) = 0;

        // Rasterize occluders to depth map.
        virtual void rasterize_occluders() = 0;

        // Test bounding boxes in our scene.
        virtual void test_bounding_boxes(GLuint counter_buffer, const unsigned *counter_offsets, unsigned num_offsets,
                const GLuint *culled_instance_buffer, GLuint instance_data_buffer,
                unsigned num_instances) = 0;

        // Debugging functionality. Verify that the depth map is being rasterized correctly.
        virtual GLuint get_depth_texture() const { return 0; }

        virtual unsigned get_num_lods() const { return SPHERE_LODS; }

    protected:
        // Common functionality for various occlusion culling implementations.
        void compute_frustum_from_view_projection(vec4 *planes, const mat4 &view_projection);
};

#define DEPTH_SIZE 256
#define DEPTH_SIZE_LOG2 8
class HiZCulling : public CullingInterface
{
    public:
        HiZCulling();
        HiZCulling(const char *program);
        ~HiZCulling();

        void setup_occluder_geometry(const std::vector<vec4> &positions, const std::vector<uint32_t> &indices);
        void set_view_projection(const mat4 &projection, const mat4 &view, const vec2 &zNearFar);

        void rasterize_occluders();
        void test_bounding_boxes(GLuint counter_buffer, const unsigned *counter_offsets, unsigned num_offsets,
                const GLuint *culled_instance_buffer, GLuint instance_data_buffer,
                unsigned num_instances);

        GLuint get_depth_texture() const { return depth_texture; }

    private:
        GLuint depth_render_program;
        GLuint depth_mip_program;
        GLuint culling_program;

        GLDrawable quad;

        struct
        {
            GLuint vertex;
            GLuint index;
            GLuint vao;
            unsigned elements;
        } occluder;

        GLuint depth_texture;
        GLuint shadow_sampler;
        unsigned lod_levels;
        std::vector<GLuint> framebuffers;

        GLuint uniform_buffer;
        struct Uniforms
        {
            mat4 uVP;
            mat4 uView;
            mat4 uProj;
            vec4 planes[6];
            vec2 zNearFar;
        };
        Uniforms uniforms;

        void init();
};

// Variant of HiZRasterizer which only uses a single LOD.
class HiZCullingNoLOD : public HiZCulling
{
    public:
        HiZCullingNoLOD()
            : HiZCulling("shaders/hiz_cull_no_lod.cs")
        {}
        unsigned get_num_lods() const { return 1; }
};

#endif

