/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_PARSER_INI_HANDLER_FONT_ATLAS_HPP
#define M_TEXTSDR_PARSER_INI_HANDLER_FONT_ATLAS_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MParserINIHandler.h"

#include "MDRFontAtlas.h"
#include "MVector2.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

class MDRParserINIHandlerFontAtlas : public MParserINIHandler
  {
public:

  // ----- Types -----

  ///
  typedef MParserINIHandler BaseClass;

  // ----- Constructors and destructors -----

  ///
  MDRParserINIHandlerFontAtlas(MDRFontAtlas& aFontAtlas);

  ///
  virtual ~MDRParserINIHandlerFontAtlas();

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

protected:

  // ----- Miscellaneous -----

  /* Parser calls this handler when a new section is found. */
  virtual void addSection(const MString& aSection);

  /* Parser calls this handler when a new definition is found. */
  /* Should be generic enough to be called from the parser with no format-specific info. */
  virtual void addDefinition(const MString& aSection,
                             const MString& aVariable,
                             const MString& aValue);

private:

  // ----- Fields -----

  //
  MDRFontAtlas& theFontAtlas;
  //
  unsigned int theImageWidth;
  unsigned int theImageHeight;

  // ----- Miscellaneous -----

  //
  bool parseAsVector(const MString& aStr,
                     MString::CharType aSeparator,
                     MVector2i& aOutVec) const;

  };

#endif
