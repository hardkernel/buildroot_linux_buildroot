/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTS_STRING_HPP
#define M_TEXTS_STRING_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MArray.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

template <typename Type, typename TypeOffset = unsigned int>
class MStringBase
  {
public:

  // ----- Types -----

  ///
  typedef Type CharType;

  ///
  typedef TypeOffset CharOffset;

  // ----- Statics and constants -----

  ///
  static const CharOffset theNotFound;

  // ----- Constructors and destructors -----

  ///
  MStringBase()
    {}

  ///
  MStringBase(const MStringBase<Type, TypeOffset>& aOther);

  ///
  MStringBase(const char* aAscii);

  ///
  MStringBase(const Type* aCharacters,
              TypeOffset aCountCharacters);

  ///
  ~MStringBase()
    {}

  // ----- Operators -----

  ///
  MStringBase& operator=(const MStringBase& aOther);

  ///
  MStringBase& operator+=(const MStringBase& aOther);

  ///
  bool operator==(const MStringBase& aOther) const
    { return (theInternalString == aOther.theInternalString); }

  ///
  Type& operator[] (TypeOffset aIndex)
    { return theInternalString[aIndex]; }

  ///
  const Type& operator[] (TypeOffset aIndex) const
    { return theInternalString[aIndex]; }

  // ----- Accessors and mutators -----

  // ----- Miscellaneous -----

  ///
  TypeOffset getLength() const;

  ///
  const Type* getData() const
    { return theInternalString.getData(); }

  ///
  void clear()
    { theInternalString.clear(); }

  ///
  void append(Type aChar);

  ///
  void append(const MStringBase<Type, TypeOffset>& aChar);

  ///
  void set(const char* aAscii);

  ///
  TypeOffset find(CharType aChar,
                  TypeOffset aStartOffset = 0) const;

  ///
  void sub(TypeOffset aStartOffset,
           TypeOffset aEndOffset,
           MStringBase<Type, TypeOffset>& aOutStr) const;

  ///
  int getAsInt() const;

private:

  // ----- Fields -----

  //
  MArray<Type> theInternalString;

  };

template<typename Type, typename TypeOffset>
const TypeOffset MStringBase<Type, TypeOffset>::theNotFound = MArray<Type>::NOT_FOUND;

template <typename Type, typename TypeOffset>
MStringBase<Type, TypeOffset>::MStringBase(
    const char* aAscii)
  {
  set(aAscii);
  }

template <typename Type, typename TypeOffset>
MStringBase<Type, TypeOffset>::MStringBase(
    const MStringBase<Type, TypeOffset>& aOther)
    :
    theInternalString()
  {
  theInternalString = aOther.theInternalString;
  }

template <typename Type, typename TypeOffset>
MStringBase<Type, TypeOffset>& MStringBase<Type, TypeOffset>::operator=(
    const MStringBase<Type, TypeOffset>& aOther)
  {
  theInternalString = aOther.theInternalString;
  return *this;
  }

template <typename Type, typename TypeOffset>
MStringBase<Type, TypeOffset>& MStringBase<Type, TypeOffset>::operator+=(
    const MStringBase<Type, TypeOffset>& aOther)
  {
  append(aOther);
  return *this;
  }

template <typename Type, typename TypeOffset>
TypeOffset MStringBase<Type, TypeOffset>::getLength() const
  {
  TypeOffset len = theInternalString.getCount();
  if (len)
    len--; // decrease because of ZERO at the end of string
  return len;
  }

template <typename Type, typename TypeOffset>
void MStringBase<Type, TypeOffset>::set(
    const char* aAscii)
  {
  if (sizeof(char) == sizeof(Type))
    {
    theInternalString.set(aAscii, (TypeOffset)strlen(aAscii));
    theInternalString.append(0);
    }
  else
    {
    TypeOffset len = (TypeOffset)strlen(aAscii);
    theInternalString.setCount(len + 1);
    for (TypeOffset index = 0; index < len; index++)
      theInternalString[index] = (Type)aAscii[index];
    theInternalString[len] = 0;
    }
  }

template <typename Type, typename TypeOffset>
TypeOffset MStringBase<Type, TypeOffset>::find(
    CharType aChar,
    TypeOffset aStartOffset) const
  {
  return theInternalString.find(aChar, aStartOffset);
  }

template <typename Type, typename TypeOffset>
void MStringBase<Type, TypeOffset>::sub(
    TypeOffset aStartOffset,
    TypeOffset aEndOffset,
    MStringBase<Type, TypeOffset>& aOutStr) const
  {
  aOutStr.clear();
  // Prevent not going to out of memory
  if (aStartOffset >= getLength() ||
      aEndOffset >= getLength())
    return;
  // Copy characters
  for (TypeOffset offset = aStartOffset; offset <= aEndOffset; offset++)
    aOutStr.append((*this)[offset]);
  }

template <typename Type, typename TypeOffset>
void MStringBase<Type, TypeOffset>::append(
    Type aChar)
  {
  TypeOffset count = theInternalString.getCount();
  if (count == 0)
    // Add first character to the array
    theInternalString.append(aChar);
  else
    // Replace last element which is a value of ZERO
    theInternalString[theInternalString.getCount() - 1] = aChar;
  // Append ZERO to the end of string
  theInternalString.append(0);
  }

template <typename Type, typename TypeOffset>
void MStringBase<Type, TypeOffset>::append(
    const MStringBase<Type, TypeOffset>& aOther)
  {
  TypeOffset count = theInternalString.getCount();
  if (count != 0)
    // Remove last ZERO element from the array
    theInternalString.setCount(count - 1);
  // Simply add other string because it already has its own ZERO at the end
  theInternalString.append(aOther.theInternalString);
  }

template <typename Type, typename TypeOffset>
int MStringBase<Type, TypeOffset>::getAsInt() const
  {
  if (getLength())
    return atoi(theInternalString.getData());
  else
    return 0;
  }

typedef MStringBase<char, unsigned int> MString;
typedef MStringBase<char, unsigned int> MPath;

#endif
