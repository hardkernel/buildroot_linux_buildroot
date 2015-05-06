/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef UTIL_H
#define UTIL_H

#include "matrix.h"
#include "common.h"
#include "shader.h"
#include <string>
#include <sstream>

/*
    Triangles are either drawn in a clockwise or counterclockwise order. Facets that face away from the viewer
    can be hidden by setting the rasterizer state to cull such facets.
*/
void cull(bool enabled, GLenum front = GL_CCW, GLenum mode = GL_BACK);

/*
    The incoming pixel depth value can be compared with the depth value present in the depth buffer,
    to determine whether the pixel shall be drawn or not.
*/
void depth_test(bool enabled, GLenum func = GL_LEQUAL);

/*
    The incoming pixel can write to the depth buffer, combined with the depth_test function.
*/
void depth_write(bool enabled);

/*
    Pixels can be drawn using a function that blends the incoming (source) RGBA values with the RGBA 
    values that are already in the frame buffer (the destination values).
*/
void blend_mode(bool enabled, GLenum src = GL_ONE, GLenum dest = GL_ONE, GLenum func = GL_FUNC_ADD);

void use_shader(Shader shader);
void attribfv(string name, GLsizei num_components, GLsizei stride, GLsizei offset);
void unset_attrib(string name);

void uniform(string name, const mat4 &v);
void uniform(string name, const vec4 &v);
void uniform(string name, const vec3 &v);
void uniform(string name, const vec2 &v);
void uniform(string name, double v);
void uniform(string name, float v);
void uniform(string name, int v);
void uniform(string name, unsigned int v);

bool read_file(const std::string &path, std::string &dest);
GLuint gen_buffer(GLenum target, GLsizei size, const void *data);
GLuint gen_buffer(GLenum target, GLenum usage, GLsizei size, const void *data);
void del_buffer(GLuint buffer);

#endif
