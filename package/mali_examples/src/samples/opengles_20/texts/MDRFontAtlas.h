/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_FONT_ATLAS_HPP
#define M_TEXTSDR_FONT_ATLAS_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MArray.h"
#include "MImageTGA.h"
#include "MString.h"

class MDRFontAtlasFactory;
class MDRFontGlyphAtlas;
class MDRParserINIHandlerFontAtlas;

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a single instance of a font atlas
 * An instance of this class cannot be crated as standalone.
 * It is created internally by MDRFontAtlasFactory
 */
class MDRFontAtlas
  {
public:

  // ----- Constructors and destructors -----

  /// Unlike constructor, the destructor can be called by a client
  ~MDRFontAtlas();

  // ----- Accessors and mutators -----

  ///
  const MImageTGA& getImage() const
    { return theImage; }

  // ----- Miscellaneous -----

  /// The method returns a definition of a requested character like:
  /// pixel coordinates, bearing information and so on.
  const MDRFontGlyphAtlas* findGlyph(unsigned char aCharacter) const;

  // ----- Friends -----

  /// MDRFontAtlasFactory and MDRParserINIHandlerFontAtlas have access
  /// to protected and even private members of this class
  friend class MDRFontAtlasFactory;
  friend class MDRParserINIHandlerFontAtlas;

protected:

  // ----- Types -----

  // ----- Constructors and destructors -----

  /// The constructor can be called only from classes being in friendship (see above).
  MDRFontAtlas();

  // ----- Miscellaneous -----

  /// Load a given image from TGA file
  bool loadImage(const MPath& aFileNameTGA);

  /// One of the helper methods for creating the atlas
  MDRFontGlyphAtlas* getLastGlyph();

  /// Other helper method for creating the atlas
  MDRFontGlyphAtlas* createNewGlyph();

private:

  // ----- Types -----

  //
  typedef MArray<MDRFontGlyphAtlas> MDRGlyphsArray;

  // ----- Fields -----

  //
  MDRGlyphsArray theGlyphs;
  MImageTGA theImage;

  };

#endif
