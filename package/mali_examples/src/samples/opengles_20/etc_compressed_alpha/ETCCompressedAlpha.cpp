/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * \file ETCCompressedAlpha.cpp
 * \brief A sample to show how to use textures with an seperate compressed image for alpha.
 *
 * ETC does not support alpha channels directly.
 * Here we use a texture which orginally contained an alpha channel but
 * was compressed using the Mali Texture Compression Tool using 
 * the "Create seperate compressed image" option for alpha handling.
 * This makes an ETC compressed image for the RGB channels and a seperate compressed image for 
 * the Alpha channel.
 * In this sample both images are loaded and the RGB and Alpha components are merged back
 * together in the fragment shader.
 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <string>

#include "ETCCompressedAlpha.h"
#include "Shader.h"
#include "Texture.h"
#include "Platform.h"
#include "EGLRuntime.h"

#define WINDOW_W 640
#define WINDOW_H 480

using std::string;
using namespace MaliSDK;

string resourceDirectory = "assets/";
string textureFilename = "good_compressed_mip_";
string imageExtension = ".pkm";
string alphaExtension = "_alpha.pkm";

string vertexShaderFilename = "ETCCompressedAlpha_dualtex.vert";
string fragmentShaderFilename = "ETCCompressedAlpha_dualtex.frag";

/* Texture variables. */
GLuint textureID = 0;
GLuint alphaTextureID = 0;

/* Shader variables. */
GLuint vertexShaderID = 0;
GLuint fragmentShaderID = 0;
GLuint programID = 0;
GLint iLocPosition = -1;
GLint iLocTexCoord = -1;
GLint iLocSampler = -1;

GLint iLocAlphaSampler= -1;

bool setupGraphics(int w, int h)
{
    LOGD("setupGraphics(%d, %d)", w, h);

    /* Full paths to the shader and texture files */
    string texturePath = resourceDirectory + textureFilename;
    string vertexShaderPath = resourceDirectory + vertexShaderFilename; 
    string fragmentShaderPath = resourceDirectory + fragmentShaderFilename;

    /* Initialize OpenGL ES. */
    /* Check which formats are supported. */
    if (!Texture::isETCSupported(true))
    {
        LOGE("ETC1 not supported");
        return false;
    }

    /* Enable alpha blending. */
    GL_CHECK(glEnable(GL_BLEND));
    /* Should do src * (src alpha) + dest * (1-src alpha). */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Initialize textures using separate files */
    Texture::loadCompressedMipmaps(texturePath.c_str(), imageExtension.c_str(), &textureID);
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    Texture::loadCompressedMipmaps(texturePath.c_str(), alphaExtension.c_str(), &alphaTextureID);

    /* Process shaders. */
    Shader::processShader(&vertexShaderID, vertexShaderPath.c_str(), GL_VERTEX_SHADER);
    LOGD("vertexShaderID = %d", vertexShaderID);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);
    LOGD("fragmentShaderID = %d", fragmentShaderID);

    programID = GL_CHECK(glCreateProgram());
    if (!programID)
    {
        LOGE("Could not create program.");
        return false;
    }
    GL_CHECK(glAttachShader(programID, vertexShaderID));
    GL_CHECK(glAttachShader(programID, fragmentShaderID));
    GL_CHECK(glLinkProgram(programID));
    GL_CHECK(glUseProgram(programID));

    /* Vertex positions. */
    iLocPosition = GL_CHECK(glGetAttribLocation(programID, "a_v4Position"));
    if(iLocPosition == -1)
    {
        LOGE("Error: Attribute not found: \"a_v4Position\"");
        exit(1);
    }
    GL_CHECK(glEnableVertexAttribArray(iLocPosition));

    /* Texture coordinates. */
    iLocTexCoord = GL_CHECK(glGetAttribLocation(programID, "a_v2TexCoord"));
    if(iLocTexCoord == -1)
    {
        LOGD("Warning: Attribute not found: \"a_v2TexCoord\"");
    }
    else
    {
        GL_CHECK(glEnableVertexAttribArray(iLocTexCoord));
    }

    /* Set the sampler to point at the 0th texture unit. */
    iLocSampler = GL_CHECK(glGetUniformLocation(programID, "u_s2dTexture"));
    if(iLocSampler == -1)
    {
        LOGD("Warning: Uniform not found: \"u_s2dTexture\"");
    }
    else
    {
        GL_CHECK(glUniform1i(iLocSampler, 0));
    }

    /* Set the alpha sampler to point at the 1st texture unit. */
    iLocAlphaSampler = GL_CHECK(glGetUniformLocation(programID, "u_s2dAlpha"));
    if(iLocAlphaSampler == -1)
    {
        LOGE("Uniform not found at line %i\n", __LINE__);
        exit(1);
    }
    GL_CHECK(glUniform1i(iLocAlphaSampler,1));


    /* Set clear screen color. */
    GL_CHECK(glClearColor(0.125f, 0.25f, 0.5f, 1.0));

    return true;
}

void renderFrame(void)
{
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    GL_CHECK(glUseProgram(programID));

    /* Pass the plane vertices to the shader. */
    GL_CHECK(glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices));

    GL_CHECK(glEnableVertexAttribArray(iLocPosition));

    if(iLocTexCoord != -1)
    {
        /* Pass the texture coordinates to the shader. */
        GL_CHECK(glVertexAttribPointer(iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinates));
        GL_CHECK(glEnableVertexAttribArray(iLocTexCoord));
    }

    GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(indices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, indices));
}

int main(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform *platform = Platform::getInstance();

    /* Initialize windowing system. */
    platform->createWindow(WINDOW_W, WINDOW_H);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES2);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

    /* Initialize OpenGL ES graphics subsystem. */
    setupGraphics(WINDOW_W, WINDOW_H);

    bool end = false;
    /* The rendering loop to draw the scene. */
    while(!end)
    {
        /* If something has happened to the window, end the sample. */
        if(platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            end = true;
        }

        /* Render a single frame */
        renderFrame();

        /* 
         * Push the EGL surface color buffer to the native window.
         * Causes the rendered graphics to be displayed on screen.
         */
        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    /* Shut down OpenGL ES. */
    /* Delete the textures. */
    GL_CHECK(glDeleteTextures(1, &textureID));
    GL_CHECK(glDeleteTextures(1, &alphaTextureID));

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;

    return 0;
}
