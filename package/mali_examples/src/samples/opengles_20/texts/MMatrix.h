/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_MATRIX_HPP
#define M_TEXTSDR_MATRIX_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MVector3.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

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

  ///
  const Type* getData() const
    { return (const Type*)theData; }

  // ----- Miscellaneous -----

  ///
  void setIdentity();

  ///
  void setRotation(Type aAngle,
                   Type aX,
                   Type aY,
                   Type aZ);

  ///
  void setScale(Type aX,
                Type aY,
                Type aZ);

  ///
  void setScale(const MVector3<Type>& aScale)
    { setScale(aScale[0], aScale[1], aScale[2]); }

  ///
  void applyTranslation(Type aX,
                        Type aY,
                        Type aZ);

  ///
  void applyTranslation(const MVector3<Type>& aPosition)
    { applyTranslation(aPosition[0], aPosition[1], aPosition[2]); }

  ///
  void multiply(const MMatrix& aLeft,
                const MMatrix& aRight,
                MMatrix& aOut) const;

  /// aOther * this
  void multiplyLeft(const MMatrix<Type>& aOther)
    { multiply(aOther, *this, *this); }

  /// this * aOther
  void multiplyRight(const MMatrix<Type>& aOther)
    { multiply(*this, aOther, *this); }

  ///
  void setPerspective(Type aFieldOfViewAngle,
                      Type aAspect,
                      Type aNear,
                      Type aFar);

  ///
  void invert();

  ///
  void transpose();

private:

  // ----- Fields -----

  //
  Type theData[16];

  };

#include "MMatrixImpl.h"

typedef MMatrix<float> MMatrix4f;

#endif
