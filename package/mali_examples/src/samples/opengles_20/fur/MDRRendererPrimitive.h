/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_RENDERER_PRIMITIVE_HPP
#define M_FURDR_RENDERER_PRIMITIVE_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MArray.h"
#include "MVector3.h"
#include "MBox.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents RAW vertices and thiers attributes data.
 * The primitive object can be percived as one draw call.
 */
class MDRRendererPrimitive
  {
public:

  // ----- Types -----

  ///
  enum MDRAttrib
    {
    ///
    ATTRIB_VERTICES = 0,
    ///
    ATTRIB_COLORS = 1,
    ///
    ATTRIB_TEXCOORDS0 = 2,
    ///
    ATTRIBS_COUNT
    };

  // ----- Constructors and destructors -----

  ///
  MDRRendererPrimitive();

  ///
  ~MDRRendererPrimitive();

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  /// The method appends glyphs definition with a given attributes
  void appendBox(MDRAttrib aAttrib,
                 const MBoxf& aBox);

  /// The method appends indices of vertices being used for glDrawElements
  bool setAttribIndex(MDRAttrib aAttrib,
                      int aIndex);

  /// Remove all appended glyphs so far.
  void clearAll();

  /// The method renders all attached glyphs
  void render();

private:

  // ----- Types -----

  ///
  typedef MArray<MVector3f> MDRArrayf;

  // ----- Fields -----

  //
  GLuint theBufferHandle;
  int theAttribs[ATTRIBS_COUNT];
  //
  MDRArrayf theData[ATTRIBS_COUNT];

  };

#endif
