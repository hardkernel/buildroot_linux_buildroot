/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_PARSER_INI_HPP
#define M_TEXTSDR_PARSER_INI_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MParserINIHandler.h"
#include "MString.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

class MParserINI
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MParserINI();

  ///
  ~MParserINI();

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  ///
  void setHandler(MParserINIHandler* aHandler)
    { theHandler = aHandler; }

  ///
  bool parse(const MPath& aFile);

private:

  // ----- Fields -----

  //
  MParserINIHandler* theHandler;
  //
  MString theVariable;
  MString theValue;
  MString theSection;
  //
  MString theString;

  // ----- Miscellaneous -----

  //
  void releaseMemory();

  //
  void parseValue();

  //
  void parseVariable();

  //
  void parseSection();

  //
  int loadFile(const char *pFilename, char **ppData, int *pLength);

  };

#endif
