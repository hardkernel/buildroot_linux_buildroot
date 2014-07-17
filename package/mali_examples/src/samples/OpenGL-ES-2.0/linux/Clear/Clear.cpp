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
 * \file Clear.cpp
 * \brief A sample which can be used to identify if vsync is disabled
 *
 * The sample rapidly clears the screen with different colors.
 * If vsync is disabled there will be horizontal tearing between 
 * one color and the next.
 */
 
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>
#include <cstdlib>
#include "Platform.h"
#include "EGLRuntime.h"

#define WINDOW_W 800
#define WINDOW_H 600

using namespace MaliSDK;

bool setupGraphics(int w, int h)
{
    GL_CHECK(glViewport(0, 0, w, h));
   
    return true;
}

void renderFrame(void)
{
    static int background = 0;

    int r = (background & 0x4) ? 255:0;
    int g = (background & 0x2) ? 255:0;
    int b = (background & 0x1) ? 255:0;

    background++;
    background &= 0x7; /* Cycle through primary and secondary colours */

    GL_CHECK(glClearColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
}

int main(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform * platform = Platform::getInstance();

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
    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object*/
    delete platform;

    return 0;
}
