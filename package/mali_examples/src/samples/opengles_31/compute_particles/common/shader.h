/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef SHADER_H
#define SHADER_H

#include "matrix.h"
#include "common.h"
#include <map>

class Shader
{
    public:
        Shader();

        bool load_from_src(const string *sources, GLenum *types, int count);
        bool load_from_src(string vs_src, string fs_src);
        bool load_from_file(const string *paths, GLenum *types, int count);
        bool load_from_file(string vs_path, string fs_path);
        bool load_compute_from_file(string cs_path);
        bool link();
        void dispose();

        void use();
        void unuse();

        GLint get_uniform_location(string name);
        GLint get_attribute_location(string name);

        void set_attribfv(string name, GLsizei num_components, GLsizei stride, GLsizei offset);
        void unset_attrib(string name);

        void set_uniform(string name, const mat4 &v);
        void set_uniform(string name, const vec4 &v);
        void set_uniform(string name, const vec3 &v);
        void set_uniform(string name, const vec2 &v);
        void set_uniform(string name, double v);
        void set_uniform(string name, float v);
        void set_uniform(string name, int v);
        void set_uniform(string name, unsigned int v);

    private:
        std::map<string, GLint> m_attributes;
        std::map<string, GLint> m_uniforms;
        GLuint m_id;
        std::vector<GLuint> m_shaders;
};

#endif
