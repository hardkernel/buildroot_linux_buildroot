/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_GEOMETRY_BASE_HPP
#define M_FURDR_GEOMETRY_BASE_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MArray.h"
#include "MRendererPrimitive.h"
#include "MVector3.h"

class MRendererProgram;

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class is a base for other geometries such as: sphere, cone, cube and so on.
 * The RAW geometry is held by thePrimitive member type of MRendererPrimitive.
 */
class MGeometryBase
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MGeometryBase();

  ///
  virtual ~MGeometryBase();

  // ----- Operators -----

  // ----- Accessors and mutators -----

  /// The method returns the read-only primitive
  const MRendererPrimitive& getPrimitive() const
    { return thePrimitive; }

  /// The method returns the read-write primitive. A user can modify the shape of primitive.
  MRendererPrimitive& getPrimitive()
    { return thePrimitive; }

  // ----- Miscellaneous -----

  /// The method renders the primitive with a program, which is passed as a parameter
  virtual void render(const MRendererProgram& aProgram);

private:

  // ----- Fields -----

  //
  MRendererPrimitive thePrimitive;

  };


#endif
