/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAP_ARRAY_HPP
#define M_SHADOWMAP_ARRAY_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents an array template type. The most important thing about arrays in general is that they tend to keep elements in contiguous memory.
 * That being said they are cache friendly.
 */
template <typename Type, typename TypeOffset = unsigned int>
class MArray
  {
public:

  // ----- Types -----

  ///
  enum
    {
    ///
    ARRAY_MEM_ALIGN = 32,
    ///
    NOT_FOUND = 0xFFFFFFFF
    };

  ///
  typedef Type ElementType;

  ///
  typedef TypeOffset OffsetType;

  ///
  typedef Type* MDRIterator;

  ///
  typedef const Type* MDRConstIterator;

  // ----- Constructors and destructors -----

  ///
  MArray(TypeOffset aSize = 16);

  ///
  ~MArray();

  // ----- Operators -----

  ///
  MArray& operator=(const MArray& aOther);

  ///
  bool operator==(const MArray& aOther) const;

  ///
  Type& operator[] (TypeOffset aIndex)
    { return theData[aIndex]; }

  ///
  const Type& operator[] (TypeOffset aIndex) const
    { return theData[aIndex]; }

  ///
  MDRConstIterator getBegin() const
    { return &theData[0]; }

  ///
  MDRConstIterator getEnd() const
    { return getBegin() + theCount; }

  ///
  MDRIterator getBegin()
    { return &theData[0]; }

  ///
  MDRIterator getEnd()
    { return getBegin() + theCount; }

  // ----- Accessors and mutators -----

  ///
  TypeOffset getCount() const
    { return theCount; }

  ///
  const Type* getData() const
    { return theData; }

  ///
  void setCount(TypeOffset aCount);

  // ----- Miscellaneous -----

  ///
  void set(const MArray& aOther);

  ///
  void append(const Type& aElement);

  ///
  void append(const MArray& aOther);

  ///
  void clear();

  ///
  void fill(Type aValue);

  ///
  void set(const Type* aData,
           TypeOffset aCount);

  ///
  TypeOffset find(Type aElement,
                  TypeOffset aStartOffset = 0) const;

private:

  // ----- Types -----

  // ----- Fields -----

  //
  Type *theData;
  // The real buffer size
  TypeOffset theSize;
  // The active elements count in the array
  TypeOffset theCount;

  // ----- Forbidden Copy Constructor -----

  //
  MArray(const MArray& aOther);

  // ----- Miscellaneous -----

  //
  void reallocateIfNeeded(TypeOffset aExpectedCountOfElements);

  };


template <typename Type, typename TypeOffset>
MArray<Type, TypeOffset>::MArray(
    TypeOffset aSize)
    :
    theData(NULL),
    theSize(aSize),
    theCount(0)
  {
  reallocateIfNeeded(theSize);
  }

template <typename Type, typename TypeOffset>
MArray<Type, TypeOffset>::~MArray()
  {
  if (theData != NULL)
    {
    free(theData);
    theData = NULL;
    }
  }

template <typename Type, typename TypeOffset>
MArray<Type, TypeOffset>& MArray<Type, TypeOffset>::operator=(
    const MArray& aOther)
  {
  set(aOther);
  return *this;
  }

template <typename Type, typename TypeOffset>
bool MArray<Type, TypeOffset>::operator==(
    const MArray& aOther) const
  {
  // Compare length
  if (theCount != aOther.getCount())
    {
    return false;
    }
  // Compare each element
  for (TypeOffset index = 0; index < theCount; index++)
    {
    if (theData[index] != aOther.theData[index])
      {
      return false;
      }
    }
  // It seems that both arrays are the same
  return true;
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::set(
    const MArray& aOther)
  {
  setCount(aOther.getCount());
  if (aOther.getCount() > 0)
    {
    memcpy(theData, aOther.theData, sizeof(Type) * aOther.getCount());
    }
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::append(
    const Type& aElement)
  {
  setCount(theCount + 1);
  theData[theCount - 1] = aElement;
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::append(
    const MArray& aOther)
  {
  TypeOffset prevCount = theCount;
  setCount(theCount + aOther.getCount());
  memcpy(theData + prevCount, aOther.theData, aOther.getCount() * sizeof(Type));
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::clear()
  {
  setCount(0);
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::fill(Type aValue)
  {
  memset(theData, aValue, theSize * sizeof(Type));
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::set(
    const Type* aData,
    TypeOffset aCount)
  {
  setCount(aCount);
  memcpy(theData, aData, sizeof(Type) * aCount);
  }

template <typename Type, typename TypeOffset>
TypeOffset MArray<Type, TypeOffset>::find(
    Type aElement,
    TypeOffset aStartOffset/* = 0*/) const
  {
  // Prevent going out of bound of array
  if (aStartOffset >= theCount)
    {
    return NOT_FOUND;
    }
  // Compare each element
  for (TypeOffset offset = aStartOffset; offset < theCount; offset++)
    {
    if (theData[offset] == aElement)
      {
      return offset;
    }
  }
  return NOT_FOUND;
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::setCount(
    TypeOffset aCount)
  {
  reallocateIfNeeded(aCount);
  theCount = aCount;
  }

template <typename Type, typename TypeOffset>
void MArray<Type, TypeOffset>::reallocateIfNeeded(
    TypeOffset aExpectedCountOfElements)
  {
  // Align memory for faster allocation
  TypeOffset newSize = ((aExpectedCountOfElements / ARRAY_MEM_ALIGN) + 1) * ARRAY_MEM_ALIGN;
  // Allocate the memory unconditionally
  theData = (Type*)realloc(theData, newSize * sizeof(Type));
  theSize = newSize;
  }

#endif
