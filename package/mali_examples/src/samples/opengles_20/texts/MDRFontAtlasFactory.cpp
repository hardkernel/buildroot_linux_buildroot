/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRFontAtlasFactory.h"

#include "MDRFontAtlas.h"
#include "MParserINI.h"
#include "MDRParserINIHandlerFontAtlas.h"

MDRFontAtlas* MDRFontAtlasFactory::load(
    const MPath& aFileINI)
  {
  MString fullPath(aFileINI);
  //
  MDRFontAtlas* fontAtlas = new MDRFontAtlas;
  MDRParserINIHandlerFontAtlas handler(*fontAtlas);
  // Create INI parser and set handler
  MParserINI parser;
  parser.setHandler(&handler);
  if (!parser.parse(fullPath))
    {
    // In case parser failed, return NULL value of a font atlas instance
    delete fontAtlas;
    fontAtlas = NULL;
    }
  return fontAtlas;
  }
