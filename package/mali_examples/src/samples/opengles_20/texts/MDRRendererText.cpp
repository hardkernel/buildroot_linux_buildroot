/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRRendererText.h"

#include "MBox.h"
#include "MDRFontAtlas.h"
#include "MDRFontGlyphAtlas.h"
#include "MDRRendererProgram.h"

MDRRendererText::MDRRendererText(
    MDRRenderer& aRenderer)
    :
    theRenderer(aRenderer),
    //
    theString(NULL),
    theStringLength(0),
    //
    theProgramRef(NULL),
    theFontAtlasRef(NULL),
    //
    theMatRotate(),
    theMatModelView(),
    theMatMVP(),
    theMatTex0(),
    //
    theRotation(),
    thePosition(),
    thePivot()
  {
  }

MDRRendererText::~MDRRendererText()
  {
  }

void MDRRendererText::setString(
    const char* aString)
  {
  // Copy the string and write a zero at the end
  theStringLength = (unsigned int)strlen(aString);
  theString = (char*)realloc(theString, theStringLength + 1);
  memcpy(theString, aString, theStringLength);
  theString[theStringLength] = 0;
  }

void MDRRendererText::setProgramRef(
    const MDRRendererProgram* aProgram)
  {
  theProgramRef = aProgram;
  //
  thePrimitive.setAttribIndex(MDRRendererPrimitive::ATTRIB_VERTICES, theProgramRef->getLocAttrib(MDRRendererProgram::ATTRIB_LOC_VERTEX));
  thePrimitive.setAttribIndex(MDRRendererPrimitive::ATTRIB_COLORS, theProgramRef->getLocAttrib(MDRRendererProgram::ATTRIB_LOC_COLOR));
  thePrimitive.setAttribIndex(MDRRendererPrimitive::ATTRIB_TEXCOORDS0, theProgramRef->getLocAttrib(MDRRendererProgram::ATTRIB_LOC_TEXCOORD0));
  }

void MDRRendererText::prepare()
  {
  if (theFontAtlasRef == NULL ||
      theString == NULL)
    return;

  thePrimitive.clearAll();
  // glyphCursor is used to translate glyph to the subseqent position
  MVector3f glyphCursor(0.0f, 0.0f, 0.0f);
  // Iterate through all characters and build primitive based on information for FontAtlas
  for (unsigned int charIndex = 0; charIndex < theStringLength; charIndex++)
    {
    // Find glyph for the character on the atlas
    const MDRFontGlyphAtlas* glyph = theFontAtlasRef->findGlyph(theString[charIndex]);
    // If glyph exist, add vertices to the primitive based on data from the glyph
    if (glyph)
      {
      MBoxf glyphGeo = glyph->getBoxGeometry();
      glyphGeo.translate(glyphCursor);
      glyphGeo.translate(glyph->getBearing());
      //
      thePrimitive.appendBox(MDRRendererPrimitive::ATTRIB_VERTICES,   glyphGeo);
      thePrimitive.appendBox(MDRRendererPrimitive::ATTRIB_TEXCOORDS0, glyph->getBoxMapping());
      //
      glyphCursor[0] += glyph->getBoxGeometry().getWidth();
      }
    }
  }

void MDRRendererText::render()
  {
  if (theProgramRef == NULL)
    return;

  /* Reset matrices */
  theMatRotate.setIdentity();
  theMatModelView.setIdentity();

  /* Apply Pivot */
  theMatModelView.applyTranslation(MVector3f(-thePivot[0], -thePivot[1], -thePivot[2]));

  /* Apply rotation around Pivot point */
  theMatRotate.setRotation((float)theRotation[0], 1.0f, 0.0f, 0.0f);
  theMatModelView.multiplyLeft(theMatRotate);
  theMatRotate.setRotation((float)theRotation[1], 0.0f, 1.0f, 0.0f);
  theMatModelView.multiplyLeft(theMatRotate);
  theMatRotate.setRotation((float)theRotation[2], 0.0f, 0.0f, 1.0f);
  theMatModelView.multiplyLeft(theMatRotate);

  /* Return back from pivot point */
  theMatModelView.applyTranslation(thePivot);

  /* Apply object position */
  theMatModelView.applyTranslation(thePosition);

  /* Prepare final matrix MVP  */
  theMatMVP = theMatModelView;
  theMatMVP.multiplyLeft(theRenderer.getMatrixP());

  /* Enable program and set ModelViewProjection matrix */
  theProgramRef->use();
  theProgramRef->setUniform(MDRRendererProgram::UNIFORM_LOC_MATRIX_MVP, theMatMVP);
  theProgramRef->setUniform(MDRRendererProgram::UNIFORM_LOC_MATRIX_T0, theMatTex0);

  /* Draw the text */
  thePrimitive.render();
  }
