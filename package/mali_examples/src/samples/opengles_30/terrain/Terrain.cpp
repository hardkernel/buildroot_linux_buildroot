/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _WIN32
    #include <unistd.h>
#endif /* _WIN32 */

#include "Terrain.h"
#include "ClipmapApplication.h"
#include "Platform.h"

#include <cmath>

using namespace MaliSDK;

ClipmapApplication* app = NULL;

/* Initialize data. */
void init()
{
    delete app;
    app = new ClipmapApplication(CLIPMAP_SIZE, CLIPMAP_LEVELS, CLIPMAP_SCALE);
}

/* Render single frame. */
void renderFrame()
{
    app->render(WINDOW_WIDTH, WINDOW_HEIGHT);
}

int main(int argc, char** argv)
{
    /* Rendering loop to draw the scene starts here. */
    bool shouldContinueTheLoop = true;

    /* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if(platform == NULL)
    {
        fprintf(stderr, "Could not create platform\n");
        exit(-1);
    }

    /* Initialize windowing system. */
    platform->createWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

    init();

    while (shouldContinueTheLoop)
    {
        /* If something happened to the window, leave the loop. */
        if (platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            shouldContinueTheLoop = false;
        }

        renderFrame();

        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;

    return 0;
}