/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAP_ANIMATION_HPP
#define M_SHADOWMAP_ANIMATION_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MArray.h"
#include "MTime.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a very simple linear animation.
 * By using this class you should be able to animate any type of value with few line of codes only.
 */
template <typename Type,
          typename PassType = Type,
          typename ReturnType = PassType>
class MAnimation
  {
public:

  // ----- Types -----

  ///
  enum MDRPlayMode
    {
    /// Default
    PLAY_MODE_ONCE,
    ///
    PLAY_MODE_REPEAT
    };

  // ----- Constructors and destructors -----

  ///
  MAnimation()
      :
      theValue((Type)0),
      theMode(PLAY_MODE_ONCE)
    { }

  ///
  ~MAnimation()
    { }

  // ----- Operators -----

  // ----- Accessors and mutators -----

  ///
  ReturnType getValue() const
    { return theValue; }

  // ----- Miscellaneous -----

  /* The method recalculate value to aTime */
  void update(const MTime& aTime);

  /* The method creates a key with a value at a given time */
  void setKey(PassType aValue,
              const MTime& aTime);

  ///
  void setMode(MDRPlayMode aMode)
    { theMode = aMode; }

  ///
  void removeAllKeys()
    { theKeys.clear(); }

private:

  // ----- Types -----

  /**
   * The class is designed for internal use only.
   * A single instance of this class contains two members which are: a time and a value assigned to the given time.
   * MAnimation object allocates an instance of the class once the setKey method is invoked.
   * By having multiple instances of this class the MAnimation is able to interpolate value between keys
   * and return a linearly estimated value at a time.
   */
  class MDRKey
    {
  public:

    // ----- Constructors and destructors -----

    ///
    MDRKey(PassType aValue,
           const MTime& aTime)
        :
        theValue(aValue),
        theTime(aTime)
      {}

    // ----- Operators -----

    ///
    MDRKey& operator = (const MDRKey& aOther)
      {
      theValue = aOther.theValue;
      theTime = aOther.theTime;
      return *this;
      }

    // ----- Friends -----

    ///
    friend class MAnimation<Type, PassType, ReturnType>;

  private:

    // ----- Fields -----

    ///
    PassType theValue;
    MTime theTime;
    };

  ///
  typedef MArray<MDRKey> MDRKeys;

  // ----- Fields -----

  //
  Type theValue;
  MDRPlayMode theMode;
  MDRKeys theKeys;

  // ------ Miscellaneous -----

  //
  void lerp(double aCoefficient,
            PassType aLeft,
            PassType aRight,
            Type& aReturnValue) const;

  };

template <typename Type, typename PassType, typename ReturnType>
void MAnimation<Type, PassType, ReturnType>::setKey(
    PassType aValue,
    const MTime& aTime)
  {
  // TO DO: implement this on the list. Otherwise you have to create key in a sequence from lower to grater time value
  theKeys.append(MDRKey(aValue, aTime));
  }

template <typename Type, typename PassType, typename ReturnType>
void MAnimation<Type, PassType, ReturnType>::update(
    const MTime& aTime)
  {
  unsigned int count = theKeys.getCount();

  if (count <= 1)
    {
    if (count == 1)
      theValue = theKeys[0].theValue;
    return;
    }

  MTime curTime = aTime;
  if (theMode == PLAY_MODE_REPEAT)
    {
    MTime range;
    range = (theKeys[count - 1].theTime - theKeys[0].theTime);

    double currSecs = curTime.getSeconds();
    currSecs = currSecs - range.getSeconds() * floor((currSecs - theKeys[0].theTime.getSeconds()) / (range.getSeconds()));
    curTime = MTime(currSecs);
    }

  // Find left and right keys in neighbourhood of aTime parameter
  MDRKey* left = NULL;
  MDRKey* right = NULL;
  for (unsigned int index = 1; index < count; index++)
    {
    if (curTime >= theKeys[index - 1].theTime &&
        curTime <= theKeys[index].theTime)
      {
      left = &theKeys[index - 1];
      right = &theKeys[index];
      break;
      }
    }

  // If neighbourhood was not found then do not calculate animation
  if (left == NULL ||
      right == NULL)
    return;

  double coefficient = (curTime.getSeconds() - left->theTime.getSeconds()) / (right->theTime.getSeconds() - left->theTime.getSeconds());
  lerp(coefficient, left->theValue, right->theValue, theValue);
  }

typedef MAnimation<float> MAnimation1f;
typedef MAnimation<int> MAnimation1i;

#endif
