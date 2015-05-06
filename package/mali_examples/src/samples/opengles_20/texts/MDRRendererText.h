/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_RENDERER_TEXT_HPP
#define M_TEXTSDR_RENDERER_TEXT_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MMatrix.h"
#include "MDRRenderer.h"
#include "MDRRendererPrimitive.h"

class MDRRendererProgram;
class MDRFontAtlas;

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a text object.
 */
class MDRRendererText
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MDRRendererText(MDRRenderer& aRenderer);

  ///
  ~MDRRendererText();

  // ----- Accessors and mutators -----

  /// The method sets characters. After characters are being set, the prepare method must be called.
  void setString(const char* aString);

  /// Returns characters of the text.
  const char* getString() const
    { return theString; }

  /// Sets reference of the program to be used for the text rendering
  void setProgramRef(const MDRRendererProgram* aProgram);

  /// Sets reference of an atlas to be used for the text rendering
  void setFontRef(const MDRFontAtlas* aFontAtlas)
    { theFontAtlasRef = aFontAtlas; }

  /// Sets position of the object
  void setPosition(const MVector3f& aPosition)
    { thePosition = aPosition; }

  /// Returns position of the object.
  const MVector3f& getPosition() const
    { return thePosition; }

  /// Sets a rotation of the object
  void setRotation(const MVector3f& aRotation)
    { theRotation = aRotation; }

  /// Gets an actual rotation of the object
  const MVector3f& getRotation() const
    { return theRotation; }

  /// Sets a pivot, according which the rotation is being done.
  /// By default the text is rotated according to the first glyph position
  void setPivot(const MVector3f& aPivot)
    { thePivot = aPivot; }

  /// Returns an actual pivot of the object
  const MVector3f& getPivot() const
    { return thePivot; }

  // ----- Miscellaneous -----

  /// Rebuilds the whole primitive according to the characters set with the setString method.
  void prepare();

  /// Renders the text into a framebuffer
  void render();

private:

  // ----- Fields -----

  //
  MDRRenderer& theRenderer;
  //
  char *theString;
  unsigned int theStringLength;
  //
  MDRRendererPrimitive thePrimitive;
  //
  const MDRRendererProgram* theProgramRef;
  const MDRFontAtlas* theFontAtlasRef;
  //
  MMatrix4f theMatRotate;
  MMatrix4f theMatModelView;
  MMatrix4f theMatMVP;
  MMatrix4f theMatTex0;
  //
  MVector3f theRotation;
  MVector3f thePosition;
  MVector3f thePivot;

  };

#endif
