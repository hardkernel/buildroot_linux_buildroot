/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_GEOMETRY_TORUS_HPP
#define M_SHADOWMAPDR_GEOMETRY_TORUS_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MGeometryBase.h"

#include "MArray.h"
#include "MVector3.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class is inherited from a MGeometryBase class and represents a torus geometry.
 */
class MGeometryTorus : public MGeometryBase
  {
public:

  // ----- Types -----

  ///
  typedef MGeometryBase BaseClass;

  ///
  typedef MArray<MVector3f> MArrayVec3f;

  // ----- Constructors and destructors -----

  ///
  MGeometryTorus();

  ///
  virtual ~MGeometryTorus();

  // ----- Operators -----

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  /// The method builds a torus geometry with a specified orbit radius and ring radius.
  /// aHorizontal and aVertical parameters are used to specify resolution of the geometry.
  void set(float aRadiusOrbit,
           float aRadiusRing,
           unsigned int aHorizontal,
           unsigned int aVertical);

private:

  // ----- Fields -----

  //
  MArrayVec3f thePoints;
  MArrayVec3f theNormals;

  // ----- Miscellaneous -----

  // Triangle Strip based on indices
  void buildPrimitveTSI(unsigned int aHorizontal,
                        unsigned int aVertical,
                        const MArrayVec3f& aPoints,
                        const MArrayVec3f& aNormals,
                        MRendererPrimitive& aOutPrimitive) const;

  // Triangle Strip
  void buildPrimitveTS(unsigned int aHorizontal,
                       unsigned int aVertical,
                       const MArrayVec3f& aPoints,
                       const MArrayVec3f& aNormals,
                       MRendererPrimitive& aOutPrimitive) const;
  };


#endif
