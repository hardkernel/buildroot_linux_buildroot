/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_FONT_ATLAS_FACTORY_HPP
#define M_TEXTSDR_FONT_ATLAS_FACTORY_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MDRFontAtlas.h"
#include "MString.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 *  The class is a factory, which creates font atlases.
 *  The factory loads INI files and based on the information contained in the files
 *  loads all required definitions of glyphs as well as an image associated to those
 *  glyphs.
 */
class MDRFontAtlasFactory
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  // ----- Accessors and mutators -----

  /// Create an instance of a font atlas based on a given INI file.
  static MDRFontAtlas* load(const MPath& aFileINI);

  };

#endif
