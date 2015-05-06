/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef COMMON_HPP__
#define COMMON_HPP__

#include "vector_math.h"

#include <stddef.h>
#include <stdio.h>
#include <string>

#include "EGLRuntime.h"
#include "Platform.h"
using namespace MaliSDK;

#include <GLES3/gl31.h>

GLuint common_compile_shader(const char *vs_source, const char *fs_source);
GLuint common_compile_compute_shader(const char *cs_source);

GLuint common_compile_shader_from_file(const char *vs_source, const char *fs_source);
GLuint common_compile_compute_shader_from_file(const char *cs_source);

void common_set_basedir(const char *basedir);
FILE *common_fopen(const char *path, const char *mode);

std::string common_get_path(const char *basepath);

#endif
