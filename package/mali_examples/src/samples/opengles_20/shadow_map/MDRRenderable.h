/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_RENDERABLE_HPP
#define M_SHADOWMAPDR_RENDERABLE_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MDRRenderable.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents an abstract object, which could be used as a render target (e.g. FBO).
 */
class MDRRenderable
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MDRRenderable();

  ///
  virtual ~MDRRenderable();

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  ///
  virtual bool initialize(unsigned int aWidth,
                          unsigned int aHeight) = 0;

  ///
  virtual bool bindRT() const = 0;

  ///
  virtual void unbindRT() const = 0;

  ///
  virtual void bindTexture() const = 0;

  ///
  virtual bool destroy() = 0;


private:

  // ----- Fields -----

  };

#endif
