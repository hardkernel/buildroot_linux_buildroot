/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_RENDER_TARGET_HPP
#define M_SHADOWMAPDR_RENDER_TARGET_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MDRRenderable.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents two types of render targets such as: Frame Buffer Object and Texture.
 * When the FBO mode is used all graphics go to the Frame Buffer Object,
 * whereas the Copy Texture mode is used all graphics is first rendered to the framebuffer and then copied to the texture.
 */
class MDRRenderTarget : public MDRRenderable
  {
public:

  // ----- Types -----

  ///
  enum MDRMode
    {
    ///
    MODE_FBO,
    ///
    MODE_COPY_TEXTURE
    };

  ///
  typedef MDRRenderable BaseClass;

  // ----- Constructors and destructors -----

  ///
  MDRRenderTarget(MDRMode aMode);

  ///
  virtual ~MDRRenderTarget();

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  /// The method creates either FBO or Texture with a specified size. 
  virtual bool initialize(unsigned int aWidth,
                          unsigned int aHeight);

  /// The method activates FBO if FBO mode is active, otherwise doesn't do anything.
  virtual bool bindRT() const;

  /// The method activates framebuffer (0) as a current render target
  virtual void unbindRT() const;

  /// The method binds the render target as a texture.
  virtual void bindTexture() const;

  /// The method releases GPU resources either FBO or Texture.
  virtual bool destroy();


private:

  // ----- Fields -----

  //
  unsigned int theWidth;
  unsigned int theHeight;
  MDRRenderable* theRenderable;
  
  };

#endif
