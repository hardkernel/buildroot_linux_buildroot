/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_PARSER_INI_HANDLER_HPP
#define M_TEXTSDR_PARSER_INI_HANDLER_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MString.h"

class MParserINI;

//------------------------------------------
// BEGIN OF CLASS DECLARATION

class MParserINIHandler
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MParserINIHandler()
    {}

  ///
  virtual ~MParserINIHandler()
    {}

  // ----- Accessors and mutators -----

  // ----- Friends -----

  ///
  friend class MParserINI;

protected:

  // ----- Miscellaneous -----

  /* Parser calls this handler when a new section is found. */
  virtual void addSection(const MString& aSection) = 0;

  /* Parser calls this handler when a new definition is found. */
  /* Should be generic enough to be called from the parser with no format-specific info. */
  virtual void addDefinition(const MString& aSection,
                             const MString& aVariable,
                             const MString& aValue) = 0;

private:

  // ----- Fields -----

  };

#endif
