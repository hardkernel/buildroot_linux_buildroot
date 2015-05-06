/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_RENDERER_HPP
#define M_FURDR_RENDERER_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MDRRendererTexture.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a very simplified renderer object which holds several states such as:
 * frame number, current texture, viewport, etc.
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

  /// Returns a frame counter, which is used for simple time measuring
  unsigned int getFrameCounter() const
    { return theFrameCounter; }

  // ----- Miscellaneous -----

  /// Initialize rendering states like: blending, depth operations, culling etc.
  bool initialize(unsigned int aWidth,
                  unsigned int aHeight);

  /// The method resets all necessary states in an every frame. Actually it clears a framebuffer only.
  void preFrameRender();

  /// The method increses theFrameCounter.
  void postFrameRender();

  /// The method activate a given texture on a selected texture unit.
  void activateTexture(MDRTexUnit aUnit,
                       const MDRRendererTexture& aTexture);

private:

  // ----- Fields -----

  //
  unsigned int theWidth;
  unsigned int theHeight;
  //
  unsigned int theFrameCounter;

  };

#endif
