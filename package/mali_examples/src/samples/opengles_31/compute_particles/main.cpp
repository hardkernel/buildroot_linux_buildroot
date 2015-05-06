/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#include <stdlib.h>

#include "app.h"
#include "common/common.h"
#include "Timer.h"

#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480

int main()
{
    Platform *platform = Platform::getInstance();
    if (platform == NULL)
    {
        return EXIT_FAILURE;
    }

    platform->createWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES31);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

    Timer timer;
    timer.reset();

    if (!load_app())
    {
        return EXIT_FAILURE;
    }

    init_app(WINDOW_WIDTH, WINDOW_HEIGHT);

    while (platform->checkWindow() == Platform::WINDOW_IDLE)
    {
        float delta_time = timer.getInterval();
        float total_time = timer.getTime();
        update_app(delta_time, total_time);
        render_app(delta_time, total_time);

        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    free_app();

    EGLRuntime::terminateEGL();
    platform->destroyWindow();
    delete platform;

    return EXIT_SUCCESS;
}
