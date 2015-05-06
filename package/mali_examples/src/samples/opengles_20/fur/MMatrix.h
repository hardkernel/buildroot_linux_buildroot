/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_MATRIX_HPP
#define M_FURDR_MATRIX_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MVector3.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The template class is a helper for matrix operations.
 */
template <typename Type>
class MMatrix
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MMatrix();

  ///
  ~MMatrix();

  // ----- Accessors and mutators -----

  /// The method returns the matrix data in an OpenGL layout.
  const Type* getData() const
    { return (const Type*)theData; }

  // ----- Miscellaneous -----

  ///
  void setIdentity();

  /// The method sets a rotation for the current matrix.
  /// It will cancel all other operations previously done on the matrix.
  void setRotation(Type aAngle,
                   Type aX,
                   Type aY,
                   Type aZ);

  /// The method sets a scale for the current matrix.
  /// It will cancel all other operations previously done on the matrix.
  void setScale(Type aX,
                Type aY,
                Type aZ);

  /// The method sets a scale for the current matrix.
  /// It will cancel all other operations previously done on the matrix.
  void setScale(const MVector3<Type>& aScale)
    { setScale(aScale[0], aScale[1], aScale[2]); }

  /// The method applies a transformation on top of the already existing operations in the current matrix.
  void applyTranslation(Type aX,
                        Type aY,
                        Type aZ);

  /// The method applies a transformation on top of the already existing operations in the current matrix.
  void applyTranslation(const MVector3<Type>& aPosition)
    { applyTranslation(aPosition[0], aPosition[1], aPosition[2]); }

  /// The method multiplies two matrices and returns a result matrix as in a parameter.
  void multiply(const MMatrix& aLeft,
                const MMatrix& aRight,
                MMatrix& aOut) const;

  /// aOther * this
  void multiplyLeft(const MMatrix<Type>& aOther)
    { multiply(aOther, *this, *this); }

  /// this * aOther
  void multiplyRight(const MMatrix<Type>& aOther)
    { multiply(*this, aOther, *this); }

  /// The method makes the current matrix as a projection matrix with given parameters.
  void setPerspective(Type aFieldOfViewAngle,
                      Type aAspect,
                      Type aNear,
                      Type aFar);

  /// The method inverts the current matrix.
  void invert();

  /// The method transposes the current matrix - rows become columns and vice versa.
  void transpose();

private:

  // ----- Fields -----

  //
  Type theData[16];

  };

#include "MMatrixImpl.h"

typedef MMatrix<float> MMatrix4f;

#endif
