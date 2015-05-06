/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#include "Texts.h"
#include "mCommon.h"

#include "MDRFontAtlas.h"
#include "MDRFontAtlasFactory.h"
#include "MImageTGA.h"
#include "MMatrix.h"
#include "MPathsManager.h"
#include "MDRRenderer.h"
#include "MDRRendererPrimitive.h"
#include "MDRRendererProgram.h"
#include "MDRRendererText.h"
#include "MDRRendererTexture.h"

#define VERTEX_SHADER_FILE "Texts_text.vert"
#define FRAGMENT_SHADER_FILE "Texts_text.frag"
#define TEXTURE_FILE "Atlas.tga"
#define FONT_INI_FILE "Atlas.ini"

#define WINDOW_W 800
#define WINDOW_H 600

/**
 * This is the main file of the Texts sample code. The sample contains a few other
 * classes defined in separate files. But essential classes for
 * this particular sample are MDRFontAtlas and MDRRenderText. The rest of the them
 * are created for general purpose of 3D graphics rendering.
 */

using namespace MaliSDK;

MDRRenderer* renderer = NULL;
MDRRendererProgram* program = NULL;
MDRRendererText* text = NULL;
MDRRendererText* text2 = NULL;
MDRRendererTexture* texture = NULL;
MDRFontAtlas* fontAtlas = NULL;

char theTmpStr[1024]        = {0};
float theAnimRotValue       = 0.0f;
float theAnimValue          = -2.0f;
float theAnimValueStep      = 0.03f;
const float theAnimValueMin = -8.0f;
const float theAnimValueMax = -0.8f;

/* A text object to draw text on the screen. */
Text *textTitle = NULL;

bool setupGraphics(int w, int h)
  {
  /* Allocate global variables */
  renderer = new MDRRenderer;
  program = new MDRRendererProgram;
  text = new MDRRendererText(*renderer);
  text2 = new MDRRendererText(*renderer);
  texture = new MDRRendererTexture;

  /* Initialize the Text object and add some text. */
  textTitle = new Text(MPathsManager::getFullPathStatic("").getData(), w, h);
  textTitle->addString(0, 0, "Texts", 255, 255, 255, 255);

  /* Initialize renderer */
  renderer->initialize(w, h);

  /* Load and parse INI file */
  fontAtlas = MDRFontAtlasFactory::load(MPathsManager::getFullPathStatic(FONT_INI_FILE));
  if (fontAtlas == NULL)
    return false;

  /* Load and create a texture atlas */
  const MImageTGA& image = fontAtlas->getImage();
  texture->create(image.getWidth(), image.getHeight(), image.getData());

  /* Vertex and pixel shaders initialization */
  program->initialize(MPathsManager::getFullPathStatic(VERTEX_SHADER_FILE),
                      MPathsManager::getFullPathStatic(FRAGMENT_SHADER_FILE));

  /* Prepare text */
  text->setProgramRef(program);
  text->setFontRef(fontAtlas);
  text2->setProgramRef(program);
  text2->setFontRef(fontAtlas);
  text2->setPosition(MVector3f(-0.1f, -0.2f, -3.0f));

  return true;
  }

void terminateGraphics()
  {
  deleteAndNULL(texture);
  deleteAndNULL(text2);
  deleteAndNULL(text);
  deleteAndNULL(program);
  deleteAndNULL(renderer);
  deleteAndNULL(fontAtlas);
  }

void renderFrame(void)
  {
  /* Do animation theAnimValue variable */
  if (theAnimValue < theAnimValueMin && theAnimValueStep < 0.0f)
    theAnimValueStep *= -1.0f;
  else if (theAnimValue > theAnimValueMax && theAnimValueStep > 0.0f)
    theAnimValueStep *= -1.0f;
  theAnimValue += theAnimValueStep;
  theAnimRotValue += 1.0f;

  /* Change and prepare text */
  sprintf(theTmpStr, "xyz=(%.2f,%.2f,%.2f)", text->getPosition()[0],
                                             text->getPosition()[1],
                                             text->getPosition()[2]);
  text->setString(theTmpStr);
  text->prepare();

  /* Change text for the Frame counter */
  sprintf(theTmpStr, "Frame=%d", renderer->getFrameCounter());
  text2->setString(theTmpStr);
  text2->prepare();

  /* Prepare frame for rendering */
  renderer->preFrameRender();

  /* Ensure the correct texture is bound to texture unit 0. */
  renderer->activateTexture(MDRRenderer::TEXUNIT0, *texture);

  /* Set position and rotation render the text */
  text->setPivot(MVector3f(0.3f, 0.0f, 0.0f));
  text->setRotation(MVector3f(0.0f, 0.0f, theAnimRotValue));
  text->setPosition(MVector3f(-0.3f, 0.0f, theAnimValue));

  /* Render texts */
  text->render();
  text2->render();

  /* Call post frame render it increases frame counter */
  renderer->postFrameRender();
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

    /* Release resources of the sample */
    terminateGraphics();

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    return 0;
}