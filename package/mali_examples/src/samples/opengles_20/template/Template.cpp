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
 * \file Template.cpp
 * \brief A blank sample to use as a basis for OpenGL ES 2.0 applications
 *
 * This is a functioning OpenGL ES 2.0 application which renders nothing to the screen.
 * Add setup code to setupGraphics(), for example, code to load shaders and textures.
 * To use assets (shaders, textures, etc.), place them in the assets folder of the sample.
 * Add code to actually render the scene in renderFrame().
 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <cstdio>
#include <cstdlib>

#include "Template.h"
#include "Text.h"
#include "Platform.h"
#include "EGLRuntime.h"

#define WINDOW_W 800
#define WINDOW_H 600

using std::string;
using namespace MaliSDK;

/* Asset directories and filenames. */
string resourceDirectory = "assets/";

/* A text object to draw text on the screen.*/ 
Text* text;

bool setupGraphics(int width, int height)
{    
    /* Initialize OpenGL ES. */
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    /* Initialize the Text object and add some text. */
    text = new Text(resourceDirectory.c_str(), width, height);
    
    text->addString(0, 0, "Template", 255, 255, 255, 255);
    
    /* Code to setup shaders, geometry, and to initialize OpenGL ES states. */
    
    return true;
}

void renderFrame(void)
{
    /* Code to draw one frame. */
    
    /* Draw fonts. */
    text->draw();
}

int main(int argc, char **argv)
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

    /* Shut down Text. */
    delete text;

    /* Shut down OpenGL ES. */
    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object*/
    delete platform;

    return 0;
}
