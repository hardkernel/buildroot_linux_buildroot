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
 * \file InstancedTesselation.cpp
 * \brief The application displays a rotating solid torus with a low-polygon wireframed mesh surrounding it. The torus is drawn
 *        by means of instanced tessellation technique.
 *
 *        To perform instanced tessellation, we need to divide our model into several patches. Each patch is densly packed with
 *        triangles and improves effect of round surfaces. In the first stage of tessellation, patches consist of vertices 
 *        placed in a form of square. Once passed to the shader, they are transformed into Bezier surface on the basis of
 *        control points stored in uniform blocks. Each instance of a draw call renders next part of the torus.
 *
 *        The following application instantiates 2 classes to manage both solid torus model and the wireframe that surrounds it.
 *        The first class is responsible for configuration of a program with shaders capable of instanced drawing, initialization
 *        of data buffers and handling instanced draw calls. To simplify the mathemathics and satisfy conditions for C1 continuity
 *        between patches, we assume that torus is constructed by 12 circles, each also defined by 12 points. In that manner, we
 *        are able to divide "big" and "small" circle of torus into four quadrants and build Bezier surfaces that approximate
 *        perfectly round shapes. For that purpose, the control points cannot lay on the surface of the torus, but have to be
 *        distorted properly.
 *
 *        The second class manages components corresponding to the wireframe. It uses vertices placed on the surface of torus and
 *        uses a simple draw call with GL_LINES mode. The size of its "small circle" is slightly bigger than the corresponding dimension
 *        of the solid torus, so there is a space between both models.
 *
 *        Common elements for both classes are placed in an abstract Torus class.
 */

#include "InstancedTesselation.h"
#include "Torus.h"
#include "WireframeTorus.h"
#include "InstancedSolidTorus.h"

#include "Platform.h"
#include "EGLRuntime.h"
#include "Matrix.h"
#include "Shader.h"

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

using std::string;
using namespace MaliSDK;

const unsigned int windowWidth  = 800;
const unsigned int windowHeight = 600;

/* Asset directory. */
string resourceDirectory = "assets/";

/* Object managing OpenGL components which draw torus in wireframe. */
Torus* wireframeTorus;
/* Object managing OpenGL components which draw instanced solid torus. */
Torus* solidTorus;

/* Initialize graphic components. */
bool setupGraphics(void)
{
    /* Distance between center of torus and center of its construction circle. */
    const float torusRadius = 1.0f;
    /* Radius of the construction circle. */
    const float circleRadius = 0.4f;
    /* Distance between solid torus and mesh. */
    const float distance = 0.05f;

    /* Construct torus objects. */
    wireframeTorus = new WireframeTorus(torusRadius, circleRadius + distance);

    if (wireframeTorus == NULL)
    {
        LOGE("Could not instantiate WireframeTorus class.");
        return false;
    }

    solidTorus = new InstancedSolidTorus(torusRadius, circleRadius);

    if (solidTorus == NULL)
    {
        LOGE("Could not instantiate InstancedSolidTorus class.");
        return false;
    }

    /* Configure projection matrix. */
    Matrix projectionMatrix = Matrix::matrixPerspective(45.0f, float(windowWidth) / float(windowHeight), 0.1f, 100.0f);

    /* Set projection matrices for each of the torus objects. */
    wireframeTorus->setProjectionMatrix(&projectionMatrix);
    solidTorus->setProjectionMatrix(&projectionMatrix);

    /* Initialize OpenGL-ES. */
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthFunc(GL_LEQUAL));

    return true;
}

/* Render single frame to back buffer. */
void renderFrame()
{
    /* Rotation angles. */
    static float angleX = 0.0f;
    static float angleY = 0.0f;
    static float angleZ = 0.0f;

    float rotationVector[] = {angleX, angleY, angleZ};

    /* Clear the screen */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Draw both toruses. */
    wireframeTorus->draw(rotationVector);
    solidTorus->draw(rotationVector);

    /* Increment rotation angles. */
    angleX += 0.5;
    angleY += 0.5;
    angleZ += 0.5;

    if(angleX >= 360.0f) angleX = 0.0f;
    if(angleY >= 360.0f) angleY = 0.0f;
    if(angleZ >= 360.0f) angleZ = 0.0f;
}

int main(int argc, char **argv)
{
    /* Intialise the Platform object for platform specific functions. */
    Platform * platform = Platform::getInstance();

    Torus::setResourceDirectory(resourceDirectory);

    if (platform != NULL)
    {
        /* Initialize windowing system. */
        platform->createWindow(windowWidth, windowHeight);

        /* Initialize EGL. */
        EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
        EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

        /* Initialize OpenGL ES graphics subsystem */
        if (setupGraphics())
        {
            bool end = false;
            /* The rendering loop to draw the scene. */
            while(!end)
            {
                /* Finish the loop when window status has been changed. */               
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

            /* Delete torus objects. */
            delete wireframeTorus;
            delete solidTorus;
        }
        else
        {
            LOGE("Graphic setup failed");
        }

        /* Shut down OpenGL ES. */
        /* Shut down EGL. */
        EGLRuntime::terminateEGL();

        /* Shut down windowing system. */
        platform->destroyWindow();

        /* Shut down the Platform object*/
        delete platform;
    }
    else
    {
        LOGE("Could not create platform");
    }

    return 0;
}
