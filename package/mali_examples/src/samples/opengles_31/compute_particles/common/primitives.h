/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "glutil.h"
#include "shader.h"

struct Mesh
{
    GLuint vertex_buffer;
    GLuint index_buffer;
    int num_indices;
    int num_vertices;

    void dispose();
    void bind();
};

Mesh gen_normal_plane();
Mesh gen_tex_quad();
Mesh gen_unit_sphere(int t_sample, int s_samples);

#endif
