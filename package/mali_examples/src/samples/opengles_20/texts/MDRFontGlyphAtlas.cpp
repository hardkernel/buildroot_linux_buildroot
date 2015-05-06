/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRFontGlyphAtlas.h"

MDRFontGlyphAtlas::MDRFontGlyphAtlas(
    unsigned char aCharacter,
    float aLeft,
    float aBottom,
    float aRight,
    float aTop,
    float aBearingLeft,
    float aBearingTop)
    :
    theCharacter(aCharacter),
    theLeft(aLeft),
    theBottom(aBottom),
    theRight(aRight),
    theTop(aTop),
    theBearingLeft(aBearingLeft),
    theBearingTop(aBearingTop),
    //
    theBoxGeometry(),
    theBoxMapping(),
    theBearing(),
    //
    theIsUpdateNeeded(true)
  {
  update();
  }

MDRFontGlyphAtlas::~MDRFontGlyphAtlas()
  {
  }

const MBoxf& MDRFontGlyphAtlas::getBoxGeometry() const
  {
  update();
  return theBoxGeometry;
  }

const MBoxf& MDRFontGlyphAtlas::getBoxMapping() const
  {
  update();
  return theBoxMapping;
  }

const MVector3f& MDRFontGlyphAtlas::getBearing() const
  {
  update();
  return theBearing;
  }

void MDRFontGlyphAtlas::setLeft(
    float aValue)
  {
  theIsUpdateNeeded = true;
  theLeft = aValue;
  }

void MDRFontGlyphAtlas::setBottom(float aValue)
  {
  theIsUpdateNeeded = true;
  theBottom = aValue;
  }

void MDRFontGlyphAtlas::setRight(float aValue)
  {
  theIsUpdateNeeded = true;
  theRight = aValue;
  }

void MDRFontGlyphAtlas::setTop(float aValue)
  {
  theIsUpdateNeeded = true;
  theTop = aValue;
  }

void MDRFontGlyphAtlas::setBearingLeft(float aValue)
  {
  theIsUpdateNeeded = true;
  theBearingLeft = aValue;
  }

void MDRFontGlyphAtlas::setBearingTop(float aValue)
  {
  theIsUpdateNeeded = true;
  theBearingTop = aValue;
  }

void MDRFontGlyphAtlas::update() const
  {
  if (theIsUpdateNeeded == false)
    return;
  //
  float w = theRight - theLeft;
  float h = theTop - theBottom;
  theBoxGeometry.getMin().set(0.0f, 0.0f, 0.0f);
  theBoxGeometry.getMax().set(w, h, 0.0f);
  //
  theBoxMapping.getMin().set(theLeft, 1.0f - theTop, 0.0f);
  theBoxMapping.getMax().set(theRight, 1.0f - theBottom, 0.0f);
  //
  theBearing.set(theBearingLeft, theBearingTop, 0.0f);
  //
  theIsUpdateNeeded = true;
  }