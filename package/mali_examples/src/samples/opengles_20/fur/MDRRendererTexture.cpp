/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRRendererTexture.h"

using namespace MaliSDK;

MDRRendererTexture::MDRRendererTexture()
    :
    theName(0)
  {
  }

MDRRendererTexture::~MDRRendererTexture()
  {
  destroy();
  }

bool MDRRendererTexture::create(
    unsigned int aWidth,
    unsigned int aHeight,
    const unsigned char* aData)
  {
  /* Load just base level texture data. */
  if (theName == 0)
    {
    GL_CHECK(glGenTextures(1, &theName));
    }

  GL_CHECK(glBindTexture(GL_TEXTURE_2D, theName));
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, aWidth, aHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, aData));

  /* Set texture mode. */
  GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)); /* Default anyway. */
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

  return true;
  }

bool MDRRendererTexture::destroy()
  {
  if (theName == 0)
    return true;
  //
  GL_CHECK(glDeleteTextures(1, &theName));
  theName = 0;
  //
  return true;
  }
