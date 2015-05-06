/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRFBO.h"

using namespace MaliSDK;

MDRFBO::MDRFBO()
    :
    BaseClass(),
    //
    theNameFBO(0),
    theNameTexture(0),
    theNameRenderBuffer(0),
    //
    theWidth(0),
    theHeight(0)
  {
  }

MDRFBO::~MDRFBO()
  {
  destroy();
  }

bool MDRFBO::initialize(
    unsigned int aWidth,
    unsigned int aHeight)
  {
  theWidth = aWidth;
  theHeight = aHeight;

  /* Initialize textures. */
  GL_CHECK(glGenTextures(1, &theNameTexture));

  /* Create a texture, which is going to be used as a render target later on. */
  GL_CHECK(glActiveTexture(GL_TEXTURE0));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, theNameTexture));
#undef LOAD_TEXTURE
#ifdef LOAD_TEXTURE
  loadData("assets/texture.raw", &pTexData);
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, theWidth, theHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTexData));
  free(pTexData);
  pTexData = NULL;
#else /* LOAD_TEXTURE */
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, theWidth, theHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
#endif /* LOAD_TEXTURE */
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)); /* Nearest for both for now for simple hard-edge shadows. */

  /* Initialize FBOs. */
  GL_CHECK(glGenFramebuffers(1, &theNameFBO));

  /* Create a render buffers which is going to be attached to the framebuffer object. */
  GL_CHECK(glGenRenderbuffers(1, &theNameRenderBuffer));
  GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, theNameRenderBuffer));
  GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, theWidth, theHeight));

  /* Bind theNameFBO framebuffer object for rendering. */
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, theNameFBO));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, theNameTexture));

  /* Attach texture to the framebuffer. */
  GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, theNameRenderBuffer));
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, theNameTexture, 0));

  /* Check FBO is OK. */
  int iResult = GL_CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER));

  /* Unbind the texture. */
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

  return (GL_FRAMEBUFFER_COMPLETE == iResult);
  }

bool MDRFBO::bindRT() const
  {
  /* Bind theNameFBO framebuffer object for rendering. */
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, theNameFBO));
  return (theNameFBO != 0);
  }

void MDRFBO::unbindRT() const
  {
  /* Bind the framebuffer for rendering - unbind FBO. */
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  }

void MDRFBO::bindTexture() const
  {
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, theNameTexture));
  }

bool MDRFBO::destroy()
  {
  // Delete the RenderBuffer
  if (theNameRenderBuffer)
    {
    GL_CHECK(glDeleteRenderbuffers(1, &theNameRenderBuffer));
    theNameRenderBuffer = 0;
    }

  // Delete the Texture
  if (theNameTexture)
    {
    GL_CHECK(glDeleteTextures(1, &theNameTexture));
    theNameTexture = 0;
    }

  // Delete the FBO
  if (theNameFBO)
    {
    GL_CHECK(glDeleteFramebuffers(1, &theNameFBO));
    theNameFBO = 0;
    }

  return true;
  }
