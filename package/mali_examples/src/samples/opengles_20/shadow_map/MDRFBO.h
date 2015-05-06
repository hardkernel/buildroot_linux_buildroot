/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_FBO_HPP
#define M_SHADOWMAPDR_FBO_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MDRRenderable.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents wrapper over Frame Buffer Object. 
 */
class MDRFBO : public MDRRenderable
  {
public:

  // ----- Types -----

  ///
  typedef MDRRenderable BaseClass;

  // ----- Constructors and destructors -----

  ///
  MDRFBO();

  ///
  virtual ~MDRFBO();

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  /// Initialize / create the Frame Buffer Object
  virtual bool initialize(unsigned int aWidth,
                          unsigned int aHeight);

  /// Bind the FBO as render target. After the method is called all draw calls
  /// go to the FBO
  virtual bool bindRT() const;

  /// Unbind the FBO. Make framebuffer as current render target.
  virtual void unbindRT() const;

  /// Bind the FBO as a texture.
  virtual void bindTexture() const;

  /// Delete the FBO.
  virtual bool destroy();

private:

  // ----- Fields -----

  //
  GLuint theNameFBO;
  GLuint theNameTexture;
  GLuint theNameRenderBuffer;
  //
  unsigned int theWidth;
  unsigned int theHeight;

  };

#endif
