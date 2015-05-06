/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "glutil.h"
#include <fstream>
#include <iostream>

Shader current;

void cull(bool enabled, GLenum front, GLenum mode)
{
    if (enabled)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(front);
        glCullFace(mode);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void depth_test(bool enabled, GLenum func)
{
    if (enabled)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(func);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

void depth_write(bool enabled)
{
    if (enabled)
    {
        glDepthMask(GL_TRUE);
        glDepthRangef(0.0f, 1.0f);
    }
    else
    {
        glDepthMask(GL_FALSE);
    }
}

void blend_mode(bool enabled, GLenum src, GLenum dest, GLenum func)
{
    if (enabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(src, dest);
        glBlendEquation(func);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void use_shader(Shader shader)
{
    current = shader;
    current.use();
}

void attribfv(string name, GLsizei num_components, GLsizei stride, GLsizei offset)
{ 
    current.set_attribfv(name, num_components, stride, offset); 
}

void unset_attrib(string name)
{
    current.unset_attrib(name);
}

void uniform(string name, const mat4 &v) { current.set_uniform(name, v); }
void uniform(string name, const vec4 &v) { current.set_uniform(name, v); }
void uniform(string name, const vec3 &v) { current.set_uniform(name, v); }
void uniform(string name, const vec2 &v) { current.set_uniform(name, v); }
void uniform(string name, double v) { current.set_uniform(name, v); }
void uniform(string name, float v) { current.set_uniform(name, v); }
void uniform(string name, int v) { current.set_uniform(name, v); }
void uniform(string name, unsigned int v) { current.set_uniform(name, v); }

bool read_file(const std::string &path, std::string &dest)
{
    std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);

    std::string msg = "Failed to open file " + path;
    ASSERT(in.is_open() && in.good(), msg.c_str());

    in.seekg(0, std::ios::end);         // Set get position to end
    dest.resize(in.tellg());            // Resize string to support enough bytes
    in.seekg(0, std::ios::beg);         // Set get position to beginning
    in.read(&dest[0], dest.size());     // Read file to string
    in.close();

    return true;
}

GLuint gen_buffer(GLenum target, GLenum usage, GLsizei size, const void *data)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, size, data, usage);
    glBindBuffer(target, 0);
    return buffer;
}

GLuint gen_buffer(GLenum target, GLsizei size, const void *data)
{
    return gen_buffer(target, GL_STATIC_DRAW, size, data);
}

void del_buffer(GLuint buffer)
{
    glDeleteBuffers(1, &buffer);
}
