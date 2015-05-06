/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAP_VECTOR2_HPP
#define M_SHADOWMAP_VECTOR2_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

template <typename Type>
class MVector2
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MVector2();

  ///
  MVector2(const Type& a1,
           const Type& a2)
    {
    set(a1, a2);
    }

  ///
  ~MVector2();

  // ----- Operators -----

  ///
  MVector2<Type>& operator+=(const MVector2<Type>& aRight)
    {
    translate(aRight);
    return *this;
    }

  ///
  MVector2<Type>& operator-=(const MVector2<Type>& aRight)
    {
    translate(-aRight.theV[0], -aRight.theV[1]);
    return *this;
    }

  ///
  MVector2<Type>& operator*=(const MVector2<Type>& aRight)
    {
    theV[0] *= aRight.theV[0];
    theV[1] *= aRight.theV[1];
    return *this;
    }

  ///
  MVector2<Type>& operator*=(Type aValue)
    {
    theV[0] *= aValue;
    theV[1] *= aValue;
    return *this;
    }

  ///
  MVector2 operator-(const MVector2& aRight) const
    {
    MVector2 tmp(*this);
    tmp.theV[0] -= aRight.theV[0];
    tmp.theV[1] -= aRight.theV[1];
    return tmp;
    }

  ///
  MVector2 operator*(Type aValue) const
    {
    MVector2 tmp(*this);
    tmp.theV[0] *= aValue;
    tmp.theV[1] *= aValue;
    return tmp;
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
    { return &theV; }

  // ----- Miscellaneous -----

  ///
  void set(Type a1,
           Type a2);

  ///
  void translate(const MVector2<Type>& aRight);

  ///
  void translate(Type a1,
                 Type a2);

  ///
  Type length() const;

private:

  // ----- Fields -----

  //
  Type theV[2];

  };


template <typename Type>
MVector2<Type>::MVector2()
  {
  memset(theV, 0, sizeof(Type) * 2);
  }

template <typename Type>
MVector2<Type>::~MVector2()
  {
  }

template <typename Type>
void MVector2<Type>::set(
    Type a1,
    Type a2)
  {
  theV[0] = a1;
  theV[1] = a2;
  }

template <typename Type>
void MVector2<Type>::translate(
    const MVector2<Type>& aRight)
  {
  theV[0] += aRight[0];
  theV[1] += aRight[1];
  }

template <typename Type>
void MVector2<Type>::translate(
    Type a1,
    Type a2)
  {
  theV[0] += a1;
  theV[1] += a2;
  }

template <typename Type>
Type MVector2<Type>::length() const
  {
  return (Type)sqrt((double)(theV[0] * theV[0] +
                             theV[1] * theV[1]));
  }

typedef MVector2<float> MVector2f;
typedef MVector2<double> MVector2d;
typedef MVector2<int> MVector2i;
typedef MVector2<unsigned int> MVector2ui;

#endif
