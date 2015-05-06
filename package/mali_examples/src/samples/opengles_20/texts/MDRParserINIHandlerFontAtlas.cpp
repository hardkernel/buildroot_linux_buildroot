/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRParserINIHandlerFontAtlas.h"

#include "MDRFontAtlas.h"
#include "MDRFontGlyphAtlas.h"
#include "MPathsManager.h"
#include "MVector2.h"

using namespace MaliSDK;

MDRParserINIHandlerFontAtlas::MDRParserINIHandlerFontAtlas(
    MDRFontAtlas& aFontAtlas)
    :
    BaseClass(),
    //
    theFontAtlas(aFontAtlas),
    //
    theImageWidth(0),
    theImageHeight(0)
  {
  }

MDRParserINIHandlerFontAtlas::~MDRParserINIHandlerFontAtlas()
  {
  }

void MDRParserINIHandlerFontAtlas::addSection(
    const MString& aSection)
  {
  if (aSection == MString("glyph"))
    theFontAtlas.createNewGlyph();
  }

void MDRParserINIHandlerFontAtlas::addDefinition(
    const MString& aSection,
    const MString& aVariable,
    const MString& aValue)
  {
  if (aSection == MString("file") &&
      aVariable == MString("filename"))
    {
    MPath fullPath = MPathsManager::getFullPathStatic(aValue);
    if (!theFontAtlas.loadImage(fullPath))
      {
      LOGE("[MDRParserINIHandlerFontAtlas::addDefinition] Cannot load file: %s!\n", aValue.getData());
      }
    theImageWidth = theFontAtlas.getImage().getWidth();
    theImageHeight = theFontAtlas.getImage().getHeight();
    }
  else if (aSection == MString("glyph"))
    {
    MVector2i v;
    MDRFontGlyphAtlas* currentGlyph = theFontAtlas.getLastGlyph();
    if (currentGlyph != NULL)
      {
      if (aVariable == MString("code"))
        {
        currentGlyph->setCharacter(aValue.getAsInt());
        }
      else if (aVariable == MString("min"))
        {
        parseAsVector(aValue, ',', v);
        currentGlyph->setLeft((float)v[0] / (float)theImageWidth);
        currentGlyph->setBottom((float)v[1] / (float)theImageHeight);
        }
      else if (aVariable == MString("max"))
        {
        parseAsVector(aValue, ',', v);
        currentGlyph->setRight((float)v[0] / (float)theImageWidth);
        currentGlyph->setTop((float)v[1] / (float)theImageHeight);
        }
      else if (aVariable == MString("top_left"))
        {
        parseAsVector(aValue, ',', v);
        currentGlyph->setBearingLeft((float)v[0] / (float)theImageWidth);
        currentGlyph->setBearingTop((float)v[1] / (float)theImageHeight);
        }
      }
    }
  else
    {
    LOGE("Unknown section '%s' at %s:%i\n", aSection.getData(), __FILE__, __LINE__);
    }
  }

bool MDRParserINIHandlerFontAtlas::parseAsVector(
    const MString& aStr,
    MString::CharType aSeparator,
    MVector2i& aOutVec) const
  {
  unsigned int index = aStr.find(aSeparator);
  if (index == MString::theNotFound)
    {
    return false;
    }
  //
  MString val1;
  aStr.sub(0, index - 1, val1);
  MString val2;
  aStr.sub(index + 1, aStr.getLength() - 1, val2);
  //
  aOutVec[0] = val1.getAsInt();
  aOutVec[1] = val2.getAsInt();
  //
  return true;
  }
