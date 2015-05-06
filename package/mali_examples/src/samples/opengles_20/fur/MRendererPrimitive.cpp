/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MRendererPrimitive.h"

using namespace MaliSDK;

MRendererPrimitive::MRendererPrimitive()
    :
    theMode(MODE_TRIANGLES),
    theDrawingMode(DRAWING_ARRAYS),
    theBufferHandle(0),
    //
    theData()
  {
  memset(theAttribs, -1, sizeof(theAttribs));
  }

MRendererPrimitive::~MRendererPrimitive()
  {
  }

void MRendererPrimitive::append(
    MDRAttrib aAttrib,
    const MVector3f& aValue)
  {
  theData[aAttrib].append(aValue);
  }

void MRendererPrimitive::appendBox(
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

bool MRendererPrimitive::append(
    const MRendererPrimitive& aOther)
  {
  // To merge both primitives they have to be in the same mode
  if (theMode != aOther.theMode ||
      theDrawingMode != aOther.theDrawingMode)
    {
    LOGI("Warning: Cannot merge primitive with different structure (modes)!");
    return false;
    }

  unsigned int thisCountOfVertices = theData[ATTRIB_VERTICES].getCount();
  unsigned int otherCount = aOther.theData[ATTRIB_VERTICES].getCount();
  // Merge Vertices, Normals, Colors etc.
  for (unsigned int attrib = 0; attrib < ATTRIBS_COUNT; attrib++)
    theData[attrib].append(aOther.theData[attrib]);

  // Merge Indices
  if (theIndices.getCount() > 0)
    {
    // Add two dummy last indices to not drawing primitive
    theIndices.append(theIndices[theIndices.getCount() - 1]);
    theIndices.append(theIndices[theIndices.getCount() - 1]);
    }
  if (aOther.theIndices.getCount() > 0)
    {
    // Add two dummy last indices to not drawing primitive
    theIndices.append(aOther.theIndices[0] + thisCountOfVertices);
    theIndices.append(aOther.theIndices[0] + thisCountOfVertices);
    }
  unsigned int countOfIndices = aOther.theIndices.getCount();
  for (unsigned int index = 0; index < countOfIndices; index++)
    theIndices.append(aOther.theIndices[index] + thisCountOfVertices);

  return true;
  }

void MRendererPrimitive::clearAll()
  {
  for (unsigned int attrib = 0; attrib < ATTRIBS_COUNT; attrib++)
    theData[attrib].clear();
  theIndices.clear();
  }

bool MRendererPrimitive::setAttribIndex(
    MDRAttrib aAttrib,
    int aIndex)
  {
  if (aAttrib >= ATTRIBS_COUNT)
    return false;
  theAttribs[aAttrib] = aIndex;
  return true;
  }

void MRendererPrimitive::render()
  {
  for (unsigned int attrib = 0; attrib < ATTRIBS_COUNT; attrib++)
    {
    if (theAttribs[attrib] >= 0)
      {
      GL_CHECK(glEnableVertexAttribArray(theAttribs[attrib]));
      GL_CHECK(glVertexAttribPointer(theAttribs[attrib], 3, GL_FLOAT, GL_FALSE, 0, theData[attrib].getData()));
      }
    }
  //
  if (theDrawingMode == DRAWING_ELEMENTS &&
      theIndices.getCount())
    {
    GL_CHECK(glDrawElements(theMode,
                            theIndices.getCount(),
                            GL_UNSIGNED_SHORT,
                            theIndices.getData()));
    }
  else
    {
    GL_CHECK(glDrawArrays(theMode, 0, theData[ATTRIB_VERTICES].getCount()));
    }
  //
  for (unsigned int attrib = 0; attrib < ATTRIBS_COUNT; attrib++)
    {
    if (theAttribs[attrib] >= 0)
      {
      GL_CHECK(glDisableVertexAttribArray(theAttribs[attrib]));
      }
    }
  }

void MRendererPrimitive::transform(
    const MVector3f& aPosition)
  {
  // Transform vertices
  MArrayf::MDRIterator it = theData[ATTRIB_VERTICES].getBegin();
  MArrayf::MDRIterator end = theData[ATTRIB_VERTICES].getEnd();
  while (it != end)
    {
    MVector3f& vertex = *it;
    vertex += aPosition;
    it++;
    }
  }
