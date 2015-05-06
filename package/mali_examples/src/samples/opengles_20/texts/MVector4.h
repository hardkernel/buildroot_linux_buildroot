/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTS_VECTOR4_HPP
#define M_TEXTS_VECTOR4_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

template <typename Type>
class MVector4
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MVector4();

  ///
  MVector4(const Type& a1,
           const Type& a2,
           const Type& a3,
           const Type& a4)
    {
    set(a1, a2, a3, a4);
    }

  ///
  ~MVector4();

  // ----- Operators -----

  ///
  MVector4<Type>& operator-(const MVector4<Type>& aRight)
    {
    translate(-aRight[0], -aRight[1], -aRight[2], -aRight[3]);
    return *this;
    }

  ///
  MVector4<Type>& operator+=(const MVector4<Type>& aRight)
    {
    translate(aRight);
    return *this;
    }

  ///
  MVector4<Type>& operator-=(const MVector4<Type>& aRight)
    {
    translate(-aRight[0], -aRight[1], -aRight[2], -aRight[3]);
    return *this;
    }

  ///
  Type& operator[] (unsigned int aIndex)
    { return theV[aIndex]; }

  ///
  const Type& operator[] (unsigned int aIndex) const
    { return theV[aIndex]; }

  // ----- Accessors and mutators -----

  ///
  const Type* getData() const
    { return theV; }

  // ----- Miscellaneous -----

  ///
  void set(const Type& a1,
           const Type& a2,
           const Type& a3,
           const Type& a4);

  ///
  void translate(const MVector4<Type>& aRight)
    { translate(aRight[0], aRight[1], aRight[2], aRight[3]); }

  ///
  void translate(const Type& a1,
                 const Type& a2,
                 const Type& a3,
                 const Type& a4);

private:

  // ----- Fields -----

  //
  Type theV[4];

  };


template <typename Type>
MVector4<Type>::MVector4()
  {
  memset(theV, 0, sizeof(Type) * 4);
  }

template <typename Type>
MVector4<Type>::~MVector4()
  {
  }

template <typename Type>
void MVector4<Type>::set(
    const Type& a1,
    const Type& a2,
    const Type& a3,
    const Type& a4)
  {
  theV[0] = a1;
  theV[1] = a2;
  theV[2] = a3;
  theV[3] = a4;
  }

template <typename Type>
void MVector4<Type>::translate(
    const Type& a1,
    const Type& a2,
    const Type& a3,
    const Type& a4)
  {
  theV[0] += a1;
  theV[1] += a2;
  theV[2] += a3;
  theV[3] += a4;
  }

typedef MVector4<float> MVector4f;
typedef MVector4<double> MVector4d;
typedef MVector4<int> MVector4i;
typedef MVector4<unsigned int> MVector4ui;

#endif
