/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_RENDERER_TEXTURE_HPP
#define M_FURDR_RENDERER_TEXTURE_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents an instance of 2D texture
 */
class MDRRendererTexture
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MDRRendererTexture();

  ///
  ~MDRRendererTexture();

  // ----- Accessors and mutators -----

  /// Returns a GL native handle to the texture instance.
  GLuint getName() const
    { return theName; }

  /// The method checks if the texture is already created.
  bool isCreated() const
    { return (theName != 0); }

  // ----- Miscellaneous -----

  /// The method creates a 2D texture with a given size (width and height) and then uploads a given data.
  /// If there is no data to be uploaded to the textute then aData should be NULL.
  bool create(unsigned int aWidth,
              unsigned int aHeight,
              const unsigned char* aData);

  /// The method removes the texture (only GL instance is removed).
  /// The instance of this object remains untouched.
  bool destroy();

private:

  // ----- Fields -----

  // The native name/handle of the textute
  GLuint theName;

  };

#endif
