/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MGeometryTorus.h"

MGeometryTorus::MGeometryTorus()
    :
    BaseClass(),
    //
    thePoints(),
    theNormals()
  {
  }

MGeometryTorus::~MGeometryTorus()
  {
  }

void MGeometryTorus::set(
    float aRadiusOrbit,
    float aRadiusRing,
    unsigned int aHorizontal,
    unsigned int aVertical)
  {
  thePoints.clear();
  theNormals.clear();

  // Evaluate vertices points for the whole sphere
    {
    float rad = 0.0f;
    MVector3f tmp(0.0f, 0.0f, 0.0f);
    MVector3f ringOrigin(0.0f, 0.0f, 0.0f);
    //
    for (unsigned int iy = 0; iy < aVertical; iy++)
      {
      float alpha = (float)iy * (360.f / (float)aVertical);
      rad = (float)(alpha * 0.01745329251994329576923690768489);
      //
      float fx = 0.0f;
      float fy = aRadiusRing * cosf(rad);
      float fz = aRadiusOrbit + aRadiusRing * sinf(rad);
      //
      float froX = 0.0f;
      float froY = 0.0f;
      float froZ = 0.0f;
      //
      for (unsigned int ix = 0; ix < aHorizontal; ix++)
        {
        float beta = (float)ix * (360.f / (float)aHorizontal);
        float rad = (float)(beta * 0.01745329251994329576923690768489);
        //
        tmp[0] = fz * sinf(rad);
        tmp[1] = fy;
        tmp[2] = fz * cosf(rad);
        //
        ringOrigin[0] = aRadiusOrbit * sinf(rad);
        ringOrigin[1] = 0.0f;
        ringOrigin[2] = aRadiusOrbit * cosf(rad);
        //
        thePoints.append(tmp);
        //
        tmp -= ringOrigin; // Calculate normal
        theNormals.append(tmp);
        }
      }
    }

#define MDR_DRAWING_ELEMENTS
#ifdef MDR_DRAWING_ELEMENTS
  buildPrimitveTSI(aHorizontal,
                   aVertical,
                   thePoints,
                   theNormals,
                   getPrimitive());
#else
  buildPrimitveTS(aHorizontal,
                  aVertical,
                  thePoints,
                  theNormals,
                  getPrimitive());
#endif
  }

void MGeometryTorus::buildPrimitveTSI(
    unsigned int aHorizontal,
    unsigned int aVertical,
    const MArrayVec3f& aPoints,
    const MArrayVec3f& aNormals,
    MRendererPrimitive& aOutPrimitive) const
  {
  // Configure primitive
  aOutPrimitive.clearAll();
  aOutPrimitive.setMode(MRendererPrimitive::MODE_TRIANGLE_STRIP);
  aOutPrimitive.setDrawingMode(MRendererPrimitive::DRAWING_ELEMENTS);

  // Copy points and normals one-to-one
  unsigned int count = aPoints.getCount();
  for (unsigned int index = 0; index < count; index++)
    {
    const MVector3f& point = aPoints[index];
    //
    aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, point);
    aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, aNormals[index]);
    aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(point[0] + 0.5f, point[1] + 0.5f, point[2] + 0.5f));
    aOutPrimitive.append(MRendererPrimitive::ATTRIB_TEXCOORDS_0, point);
    }

  // Build indices
    {
    for (unsigned int iy = 1; iy < aVertical; iy++)
      {
      for (unsigned int ix = 1; ix < aHorizontal; ix++)
        {
        /*

                vPrev
           [ix - 1, iy - 1]
                  o
                    \
                      \
                        \
                          \
                            o
                         [ix, iy]
                           vCurr

        */
        unsigned int iPrev = (iy - 1) * aHorizontal + (ix - 1);
        unsigned int iCurr = iy * aHorizontal + ix;
        //
        aOutPrimitive.appendIndex(iPrev);
        aOutPrimitive.appendIndex(iCurr);
        }
      /*

              vLast                vFirstPrev
      [aHorizontal - 1, iy - 1]   [0, iy - 1]
                o                   o
                    \               |   \
                        \           |       \
                            \       |           \
                                \   |               \
                                    o                   o
                                 [0, iy]              [1, iy]
                                vFirstCurr           vSecondCurr

      */
      // Close the slice
      unsigned int iLast       = (iy - 1) * aHorizontal + (aHorizontal - 1);
      unsigned int iFirstCurr  = iy * aHorizontal + 0;
      unsigned int iFirstPrev  = (iy - 1) * aHorizontal + 0;
      unsigned int iSecondCurr = iy * aHorizontal + 1;
      //
      aOutPrimitive.appendIndex(iLast);
      aOutPrimitive.appendIndex(iFirstCurr);
      aOutPrimitive.appendIndex(iFirstPrev);
      aOutPrimitive.appendIndex(iSecondCurr);

      // Now the cursor moves to the first position of a next slice without
      // drawing a triangle and without necesity of switching between drawing modes nor by calling
      // glDrawArrays or glDrawElements for an each slice

      // Draw invisible line from vFirstPrev to vSecondCurr
      aOutPrimitive.appendIndex(iSecondCurr);
      // Draw invisible line from vSecondCurr to vFirstCurr
      aOutPrimitive.appendIndex(iFirstCurr);
      // Draw point in vFirstCurr
      aOutPrimitive.appendIndex(iFirstCurr);
      // Draw the point once again to correct face culling
      aOutPrimitive.appendIndex(iFirstCurr);

      if (iy == (aVertical - 1))
        {
        // This is last slice and should be connected with the first one
        for (unsigned int ix = 1; ix < aHorizontal; ix++)
          {
          /*

                  vPrev
             [ix - 1, iy]
                    o
                      \
                        \
                          \
                            \
                              o
                           [ix, 0]
                             vCurr

          */
          unsigned int iPrev = iy * aHorizontal + (ix - 1);
          unsigned int iCurr = 0 * aHorizontal + ix;
          //
          aOutPrimitive.appendIndex(iPrev);
          aOutPrimitive.appendIndex(iCurr);
          }
        /*

                vLast                vFirstPrev
        [aHorizontal - 1, iy]       [0, iy]
                  o                   o
                      \               |   \
                          \           |       \
                              \       |           \
                                  \   |               \
                                      o                   o
                                   [0, 0]               [1, 0]
                                  vFirstCurr           vSecondCurr

        */
        // Close the slice
        unsigned int iLast       = iy * aHorizontal + (aHorizontal - 1);
        unsigned int iFirstCurr  = 0 * aHorizontal + 0;
        unsigned int iFirstPrev  = iy * aHorizontal + 0;
        unsigned int iSecondCurr = 0 * aHorizontal + 1;
        //
        aOutPrimitive.appendIndex(iLast);
        aOutPrimitive.appendIndex(iFirstCurr);
        aOutPrimitive.appendIndex(iFirstPrev);
        aOutPrimitive.appendIndex(iSecondCurr);
        }
      // That's all, lets try!!!
      }
    }
  }


void MGeometryTorus::buildPrimitveTS(
    unsigned int aHorizontal,
    unsigned int aVertical,
    const MArrayVec3f& aPoints,
    const MArrayVec3f& aNormals,
    MRendererPrimitive& aOutPrimitive) const
  {
  // Configure primitive
  aOutPrimitive.clearAll();
  aOutPrimitive.setMode(MRendererPrimitive::MODE_TRIANGLE_STRIP);
  aOutPrimitive.setDrawingMode(MRendererPrimitive::DRAWING_ARRAYS);

  // Build primitive based on previously created points with STRIP mode
    {
    aOutPrimitive.setMode(MRendererPrimitive::MODE_TRIANGLE_STRIP);
    aOutPrimitive.setDrawingMode(MRendererPrimitive::DRAWING_ELEMENTS);
    for (unsigned int iy = 1; iy < aVertical; iy++)
      {
      for (unsigned int ix = 1; ix < aHorizontal; ix++)
        {
        /*

                vPrev
           [ix - 1, iy - 1]
                  o
                    \
                      \
                        \
                          \
                            o
                         [ix, iy]
                           vCurr

        */
        unsigned int iPrev = (iy - 1) * aHorizontal + (ix - 1);
        unsigned int iCurr = iy * aHorizontal + ix;
        //
        const MVector3f& vPrev = thePoints[iPrev];
        const MVector3f& vCurr = thePoints[iCurr];
        //
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vPrev);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vCurr);
        //
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iPrev]);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iCurr]);
        //
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vPrev[0] + 0.5f, vPrev[1] + 0.5f, vPrev[2] + 0.5f));
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vCurr[0] + 0.5f, vCurr[1] + 0.5f, vCurr[2] + 0.5f));
        }
      /*

              vLast                vFirstPrev
      [aHorizontal - 1, iy - 1]   [0, iy - 1]
                o                   o
                    \               |   \
                        \           |       \
                            \       |           \
                                \   |               \
                                    o                   o
                                 [0, iy]              [1, iy]
                                vFirstCurr           vSecondCurr

      */
      // Close the slice
      unsigned int iLast       = (iy - 1) * aHorizontal + (aHorizontal - 1);
      unsigned int iFirstCurr  = iy * aHorizontal + 0;
      unsigned int iFirstPrev  = (iy - 1) * aHorizontal + 0;
      unsigned int iSecondCurr = iy * aHorizontal + 1;
      //
      const MVector3f& vLast       = thePoints[iLast];
      const MVector3f& vFirstCurr  = thePoints[iFirstCurr];
      const MVector3f& vFirstPrev  = thePoints[iFirstPrev];
      const MVector3f& vSecondCurr = thePoints[iSecondCurr];
      //
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vLast);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vFirstCurr);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vFirstPrev);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vSecondCurr);
      //
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iLast]);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iFirstCurr]);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iFirstPrev]);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iSecondCurr]);
      //
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vLast[0] + 0.5f, vLast[1] + 0.5f, vLast[2] + 0.5f));
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vFirstCurr[0] + 0.5f, vFirstCurr[1] + 0.5f, vFirstCurr[2] + 0.5f));
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vFirstPrev[0] + 0.5f, vFirstPrev[1] + 0.5f, vFirstPrev[2] + 0.5f));
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vSecondCurr[0] + 0.5f, vSecondCurr[1] + 0.5f, vSecondCurr[2] + 0.5f));

      // Check if next slice is available and set vertex position in appropriate place to
      // start drawing triangles for next slice (row)

      // In this case the drawing cursor should be move to the first position of a next slice without
      // drawing triangles and without necesity of switching between drawing modes nor by calling
      // glDrawArrays or glDrawElements for an each slice

      // Draw invisible line from vFirstPrev to vSecondCurr
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vSecondCurr);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iSecondCurr]);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(1.0f, 1.0f, 1.0f));
      // Draw invisible line from vSecondCurr to vFirstCurr
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vFirstCurr);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iFirstCurr]);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(1.0f, 1.0f, 1.0f));
      // Draw point in vFirstCurr
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vFirstCurr);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iFirstCurr]);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(1.0f, 1.0f, 1.0f));
      // Draw the point once again to correct face culling
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vFirstCurr);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iFirstCurr]);
      aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(1.0f, 1.0f, 1.0f));

      if (iy == (aVertical - 1))
        {
        // This is last slice and should be connected with the first one
        for (unsigned int ix = 1; ix < aHorizontal; ix++)
          {
          /*

                  vPrev
             [ix - 1, iy]
                    o
                      \
                        \
                          \
                            \
                              o
                           [ix, 0]
                             vCurr

          */
          unsigned int iPrev = iy * aHorizontal + (ix - 1);
          unsigned int iCurr = 0 * aHorizontal + ix;
          //
          //
          const MVector3f& vPrev = thePoints[iPrev];
          const MVector3f& vCurr = thePoints[iCurr];
          //
          aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vPrev);
          aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vCurr);
          //
          aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iPrev]);
          aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iCurr]);
          //
          aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vPrev[0] + 0.5f, vPrev[1] + 0.5f, vPrev[2] + 0.5f));
          aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vCurr[0] + 0.5f, vCurr[1] + 0.5f, vCurr[2] + 0.5f));
          }
        /*

                vLast                vFirstPrev
        [aHorizontal - 1, iy]       [0, iy]
                  o                   o
                      \               |   \
                          \           |       \
                              \       |           \
                                  \   |               \
                                      o                   o
                                   [0, 0]               [1, 0]
                                  vFirstCurr           vSecondCurr

        */
        // Close the slice
        unsigned int iLast       = iy * aHorizontal + (aHorizontal - 1);
        unsigned int iFirstCurr  = 0 * aHorizontal + 0;
        unsigned int iFirstPrev  = iy * aHorizontal + 0;
        unsigned int iSecondCurr = 0 * aHorizontal + 1;
        //
        const MVector3f& vLast       = thePoints[iLast];
        const MVector3f& vFirstCurr  = thePoints[iFirstCurr];
        const MVector3f& vFirstPrev  = thePoints[iFirstPrev];
        const MVector3f& vSecondCurr = thePoints[iSecondCurr];
        //
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vLast);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vFirstCurr);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vFirstPrev);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_VERTICES, vSecondCurr);
        //
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iLast]);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iFirstCurr]);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iFirstPrev]);
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_NORMALS, theNormals[iSecondCurr]);
        //
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vLast[0] + 0.5f, vLast[1] + 0.5f, vLast[2] + 0.5f));
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vFirstCurr[0] + 0.5f, vFirstCurr[1] + 0.5f, vFirstCurr[2] + 0.5f));
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vFirstPrev[0] + 0.5f, vFirstPrev[1] + 0.5f, vFirstPrev[2] + 0.5f));
        aOutPrimitive.append(MRendererPrimitive::ATTRIB_COLORS, MVector3f(vSecondCurr[0] + 0.5f, vSecondCurr[1] + 0.5f, vSecondCurr[2] + 0.5f));
        }
      // That's all, lets try!!!
      }
    }
  }
