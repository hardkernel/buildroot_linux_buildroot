/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef MESH_HPP__
#define MESH_HPP__

#include <vector>
#include <stdint.h>
#include "common.hpp"

struct Vertex
{
    Vertex() {}
    Vertex(vec3 position, vec3 normal, vec2 tex)
        : position(position), normal(normal), tex(tex) {}

    vec3 position;
    vec3 normal;
    vec2 tex;
};

struct AABB
{
    vec4 minpos;
    vec4 maxpos;
};

struct Mesh
{
    std::vector<Vertex> vbo;
    std::vector<uint16_t> ibo;
    AABB aabb;
};

Mesh create_box_mesh(const AABB &aabb);
Mesh create_sphere_mesh(float radius, vec3 center, unsigned vertices_per_circumference);

class GLDrawable
{
    public:
        GLDrawable(const Mesh &mesh);
        GLDrawable();
        ~GLDrawable();

        const AABB& get_aabb() const;

        GLuint get_vertex_array() const;
        unsigned get_num_elements() const;

    private:
        GLuint vertex_array;
        GLuint vertex_buffer;
        GLuint index_buffer;

        unsigned num_elements;
        AABB aabb;
};

#endif
