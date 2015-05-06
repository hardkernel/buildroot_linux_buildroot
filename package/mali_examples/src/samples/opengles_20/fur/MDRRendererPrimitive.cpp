/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRRendererPrimitive.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

using namespace MaliSDK;

MDRRendererPrimitive::MDRRendererPrimitive()
    :
    theBufferHandle(0)
  {
  memset(theAttribs, -1, ATTRIBS_COUNT - 1);
  }

MDRRendererPrimitive::~MDRRendererPrimitive()
  {
  }

void MDRRendererPrimitive::appendBox(
    MDRAttrib aAttrib,
    const MBoxf& aBox)
  {
  const MVector3f& lb = aBox.getMin(); // Left-Bottom
  const MVector3f& rt = aBox.getMax(); // Right-Top
  //
  theData[aAttrib].append(lb);
  theData[aAttrib].append(MVector3f(lb[0], rt[1], (lb[2] + rt[2]) / 2.0f));
  theData[aAttrib].append(rt);
  theData[aAttrib].append(lb);
  theData[aAttrib].append(rt);
  theData[aAttrib].append(MVector3f(rt[0], lb[1], (lb[2] + rt[2]) / 2.0f));
  }

void MDRRendererPrimitive::clearAll()
  {
  for (unsigned int attrib = 0; attrib < ATTRIBS_COUNT; attrib++)
    theData[attrib].clear();
  }

bool MDRRendererPrimitive::setAttribIndex(
    MDRAttrib aAttrib,
    int aIndex)
  {
  if (aAttrib >= ATTRIBS_COUNT)
    return false;
  theAttribs[aAttrib] = aIndex;
  return true;
  }

void MDRRendererPrimitive::render()
  {
  for (unsigned int attrib = 0; attrib < ATTRIBS_COUNT; attrib++)
    {
    if (theAttribs[attrib] >= 0)
      {
      GL_CHECK(glEnableVertexAttribArray(theAttribs[attrib]));
      GL_CHECK(glVertexAttribPointer(theAttribs[attrib], 3, GL_FLOAT, GL_FALSE, 0, theData[attrib].getData()));
      }
    }
  glDrawArrays(GL_TRIANGLES, 0, theData[ATTRIB_VERTICES].getCount());
  }
