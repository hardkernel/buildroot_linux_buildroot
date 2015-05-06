/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_GEOMETRY_COMPLEX_HPP
#define M_SHADOWMAPDR_GEOMETRY_COMPLEX_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MGeometryBase.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a complex geometry. A user can merge various primitives into
 * one object, which is represented by this class.
 * In other words a user can combine multiple draw calls into one by using this class.
 */
class MGeometryComplex : public MGeometryBase
  {
public:

  // ----- Types -----

  ///
  typedef MGeometryBase BaseClass;

  // ----- Constructors and destructors -----

  ///
  MGeometryComplex();

  ///
  virtual ~MGeometryComplex();

  // ----- Operators -----

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  /// The method replaces current primitive by aPrimitive including modes
  void set(const MRendererPrimitive& aPrimitive)
    { getPrimitive() = aPrimitive; }

  /// The method merges primitive to the one mesh, which is represented by this object
  void append(const MRendererPrimitive& aPrimitive)
    { getPrimitive().append(aPrimitive); }

private:

  // ----- Fields -----

  };


#endif
