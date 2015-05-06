/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAP_BOX_HPP
#define M_SHADOWMAP_BOX_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MVector3.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The template represents a box, which can be used for representing a 3D cube bounding box.
 */
template <typename Type>
class MBox
  {
public:

  // ----- Types -----

  ///
  typedef MVector3<Type> MVector3Type;

  // ----- Constructors and destructors -----

  ///
  MBox();

  ///
  MBox(const MVector3Type& aMin,
       const MVector3Type& aMax)
    {
    set(aMin, aMax);
    }

  ///
  ~MBox();

  // ----- Accessors and mutators -----

  /// The method returns the front-top-left corner for read-write.
  MVector3Type& getMin()
    { return theMin; }

  /// The method returns the back-bottom-right corner for read-write.
  MVector3Type& getMax()
    { return theMax; }

  /// The method returns the front-top-left corner for read-only.
  const MVector3Type& getMin() const
    { return theMin; }

  /// The method returns the back-bottom-right corner for read-only.
  const MVector3Type& getMax() const
    { return theMax; }

  // ----- Miscellaneous -----

  /// The method returns a width of the box
  Type getWidth() const
    { return (theMax[0] - theMin[0]); }

  /// The method sets front-top-left and back-bottom-right corners of the box.
  void set(const MVector3Type& aMin,
           const MVector3Type& aMax);

  /// The method translates the box with a given vector (aVector)
  void translate(const MVector3Type& aVector);

private:

  // ----- Fields -----

  //
  MVector3Type theMin;
  MVector3Type theMax;

  };


template <typename Type>
MBox<Type>::MBox()
    :
    theMin(),
    theMax()
  {
  }

template <typename Type>
MBox<Type>::~MBox()
  {
  }

template <typename Type>
void MBox<Type>::set(
    const MVector3Type& aMin,
    const MVector3Type& aMax)
  {
  theMin = aMin;
  theMax = aMax;
  }

template <typename Type>
void MBox<Type>::translate(
    const MVector3Type& aVector)
  {
  theMin += aVector;
  theMax += aVector;
  }

typedef MBox<float> MBoxf;

#endif
