/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRRenderTarget.h"

#include "MDRFBO.h"
#include "MDRRendererTexture.h"

using namespace MaliSDK;

MDRRenderTarget::MDRRenderTarget(
    MDRMode aMode)
    :
    BaseClass(),
    //
    theWidth(0),
    theHeight(0),
    theRenderable(NULL)
  {
  switch (aMode)
    {
  case MODE_FBO:
    theRenderable = new MDRFBO;
    break;
  case MODE_COPY_TEXTURE:
  default:
    theRenderable = new MDRRendererTexture(false);
    break;
    }
  }

MDRRenderTarget::~MDRRenderTarget()
  {
  deleteAndNULL(theRenderable);
  }

bool MDRRenderTarget::initialize(
    unsigned int aWidth,
    unsigned int aHeight)
  {
  theWidth = aWidth;
  theHeight = aHeight;
  return theRenderable->initialize(theWidth, theHeight);
  }


bool MDRRenderTarget::bindRT() const
  {
  bool result = theRenderable->bindRT();
    /* Draw into FBO. */
  GL_CHECK(glViewport(0, 0, theWidth, theHeight));

  return result;
  }

void MDRRenderTarget::unbindRT() const
  {
  theRenderable->unbindRT();
  }

void MDRRenderTarget::bindTexture() const
  {
  theRenderable->bindTexture();
  }

bool MDRRenderTarget::destroy()
  {
  return theRenderable->destroy();
  }
