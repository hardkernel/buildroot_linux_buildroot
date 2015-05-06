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
 * \file Triangle.cpp
 * \brief A sample which shows how to draw a simple triangle to the screen.
 *
 * Uses a simple shader to fill the the triangle with a gradient color.
 */
 
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "Triangle.h"
#include "Text.h"
#include "Shader.h"
#include "Timer.h"
#include "Platform.h"
#include "EGLRuntime.h"

#define WINDOW_W 800
#define WINDOW_H 600

using std::string;
using namespace MaliSDK;

/* Asset directories and filenames. */
string resourceDirectory = "assets/";
string vertexShaderFilename = "Triangle_triangle.vert";
string fragmentShaderFilename = "Triangle_triangle.frag";

/* Shader variables. */
GLuint programID;
GLint iLocPosition = -1;
GLint iLocFillColor = -1;

/* A text object to draw text on the screen. */ 
Text *text;

bool setupGraphics(int width, int height)
{
    LOGD("setupGraphics(%d, %d)", width, height);

    /* Full paths to the shader files */
    string vertexShaderPath = resourceDirectory + vertexShaderFilename; 
    string fragmentShaderPath = resourceDirectory + fragmentShaderFilename;

    GLuint vertexShaderID = 0;
    GLuint fragmentShaderID = 0;

    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthFunc(GL_LEQUAL));
    
    
    /* Initialize OpenGL ES. */
    GL_CHECK(glEnable(GL_BLEND));
    /* Should do: src * (src alpha) + dest * (1-src alpha). */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Initialize the Text object and add some text. */
    text = new Text(resourceDirectory.c_str(), width, height);
    text->addString(0, 0, "Simple triangle", 255, 255, 0, 255);

    /* Process shaders. */
    Shader::processShader(&vertexShaderID, vertexShaderPath.c_str(), GL_VERTEX_SHADER);
    LOGD("vertexShaderID = %d", vertexShaderID);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);
    LOGD("fragmentShaderID = %d", fragmentShaderID);

    programID = GL_CHECK(glCreateProgram());
    if (programID == 0)
    {
        LOGE("Could not create program.");
        return false;
    }

    GL_CHECK(glAttachShader(programID, vertexShaderID));
    GL_CHECK(glAttachShader(programID, fragmentShaderID));
    GL_CHECK(glLinkProgram(programID));
    GL_CHECK(glUseProgram(programID));

    /* Positions. */
    GL_CHECK(iLocPosition = glGetAttribLocation(programID, "a_v4Position"));
    LOGD("glGetAttribLocation(\"a_v4Position\") = %d\n", iLocPosition);

    /* Fill colors. */
    GL_CHECK(iLocFillColor = glGetAttribLocation(programID, "a_v4FillColor"));
    LOGD("glGetAttribLocation(\"a_v4FillColor\") = %d\n", iLocFillColor);

    GL_CHECK(glViewport(0, 0, width, height));

    /* Set clear screen color. */
    GL_CHECK(glClearColor(0.0f, 0.0f, 1.0f, 1.0f));
    GL_CHECK(glClearDepthf(1.0f));
    return true;
}

void renderFrame(void)
{
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    GL_CHECK(glUseProgram(programID));

    /* Pass the triangle vertex positions to the shader */
    GL_CHECK(glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, triangleVertices));

    GL_CHECK(glEnableVertexAttribArray(iLocPosition));

    if(iLocFillColor != -1)
    {
        /* Pass the vertex colours to the shader */
        GL_CHECK(glVertexAttribPointer(iLocFillColor, 4, GL_FLOAT, GL_FALSE, 0, triangleColors));
        GL_CHECK(glEnableVertexAttribArray(iLocFillColor));
    }

    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));

    /* Draw fonts. */
    text->draw();
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

    /* Timer variable to calculate FPS. */
    Timer fpsTimer;
    fpsTimer.reset();

    bool end = false;
    /* The rendering loop to draw the scene. */
    while(!end)
    {
        /* If something has happened to the window, end the sample. */
        if(platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            end = true;
        }

        /* Calculate FPS. */
        float fFPS = fpsTimer.getFPS();
        if(fpsTimer.isTimePassed(1.0f))
        {
            LOGI("FPS:\t%.1f\n", fFPS);
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
    /* Shut down Text. */
    delete text;

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object*/
    delete platform;

    return 0;
}