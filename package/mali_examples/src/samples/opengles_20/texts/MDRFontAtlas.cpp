/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRFontAtlas.h"

#include "MDRFontGlyphAtlas.h"


MDRFontAtlas::MDRFontAtlas()
    :
    theGlyphs(),
    theImage()
  {
  }

MDRFontAtlas::~MDRFontAtlas()
  {
  }

bool MDRFontAtlas::loadImage(
    const MPath& aFileNameTGA)
  {
  return theImage.load(aFileNameTGA);;
  }

const MDRFontGlyphAtlas* MDRFontAtlas::findGlyph(
    unsigned char aCharacter) const
  {
  MDRGlyphsArray::MDRConstIterator it  = theGlyphs.getBegin();
  MDRGlyphsArray::MDRConstIterator end = theGlyphs.getEnd();
  while (it != end)
    {
    const MDRFontGlyphAtlas& glyph = *it;
    //
    if (glyph.getCharacter() == aCharacter)
      return &glyph;
    //
    it++;
    }
  return NULL;
  }

MDRFontGlyphAtlas* MDRFontAtlas::getLastGlyph()
  {
  if (theGlyphs.getCount() == 0)
    return NULL;
  return &theGlyphs[theGlyphs.getCount() - 1];
  }

MDRFontGlyphAtlas* MDRFontAtlas::createNewGlyph()
  {
  MDRFontGlyphAtlas glyph(0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  theGlyphs.append(glyph);
  return &theGlyphs[theGlyphs.getCount() - 1];
  }
