/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MParserINI.h"

#include "MString.h"

#define STATE_NEW_LINE      1
#define STATE_NEW_SECTION    2
#define STATE_NEW_VARIABLE    3
#define STATE_NEW_VALUE      4
#define STATE_END_SECTION    5
#define STATE_NEW_ASSIGNMENT  6
#define STATE_COMMENT      7

using namespace MaliSDK;

/* File data globals. */
static char *m_pData = NULL;
static int m_lLength = 0;

/* State globals. */
static int m_lPos = 0;
static char m_cChar = '\0';
static int m_iState = STATE_NEW_LINE;


/* Get the next character, checking for EOF. */
static char getChar()
{
  char cChar = m_pData[m_lPos++];
  if(m_lPos > m_lLength)
  {
    LOGE("EOF\n");
    exit(1);
  }
  return cChar;
}



/* We found an assignment, consume it. */
static void parseAssignment()
{
  if(m_cChar == '=')
  {
    m_iState = STATE_NEW_VALUE;
    m_cChar = getChar(); /* Consume '='. */
  }
  else
  {
    LOGE("Unexpected character '%c' in file at position %i\n", m_cChar, m_lPos - 1);
    exit(1);
  }
}


MParserINI::MParserINI()
    :
    theHandler(NULL),
    //
    theVariable(),
    theValue(),
    theSection(),
    //
    theString()
  {
  }

MParserINI::~MParserINI()
  {
  releaseMemory();
  }

/* We're past the '=' assignment so parse the value. */
void MParserINI::parseValue()
{
  int bEnd = 0;
  theString.clear();

  while(!bEnd)
  {
    switch(m_cChar)
    {
      case ' ':
      case '\t':
        /* Skip whitespace. */
        while((m_cChar == '\t') || (m_cChar == ' ')) m_cChar = getChar();
        break;

      case '\n':
      case '\r':
      case ';':
        /* End of value. */
        /* NB This could be done with a growing string helper here. */
        theValue = theString;
        /* Now we have everything for a definition, handle it. */
        theHandler->addDefinition(theSection, theVariable, theValue);
        /* Finish the line. */
        if(m_cChar == '\n' || m_cChar == '\r')
        {
          m_cChar = getChar(); /* Consume newline. */
          m_iState = STATE_NEW_LINE;
        }
        else
        {
          m_iState = STATE_COMMENT;
        }
        bEnd = 1;
        break;

      default:
        /* Find value. */
        if(((m_cChar >= 'A') && (m_cChar <= 'Z')) || ((m_cChar >= 'a') && (m_cChar <= 'z')) || m_cChar == '-' || m_cChar == '+' || m_cChar == '_' || m_cChar == ',' || m_cChar == '.' || ((m_cChar >= '0') && (m_cChar <= '9')))
        {
          theString.append(m_cChar);
          m_cChar = getChar();
        }
        else
        {
          LOGE("Unexpected character '%c' in file at position %i\n", m_cChar, m_lPos - 1);
          exit(1);
        }
        break;
    }
  }
}



/* Found the start of a variable - parse it. */
void MParserINI::parseVariable()
{
  int bEnd = 0;
  theString.clear();

  while(!bEnd)
  {
    switch(m_cChar)
    {
      case ' ':
      case '\t':
      case '=':
        /* Skip whitespace. */
        if((m_cChar == ' ') || (m_cChar == '\t'))
        {
          while((m_cChar == '\t') || (m_cChar == ' ')) m_cChar = getChar();
        }
        else
        {
          theVariable = theString;
          bEnd = 1;
          m_iState = STATE_NEW_ASSIGNMENT;
        }
        break;

      default:
        /* Find variable. */
        if(((m_cChar >= 'A') && (m_cChar <= 'Z')) || ((m_cChar >= 'a') && (m_cChar <= 'z')) || m_cChar == '_')
        {
          theString.append(m_cChar);
          m_cChar = getChar();
        }
        else
        {
          LOGE("Unexpected character '%c' in file at position %i\n", m_cChar, m_lPos - 1);
          exit(1);
        }
        break;
    }
  }
}



/* A section has started, parse its name. */
void MParserINI::parseSection()
{
  int bEnd = 0;
  theString.clear();

  while(!bEnd)
  {
    switch(m_cChar)
    {
      case ']':
        /* End of section. */
        /* NB This could be done with a growing string helper here. */
        theSection = theString;
        /* Call generic handler for new section. */
        theHandler->addSection(theSection);
        m_cChar = getChar(); /* Consume ']'. */
        m_iState = STATE_END_SECTION;
        bEnd = 1;
        break;

      default:
        /* Find section. */
        if(((m_cChar >= 'A') && (m_cChar <= 'Z')) || ((m_cChar >= 'a') && (m_cChar <= 'z')) || m_cChar == '_')
        {
          theString.append(m_cChar);
          m_cChar = getChar();
        }
        else
        {
          LOGE("Unexpected character '%c' in file at position %i\n", m_cChar, m_lPos - 1);
          exit(1);
        }
        break;
    }
  }
}



/* Found the end of a section, consume to the end of the current line. */
static void parseEndLine()
{
  int bEnd = 0;

  while(!bEnd)
  {
    switch(m_cChar)
    {
      case '\n':
      case '\r':
        m_cChar = getChar(); /* Consume newline. */
        m_iState = STATE_NEW_LINE;
        bEnd = 1;
        break;

      case ' ':
      case '\t':
        /* Skip whitespace. */
        while((m_cChar == ' ') || (m_cChar == '\t')) m_cChar = getChar();
        break;

      case ';':
        /* Skip comment. */
        m_iState = STATE_COMMENT;
        bEnd = 1;
        break;

      default:
        LOGE("Unexpected character '%c' in file at position %i\n", m_cChar, m_lPos - 1);
        exit(1);
        break;
    }
  }
}



/* Consume the comment until the end of the current line. */
static void parseComment()
{
  while(m_cChar != '\n') m_cChar = getChar();
  m_cChar = getChar(); /* Consume newline. */
  m_iState = STATE_NEW_LINE;
}



/* Parse from the start of a new line. */
static void parseLine()
{
  int bEnd = 0;

  /* Parse a line. */
  while(!bEnd)
  {
    switch(m_cChar)
    {
      /* Skip white space. */
      case ' ':
      case '\t':
        while((m_cChar == ' ') || (m_cChar == '\t')) m_cChar = getChar();
        break;

      /* Skip whole-line comments. */
      case ';':
        m_iState = STATE_COMMENT;
        bEnd = 1;
        break;

      /* Catch end of line. */
      case '\n':
      case '\r':
        m_cChar = getChar(); /* Consume newline. */
        m_iState = STATE_NEW_LINE;
        bEnd = 1;
        break;

      /* Find sections. */
      case '[':
        m_cChar = getChar(); /* Consume '['. */
        m_iState = STATE_NEW_SECTION;
        bEnd = 1;
        break;

      default:
        /* Find variables. */
        if(((m_cChar >= 'A') && (m_cChar <= 'Z')) || ((m_cChar >= 'a') && (m_cChar <= 'z')) || m_cChar == '_')
        {
          m_iState = STATE_NEW_VARIABLE;
          bEnd = 1;
        }
        else
        {
          LOGE("Unexpected character '%c' in file at position %i\n", m_cChar, m_lPos - 1);
          exit(1);
        }
        break;
    }
  }
}

/* Parse the file. */
bool MParserINI::parse(
    const MPath& aFilename)
  {
  int bEnd = 0;
  int iError = 0;

  if (theHandler == NULL)
    {
    LOGE("MParserINI::parse(%s) failed because theHandler is NULL!\n", (const char*)aFilename.getData());
    return false;
    }

  m_lPos = 0;
  iError = loadFile(aFilename.getData(), &m_pData, &m_lLength);
  if(iError != 0)
    {
    LOGE("loadFile() failed with %i\n", iError);
    return false;
    }

  theString.clear();
  m_cChar = getChar();

  while(!bEnd)
    {
    switch(m_iState)
      {
      case STATE_NEW_LINE:
        /* We're at the start of a line. */
        if(m_lPos < (m_lLength - 1))
          parseLine();
        else
          /* End of file. */
          bEnd = 1;
      break;

      case STATE_COMMENT:
        parseComment();
      break;

      case STATE_NEW_SECTION:
        parseSection();
      break;

      case STATE_END_SECTION:
        parseEndLine();
      break;

      case STATE_NEW_ASSIGNMENT:
        parseAssignment();
      break;

      case STATE_NEW_VALUE:
        parseValue();
      break;

      case STATE_NEW_VARIABLE:
        parseVariable();
      break;

      default:
        LOGE("Unknown state in parser.\n");
        releaseMemory();
        return false;
      break;
      }
    }

  // Clean memory
  releaseMemory();

  return true;
  }

void MParserINI::releaseMemory()
  {
  theString.clear();

  free(m_pData);
  m_pData = NULL;
  m_lLength = 0;
  }

/* Load the specified file into freshly allocated memory and return the pointer. */
int MParserINI::loadFile(const char* pFilename, char **ppData, int *pLength)
{
  FILE *pFile = NULL;
  char *pData = NULL;
  int lLength = 0;
  size_t sRead = 0;

  /* Open file for reading. */
  pFile = fopen(pFilename, "rb");
  if(pFile == NULL) return 1;

  /* Seek to end. */
  if(fseek(pFile, 0, SEEK_END) != 0)
  {
    fclose(pFile);
    return 2;
  }

  /* How long? */
  lLength = ftell(pFile);
  if(lLength == -1L)
  {
    fclose(pFile);
    return 3;
  }

  /* Back to beginning. */
  if(fseek(pFile, 0, SEEK_SET) != 0)
  {
    fclose(pFile);
    return 4;
  }

  /* Allocate memory. */
  pData = (char *)calloc(lLength, sizeof(char));
  if(pData == NULL)
  {
    fclose(pFile);
    return 5;
  }

  /* Read in. */
  sRead = fread(pData, sizeof(char), lLength, pFile);
  if(sRead != lLength)
  {
    free(pData);
    fclose(pFile);
    return 6;
  }

  /* Close file. */
  if(fclose(pFile) != 0)
  {
    free(pData);
    return 7;
  }

  *pLength = lLength;
  *ppData = pData;
  return 0;
}

