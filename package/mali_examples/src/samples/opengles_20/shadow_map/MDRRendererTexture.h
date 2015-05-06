/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_RENDERER_TEXTURE_HPP
#define M_SHADOWMAPDR_RENDERER_TEXTURE_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MDRRenderable.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class unlike the MDRFBO represents virtual render target based on 2D texture functionality.
 * Instead of using FBO it simply copy framebuffer to texture.
 */
class MDRRendererTexture : public MDRRenderable
  {
public:

  // ----- Types -----

  ///
  typedef MDRRenderable BaseClass;

  // ----- Constructors and destructors -----

  ///
  MDRRendererTexture(bool aUseMipmap = true);

  ///
  virtual ~MDRRendererTexture();

  // ----- Accessors and mutators -----

  /// Returns the current native GL name of a texture.
  GLuint getName() const
    { return theName[theIndex]; }

  /// Checks if the texture has already been created.
  bool isCreated() const
    { return (theName != 0); }

  // ----- Miscellaneous -----

  /// The method creates a 2D texture with specified size and data to be uploaded during the texture creation*/
  bool create(unsigned int aWidth,
              unsigned int aHeight,
              const unsigned char* aData);

  /// The method calls the create method with aData = NULL
  virtual bool initialize(unsigned int aWidth,
                          unsigned int aHeight);

  /// It swaps the current index of a texture used for rendering and the other to be updated.
  /// In other words, it rotates the ring of textures.
  virtual bool bindRT() const;

  /// Copies the framebuffer content into the texture
  virtual void unbindRT() const;

  /// Bind the current texture to be rendered on top of primitives
  virtual void bindTexture() const;

  /// Remove the texture
  virtual bool destroy();

private:

  // ----- Fields -----

  //
  enum
    {
    // Define a quantity of textures in a ring.
    TEXTURES_COUNT = 2
    };

  //
  const GLuint theTarget;
  unsigned int theWidth;
  unsigned int theHeight;
  bool theUseMipmap;
  //
  mutable unsigned int theIndex;
  //
  GLuint theName[TEXTURES_COUNT];

  };

#endif
