/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRRenderer.h"

using namespace MaliSDK;

MDRRenderer::MDRRenderer()
    :
    theWidth(0xFFFFFFFF),
    theHeight(0xFFFFFFFF),
    //
    theMatrixP(),
    //
    theFrameCounter(0)
  {
  }

MDRRenderer::~MDRRenderer()
  {
  }

bool MDRRenderer::initialize(
    unsigned int aWidth,
    unsigned int aHeight)
  {
  theWidth = aWidth;
  theHeight = aHeight;

  // Calculate projective matrix
  theMatrixP.setPerspective(45.0f, (float)theWidth/(float)theHeight, 0.1f, 100.0f);

  /* Initialize OpenGL-ES. */
  GL_CHECK(glClearColor(0.2f, 0.2f, 0.2f, 0.0f));
  GL_CHECK(glClearDepthf(0.0f));
  GL_CHECK(glEnable(GL_BLEND));
  GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL_CHECK(glDisable(GL_CULL_FACE));
  GL_CHECK(glEnable(GL_DEPTH_TEST));
  GL_CHECK(glDepthMask(false));
  GL_CHECK(glDepthFunc(GL_GREATER));

  return true;
  }

void MDRRenderer::preFrameRender()
  {
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
  }

void MDRRenderer::postFrameRender()
  {
  theFrameCounter++;
  }

void MDRRenderer::activateTexture(
    MDRTexUnit aUnit,
    const MDRRendererTexture& aTexture)
  {
  GL_CHECK(glActiveTexture(GL_TEXTURE0 + aUnit));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, aTexture.getName()));
  }
