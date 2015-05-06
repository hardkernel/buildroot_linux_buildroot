/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "mCommon.h"

#define SAMPLE_TITLE "Fur"
#define WINDOW_W 800
#define WINDOW_H 600

#define VERTEX_SHADER_FILE "Fur_fur.vert"
#define FRAGMENT_SHADER_FILE "Fur_fur.frag"
#define IMAGE_FILE_FUR "Grain.tga"
#define IMAGE_FILE_LOGO "Logo.tga"

#include "MAnimation.h"
#include "MGeometrySphere.h"
#include "MImageTGA.h"
#include "MDRRenderer.h"
#include "MRendererPrimitive.h"
#include "MRendererProgram.h"
#include "MDRRendererTexture.h"
#include "MTransformation.h"
#include "MAnimation.h"
#include "MPathsManager.h"
#include "MTime.h"
#include "MVector2.h"
#include "MVector3.h"

using namespace MaliSDK;

MDRRendererTexture* theTexture = NULL;
MRendererProgram* theProgram = NULL;
MDRRenderer* theRenderer = NULL;
MTransformation* theTransformation = NULL;
MGeometrySphere* theGeometrySphere = NULL;

MVector3f theSensorPosModifier(0.0f, 0.0f, 0.0f);
MVector3f theSensorRotModifier(0.0f, 0.0f, 0.0f);
MTime theTime;
MAnimation1f theAnimX;
MAnimation1f theAnimY;
float theSpeedModifier = 0.0f;
float theAngle = 0.0f;
const unsigned int theFurLength = 20;

Text* text = NULL;

bool setupGraphics(int w, int h)
{
    /* Allocate global variables */
    theTexture = new MDRRendererTexture;
    theProgram = new MRendererProgram;
    theRenderer = new MDRRenderer;
    theTransformation = new MTransformation;
    theGeometrySphere = new MGeometrySphere;

    /* Initialize font. */
    text = new Text(MPathsManager::getFullPathStatic("").getData(), w, h);
    text->addString(0, 0, "Fur", 255, 255, 255, 255);

    /* Initialize theRenderer */
    theRenderer->initialize(w, h);

    /* Load and create textures */
    MImageTGA image;
    image.load(MPathsManager::getFullPathStatic(IMAGE_FILE_FUR));
    theTexture->create(image.getWidth(), image.getHeight(), image.getData());

    /* Shader initialization */
    theProgram->initialize(MPathsManager::getFullPathStatic(VERTEX_SHADER_FILE),
                           MPathsManager::getFullPathStatic(FRAGMENT_SHADER_FILE));
    theProgram->use();
    /* Bind attribute parameters */
    theProgram->bindAttrib(MRendererProgram::A_LOC_VERTEX,     "a_v4position");
    theProgram->bindAttrib(MRendererProgram::A_LOC_TEXCOORD_0, "a_v2TexCoord");
    /* Bind uniforms parameters */
    theProgram->bindUniform(MRendererProgram::U_LOC_MAT_MVP,   "u_m4MVP");
    theProgram->bindUniform(MRendererProgram::U_LOC_GENERIC_1, "u_v4Color");
    theProgram->bindUniform(MRendererProgram::U_LOC_SAMPLER_0, "u_s2dShadowMap");
    /* Bind u_s2dShadowMap onto first texture unit*/
    theProgram->setUniform(MRendererProgram::U_LOC_SAMPLER_0, 0);

    /*Create geometries*/
    theGeometrySphere->set(0.5f, 10, 10);

    /*Set default transformation settings*/
    theTransformation->setProjection(45.0f, (float)theRenderer->getWidth()/(float)theRenderer->getHeight(), 0.1f, 100.0f);
    theTransformation->setCameraPosition(MVector3f(0.0f, 0.0f, 3.5f));
    theTransformation->setCameraRotation(MVector4f(-30.0f, 1.0f, 0.0f, 0.0f));
    theTransformation->setObjectPosition(MVector3f(0.0f, 0.0f, 0.0f));
    theTransformation->setObjectScale(MVector3f(1.0f, 1.0f, 1.0f));

    theAnimX.setMode(MAnimation1f::PLAY_MODE_REPEAT);
    theAnimX.setKey(0.0f, MTime(0));
    theAnimX.setKey(360.0f, MTime(5.0));

    return true;
}

void terminateGraphics()
{
    deleteAndNULL(theTexture);
    deleteAndNULL(theProgram);
    deleteAndNULL(theRenderer);
    deleteAndNULL(theTransformation);
    deleteAndNULL(theGeometrySphere);
    //
    deleteAndNULL(text);
}

void renderFrame(void)
{
    /* Get current time and all the animations based on this time */
    theTime.update();
    theAnimX.update(theTime);
    theAnimY.update(theTime);

    /* Evaluate animations */
    MVector3f scaleObject(1.0f, 1.0f, 1.0f);
    MVector3f position(0.0f, 0.0f, 0.0f);
    MVector3f rotation(theSensorRotModifier);

    MVector4f handRot(theAnimX.getValue(), 0.0f, 1.0f, 0.0f);

    /*  Clear a framebuffer */
    theRenderer->preFrameRender();

    /* Ensure the correct texture is bound to texture unit 0. */
    theRenderer->activateTexture(MDRRenderer::TEXUNIT0, *theTexture);

    /* Bind vertex and fragment shaders */
    theProgram->use();

    /* Draw front faces of geometry as multiple semi-transparent shells (this will give impression of a fur) */
    for (unsigned int index = 0; index < theFurLength; index++)
    {
        /* Make a rendered geometry bigger and bigger in an every iteration */
        scaleObject += 0.018f;

        /* On Android devices do some additional effect according to sensors like a gyroscope and accelerometer */
        position += theSensorPosModifier;
        rotation += theSensorRotModifier;
        handRot[0] -= theSpeedModifier * 0.25f;

        /* Apply modifiers to the transformation of the geometry */
        theTransformation->setObjectRotation(handRot);
        theTransformation->setObjectPosition(position);
        theTransformation->setPreObjectScale(scaleObject);
        theTransformation->setPreObjectEulerRotation(rotation);

        /* Translate the transformation onto the MVP matris and pass as a uniform parameter to a vertex shader */
        theProgram->setUniform(MRendererProgram::U_LOC_MAT_MVP, theTransformation->getMatrix(MTransformation::TYPE_MATRIX_MVP));

        /* Make a geometry more transparent in an every following iteration by modifying an alpha channel */
        theProgram->setUniform(MRendererProgram::U_LOC_GENERIC_1, MVector4f(1.0f, 1.0f, 1.0f, (theFurLength - index) * 0.01f));

        /* Render the a geometry as one of the shells */
        theGeometrySphere->render(*theProgram);
    }

    // Call post frame render it increases frame counter
    theRenderer->postFrameRender();

    /* Draw any text. */
    text->draw();

    /* Modify rotation speed (this is for hindering the spining animation if not interaction is being done). */
    theAngle += theSpeedModifier;
    theSpeedModifier -= theSpeedModifier * 0.01f;
}

/* #define TYPE_ACCELEROMETER 0x00000001 */
#define TYPE_GYROSCOPE 0x00000004
/*
 * For more types see: http://developer.android.com/reference/android/hardware/Sensor.html
 * In order to activate more types you have to edit Fur.java file and add as many sensors as you wish.
 */

void sensorChanged(
    int aType,
    float aValue1,
    float aValue2,
    float aValue3)
{
    switch (aType)
    {
        case TYPE_GYROSCOPE:
        {
            float coef = 0.5f;
            theSensorRotModifier[0] = -aValue1 * coef;
            theSensorRotModifier[1] = -aValue2 * coef;
            theSensorRotModifier[2] = -aValue3 * coef;
            break;
        }
        default:
            LOGE("Unsupported sensor type: [%d]", aType);
    }
  }

void touchStart(int x, int y)
{
    /* Empty. */
}

MVector2i* theStartPnt = NULL;
void touchMove (int x, int y)
{
    static MVector2i prevPnt(0, 0);
    if (theStartPnt == NULL)
    {
        theStartPnt = new MVector2i(x, y);
        prevPnt = *theStartPnt;

        /*
         * Get a current values from animations, and then remove all existing keyframes from the animations.
         * In place of all removed keyframes crate only one keyframe with current value of the animations.
         * This piece of code is to prevent memory leak because adding keyframes to the animations because
         * sooner or later application will run out of memory because of number of keyframes.
         */
        float v1 = theAnimX.getValue();
        float v2 = theAnimY.getValue();
        theAnimX.removeAllKeys();
        theAnimY.removeAllKeys();
        theAnimX.setKey(v1, theTime);
        theAnimY.setKey(v2, theTime);

        /*
         * This case is treated as first touch where we do not have previous point. So there is nothing we can do more here.
         * So return from the function and let's wait for the next point.
         */
        return;
    }

    /* Calculate offset between current and previous position of touch. */
    MVector2i currMovePnt(x, y);
    MVector2i offset = currMovePnt - prevPnt;

    /* Eliminate noise when the finger is nearly at the same position. */
    if (offset.length() < 20)
        return;

    /* Convert offset to floating point offset. */
    MVector2f offsetf((float)offset[0], (float)offset[1]);

    /* Create animation of vector of quaternion around which the object is being spun. */
    theAnimX.setKey(offsetf[0], theTime + MTime(0.3));
    theAnimY.setKey(offsetf[1], theTime + MTime(0.3));

    /* Modify speed of rotation around the quaternion. */
    theSpeedModifier += offsetf.length() * 0.01f;

    /* Now store a current position as previous one. */
    prevPnt = currMovePnt;
}

void touchEnd(int x, int y)
{
    if (theStartPnt == NULL)
    {
        return;
    }
    delete theStartPnt;
    theStartPnt = NULL;
}

int main(int argc, char **argv)
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
        if(platform->checkWindow() == Platform::WINDOW_EXIT)
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

    /* Release resources of the sample */
    terminateGraphics();

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    return 0;
}
