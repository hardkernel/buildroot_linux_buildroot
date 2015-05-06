/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "Shader.h"
#include "Platform.h"

#include <cstdio>
#include <cstdlib>

namespace MaliSDK
{
    /* Please see header for the specification. */
    void Shader::processShader(GLuint* shaderPtr, const char* shaderSourcePtr, GLint shaderType)
    {
        GLint compilationStatus = GL_FALSE;

        /* Create shader and load into GL. */
        *shaderPtr = GL_CHECK(glCreateShader(shaderType));

        GL_CHECK(glShaderSource(*shaderPtr, 1, &shaderSourcePtr, NULL));

        /* Try compiling the shader. */
        GL_CHECK(glCompileShader(*shaderPtr));

        GL_CHECK(glGetShaderiv(*shaderPtr, GL_COMPILE_STATUS, &compilationStatus));

        /* Dump debug info (source and log) if compilation failed. */
        if(compilationStatus != GL_TRUE)
        {
            GLint length;
            char *debugSource = NULL;
            char *errorLog    = NULL;

            /* Get shader source. */
            GL_CHECK(glGetShaderiv(*shaderPtr, GL_SHADER_SOURCE_LENGTH, &length));

            debugSource = (char *)malloc(length);

            GL_CHECK(glGetShaderSource(*shaderPtr, length, NULL, debugSource));

            LOGE("Debug source START:\n%s\nDebug source END\n\n", debugSource);

            free(debugSource);

            /* Now get the info log. */
            GL_CHECK(glGetShaderiv(*shaderPtr, GL_INFO_LOG_LENGTH, &length));

            errorLog = (char *)malloc(length);

            GL_CHECK(glGetShaderInfoLog(*shaderPtr, length, NULL, errorLog));

            LOGE("Log START:\n%s\nLog END\n\n", errorLog);

            free(errorLog);

            LOGE("Compilation FAILED!\n\n");
            exit(1);
        }
    }
}
