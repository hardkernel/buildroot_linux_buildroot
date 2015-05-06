/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "common.hpp"
#include "scene.hpp"
#include "common.hpp"
#include <stdlib.h>

#define GLES_VERSION 3
#include "Timer.h"
#include "Text.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

using namespace std;

static void render_text(Text &text, const char *method, float current_time)
{
    // Enable alpha blending.
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    char method_string[128];
    sprintf(method_string, "Method: %s (%4.1f / 10.0 s)", method, current_time);

    text.clear();
    text.addString(300, WINDOW_HEIGHT - 20, method_string, 255, 255, 255, 255);

    text.addString(20, WINDOW_HEIGHT - 40,  "             Legend:", 255, 255, 255, 255);
    text.addString(20, WINDOW_HEIGHT - 60,  "Green tinted sphere: LOD 0", 255, 255, 0, 255);
    text.addString(20, WINDOW_HEIGHT - 80,  " Blue tinted sphere: LOD 1 - LOD 3", 255, 255, 0, 255);
    text.addString(20, WINDOW_HEIGHT - 100, "        Dark sphere: Occluded spheres", 255, 255, 0, 255);

    text.draw();
    GL_CHECK(glDisable(GL_BLEND));
}

static void run_application(Platform *platform)
{
    Scene *scene = new Scene;
    scene->set_show_redundant(true);
    scene->set_culling_method(Scene::CullHiZ);

    Text *text = new Text("assets/", WINDOW_WIDTH, WINDOW_HEIGHT);

    Timer timer;
    timer.reset();

    unsigned phase = 0;
    float culling_timer = 0.0f;

    while (platform->checkWindow() == Platform::WINDOW_IDLE)
    {
        float delta_time = timer.getInterval();

        // Render scene.
        scene->move_camera(delta_time * 0.1f, 0.0f);
        scene->update(delta_time, WINDOW_WIDTH, WINDOW_HEIGHT);
        scene->render(WINDOW_WIDTH, WINDOW_HEIGHT);

        static const char *methods[] = {
            "Hierarchical-Z occlusion culling with level-of-detail",
            "Hierarchical-Z occlusion culling without level-of-detail",
            "No culling"
        };
        render_text(*text, methods[phase], culling_timer);

        // Don't need depth nor stencil buffers anymore. Just discard them so they are not written out to memory on Mali.
        static const GLenum attachments[] = { GL_DEPTH, GL_STENCIL };
        GL_CHECK(glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments));

        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);

        // Change the culling method over time.
        culling_timer += delta_time;
        if (culling_timer > 10.0f)
        {
            culling_timer = 0.0f;
            phase = (phase + 1) % 3;

            switch (phase)
            {
                case 0:
                    scene->set_culling_method(Scene::CullHiZ);
                    break;
                case 1:
                    scene->set_culling_method(Scene::CullHiZNoLOD);
                    break;
                case 2:
                    scene->set_culling_method(Scene::CullNone);
                    break;
            }
        }
    }

    delete text;
    delete scene;
}

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

    common_set_basedir("assets");
    run_application(platform);

    EGLRuntime::terminateEGL();
    platform->destroyWindow();
    delete platform;

    return EXIT_SUCCESS;
}

