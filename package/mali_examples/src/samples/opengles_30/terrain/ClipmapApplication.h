/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef CLIPMAP_APPLICATION_H__
#define CLIPMAP_APPLICATION_H__

#include <GLES3/gl3.h>
#include <string>

#include "GroundMesh.h"
#include "Heightmap.h"
#include "vector_math.h"

class ClipmapApplication
{
public:
    ClipmapApplication(unsigned int size, unsigned int levels, float clip_scale);
    ~ClipmapApplication();
    void render(unsigned int viewport_width, unsigned int viewport_height);

private:
    GLuint program;
    GLuint compile_program(const char *vertex_shader_source, const char *fragment_shader_source);
    GLuint compile_shader(GLenum type, const char *source);
    std::string load_shader_string(const char *path);

    GroundMesh mesh;
    Heightmap heightmap;

    GLint mvp_loc;
    GLint camera_pos_loc;
    int frame;
};

#endif
