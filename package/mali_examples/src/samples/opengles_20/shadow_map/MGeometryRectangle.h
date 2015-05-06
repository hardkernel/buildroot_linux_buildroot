/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_GEOMETRY_RECTANGLE_HPP
#define M_SHADOWMAPDR_GEOMETRY_RECTANGLE_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MGeometryBase.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class is inherited from a MGeometryBase class and represents a rectangle geometry.
 */
class MGeometryRectangle : public MGeometryBase
  {
public:

  // ----- Types -----

  ///
  typedef MGeometryBase BaseClass;

  // ----- Constructors and destructors -----

  ///
  MGeometryRectangle();

  ///
  virtual ~MGeometryRectangle();

  // ----- Operators -----

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  /// The method builds a rectangle geometry with a specified width and height.
  /// aHorizontal and aVertical parameters are used to specify resolution of the geometry.
  void set(float aWidth,
           float aHeight,
           unsigned int aHorizontal,
           unsigned int aVertical);

private:

  // ----- Fields -----

  };


#endif
