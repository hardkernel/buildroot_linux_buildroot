/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_RENDERER_HPP
#define M_SHADOWMAPDR_RENDERER_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MMatrix.h"
#include "MDRRendererTexture.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a very simplified renderer object which holds several states such as:
 * frame number, current texture, viewport, projection matrix etc.
 */
class MDRRenderer
  {
public:

  // ----- Types -----

  ///
  enum MDRTexUnit
    {
    ///
    TEXUNIT0,
    ///
    TEXUNIT1,
    ///
    TEXUNIT2,
    ///
    TEXUNIT3
    };

  // ----- Constructors and destructors -----

  ///
  MDRRenderer();

  ///
  ~MDRRenderer();

  // ----- Accessors and mutators -----

  /// Returns width of a current viewport
  unsigned int getWidth() const
    { return theWidth; }

  /// Returns height of a current viewport
  unsigned int getHeight() const
    { return theHeight; }

  /// Returns a current projection matrix
  const MMatrix4f& getMatrixP() const
    { return theMatrixP; }

  /// Returns a frame counter, which is used for simple time measuring
  unsigned int getFrameCounter() const
    { return theFrameCounter; }

  // ----- Miscellaneous -----

  /// Initialize renderer with a given size of viewport expressed in pixels
  bool initialize(unsigned int aWidth,
                  unsigned int aHeight);

  /// The method resets all necessary states in every frame. In fact it clears a framebuffer only now.
  void preFrameRender();

  /// The method increses theFrameCounter
  void postFrameRender();

  /// The method activate a given texture on a selected texture unit
  void activateTexture(MDRTexUnit aUnit,
                       const MDRRendererTexture& aTexture);

private:

  // ----- Fields -----

  //
  unsigned int theWidth;
  unsigned int theHeight;
  // Projective matrix is calculated automatically in initialize method
  MMatrix4f theMatrixP;
  //
  unsigned int theFrameCounter;

  };

#endif
