/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MGeometryRectangle.h"

MGeometryRectangle::MGeometryRectangle()
    :
    BaseClass()
  {
  }

MGeometryRectangle::~MGeometryRectangle()
  {
  }


void MGeometryRectangle::set(
    float aWidth,
    float aHeight,
    unsigned int aHorizontal,
    unsigned int aVertical)
  {
  MRendererPrimitive& primitive = getPrimitive();
  //
  primitive.clearAll();
  primitive.setMode(MRendererPrimitive::MODE_TRIANGLE_STRIP);
  primitive.setDrawingMode(MRendererPrimitive::DRAWING_ELEMENTS);
  //
  MVector3f point(0.0f, 0.0f, 0.0f);
  MVector3f normal(0.0f, 0.0f, 1.0f);
  MVector3f coord(0.0f, 0.0f, 0.0f);
  float hW = aWidth / 2.0f;
  float hH = aHeight / 2.0f;
  unsigned int index = 0;
  for (unsigned int ix = 0; ix <= aHorizontal; ix++)
    {
    point[0] = hW - (float)ix * (aWidth / (float)aHorizontal);
    coord[0] = (float)ix / (float)aHorizontal;
    for (unsigned int iy = 0; iy <= aVertical; iy++)
      {
      point[1] = hH - (float)iy * (aHeight / (float)aVertical);
      coord[1] = (float)iy / (float)aVertical;
      //
      primitive.append(MRendererPrimitive::ATTRIB_VERTICES, point);
      primitive.append(MRendererPrimitive::ATTRIB_NORMALS, normal);
      primitive.append(MRendererPrimitive::ATTRIB_COLORS, point + 0.5f);
      primitive.append(MRendererPrimitive::ATTRIB_TEXCOORDS_0, coord);

      primitive.appendIndex(index++);
      }
    }
  }
