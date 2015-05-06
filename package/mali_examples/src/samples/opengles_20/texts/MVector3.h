/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTS_VECTOR3_HPP
#define M_TEXTS_VECTOR3_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

template <typename Type, typename PassType=Type>
class MVector3
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MVector3();

  ///
  MVector3(const PassType& a1,
           const PassType& a2,
           const PassType& a3)
    {
    set(a1, a2, a3);
    }

  ///
  ~MVector3();

  // ----- Operators -----

  ///
  MVector3 operator+(PassType aRightValue) const
    {
    MVector3 tmp(*this);
    tmp.translate(aRightValue, aRightValue, aRightValue);
    return tmp;
    }

  ///
  MVector3 operator*(PassType aRightValue) const
    {
    MVector3 tmp(*this);
    tmp[0] *= aRightValue;
    tmp[1] *= aRightValue;
    tmp[2] *= aRightValue;
    return tmp;
    }

  ///
  MVector3 operator+(const MVector3& aRight) const
    {
    MVector3 tmp(*this);
    tmp.translate(aRight[0], aRight[1], aRight[2]);
    return tmp;
    }

  ///
  MVector3 operator-(const MVector3& aRight) const
    {
    MVector3 tmp(*this);
    tmp.translate(-aRight[0], -aRight[1], -aRight[2]);
    return tmp;
    }

  ///
  MVector3 operator/(const MVector3& aRight) const
    {
    MVector3 tmp(theV[0] / aRight[0],
                   theV[1] / aRight[1],
                   theV[2] / aRight[2]);
    return tmp;
    }

  ///
  MVector3& operator-(const MVector3& aRight)
    {
    MVector3 tmp(*this);
    tmp.translate(-aRight[0], -aRight[1], -aRight[2]);
    return tmp;
    }

  ///
  MVector3& operator+=(PassType aValue)
    {
    translate(aValue, aValue, aValue);
    return *this;
    }

  ///
  MVector3& operator+=(const MVector3& aRight)
    {
    translate(aRight);
    return *this;
    }

  ///
  MVector3& operator-=(const MVector3& aRight)
    {
    translate(-aRight[0], -aRight[1], -aRight[2]);
    return *this;
    }

  ///
  MVector3& operator/=(const MVector3& aRight)
    {
    theV[0] /= aRight[0];
    theV[1] /= aRight[1];
    theV[2] /= aRight[2];
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
  void set(PassType a1,
           PassType a2,
           PassType a3);

  ///
  void translate(const MVector3& aRight)
    { translate(aRight[0], aRight[1], aRight[2]); }

  ///
  void translate(PassType a1,
                 PassType a2,
                 PassType a3);

  ///
  Type length() const;

  ///
  void normalize();

  /// Calculate dot product. Notice if both vector are normalized then you will get cosinus angle between these vectors
  Type dot(const MVector3& aOther) const;

  /// Cross product aLeft x aRight
  MVector3 cross(const MVector3& aLeft,
                 const MVector3& aRight) const;

private:

  // ----- Fields -----

  //
  Type theV[3];

  };


template <typename Type, typename PassType>
MVector3<Type, PassType>::MVector3()
  {
  memset(theV, 0, sizeof(Type) * 3);
  }

template <typename Type, typename PassType>
MVector3<Type, PassType>::~MVector3()
  {
  }

template <typename Type, typename PassType>
void MVector3<Type, PassType>::set(
    PassType a1,
    PassType a2,
    PassType a3)
  {
  theV[0] = a1;
  theV[1] = a2;
  theV[2] = a3;
  }

template <typename Type, typename PassType>
void MVector3<Type, PassType>::translate(
    PassType a1,
    PassType a2,
    PassType a3)
  {
  theV[0] += a1;
  theV[1] += a2;
  theV[2] += a3;
  }

template <typename Type, typename PassType>
Type MVector3<Type, PassType>::length() const
  {
  return (Type)sqrt(theV[0] * theV[0] +
                    theV[1] * theV[1] +
                    theV[2] * theV[2]);
  }

template <typename Type, typename PassType>
void MVector3<Type, PassType>::normalize()
  {
  Type len = length();
  theV[0] /= len;
  theV[1] /= len;
  theV[2] /= len;
  }

template <typename Type, typename PassType>
Type MVector3<Type, PassType>::dot(
    const MVector3& aOther) const
  {
  return (Type)(theV[0] * aOther.theV[0] +
                theV[1] * aOther.theV[1] +
                theV[2] * aOther.theV[2]);
  }

template <typename Type, typename PassType>
MVector3<Type, PassType> MVector3<Type, PassType>::cross(
    const MVector3& aLeft,
    const MVector3& aRight) const
  {
  MVector3 f;
  f[0] = aLeft[1] * aRight[2] - aLeft[2] * aRight[1];
  f[1] = aLeft[2] * aRight[0] - aLeft[0] * aRight[2];
  f[2] = aLeft[0] * aRight[1] - aLeft[1] * aRight[0];
  return f;
  }

typedef MVector3<float> MVector3f;
typedef MVector3<double> MVector3d;
typedef MVector3<int> MVector3i;
typedef MVector3<unsigned int> MVector3ui;

#endif
