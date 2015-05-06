/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_GEOMETRY_SPHERE_HPP
#define M_FURDR_GEOMETRY_SPHERE_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MGeometryBase.h"

#include "MArray.h"
#include "MVector3.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class is inherited from a MGeometryBase class and represents a sphere geometry.
 */
class MGeometrySphere : public MGeometryBase
  {
public:

  // ----- Types -----

  ///
  typedef MGeometryBase BaseClass;

  ///
  typedef MArray<MVector3f> MArrayVec3f;

  // ----- Constructors and destructors -----

  ///
  MGeometrySphere();

  ///
  virtual ~MGeometrySphere();

  // ----- Operators -----

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  /// The method builds a sphere geometry with a specified radius.
  /// aHorizontal and aVertical parameters are used to specify resolution of the geometry.
  void set(float aRadius,
           unsigned int aHorizontal,
           unsigned int aVertical);

private:

  // ----- Fields -----

  // ----- Miscellaneous -----

  // Triangle Strip based on Indices
  void buildPrimitveTSI(unsigned int aHorizontal,
                        unsigned int aVertical,
                        const MArrayVec3f& aPoints,
                        const MArrayVec3f& aCoords,
                        MRendererPrimitive& aOutPrimitive) const;

  // Triangle Strip
  void buildPrimitveTS(unsigned int aHorizontal,
                       unsigned int aVertical,
                       const MArrayVec3f& aPoints,
                       MRendererPrimitive& aOutPrimitive) const;
  };


#endif
