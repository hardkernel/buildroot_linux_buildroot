/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_FONT_GLYPH_ATLAS_HPP
#define M_TEXTSDR_FONT_GLYPH_ATLAS_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MBox.h"
#include "MVector3.h"

class MDRParserINIHandlerFontAtlas;

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/*
   The class represents a single glyph (graphical representation of a character) definition.
   All required input parameters come from an INI file, which is loaded by MDRAtlasFactory class.
*/
class MDRFontGlyphAtlas
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MDRFontGlyphAtlas(unsigned char aCharacter,
                    float aLeft,
                    float aBottom,
                    float aRight,
                    float aTop,
                    float aBearingLeft,
                    float aBearingTop);

  ///
  ~MDRFontGlyphAtlas();

  // ----- Accessors and mutators -----

  /* Returns a character code */
  unsigned char getCharacter() const
    { return theCharacter; }

  /* Returns geometry box representation. Each glyph is represented by different geometry */
  const MBoxf& getBoxGeometry() const;

  /* Returns the textrue mapping box. */
  const MBoxf& getBoxMapping() const;

  /* Returns glyph bearing informtion */
  const MVector3f& getBearing() const;

  // ----- Miscellaneous -----

  // ----- Friends -----

  ///
  friend class MDRParserINIHandlerFontAtlas;

protected:

  // ----- Setters and getters -----

  ///
  void setCharacter(unsigned char aCharacter)
    { theCharacter = aCharacter; }

  ///
  void setLeft(float aValue);

  ///
  void setBottom(float aValue);

  ///
  void setRight(float aValue);

  ///
  void setTop(float aValue);

  ///
  void setBearingLeft(float aValue);

  ///
  void setBearingTop(float aValue);

  ///
  MBoxf& getBoxGeometry()
    { return theBoxGeometry; }

  ///
  MBoxf& getBoxMapping()
    { return theBoxMapping; }

  ///
  MVector3f& getBearing()
    { return theBearing; }

  // ----- Miscellaneous -----

private:

  // ----- Fields -----

  //
  unsigned char theCharacter;
  float theLeft;
  float theBottom;
  float theRight;
  float theTop;
  float theBearingLeft;
  float theBearingTop;
  //
  mutable MBoxf theBoxGeometry;
  mutable MBoxf theBoxMapping;
  mutable MVector3f theBearing;
  //
  mutable bool theIsUpdateNeeded;

  // ----- Miscellaneous -----

  //
  void update() const;

  };

#endif
