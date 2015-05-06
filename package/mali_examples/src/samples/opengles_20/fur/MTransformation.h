/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_TRANSFORMATION_HPP
#define M_FURDR_TRANSFORMATION_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MMatrix.h"
#include "MVector3.h"
#include "MVector4.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a complex transformation for Model, View and Projection.
 * Once one of the transformation is changed all matrices are updated if necessary.
 * Very handy class for obtaining various different type of matrices from a single transformation.
 */
class MTransformation
  {
public:

  // ----- Types -----

  ///
  enum MMatrixType
    {
    ///
    TYPE_MATRIX,
    ///
    TYPE_MATRIX_INV,
    ///
    TYPE_MATRIX_INV_TRANS,
    ///
    TYPE_MATRIX_VIEW, // Camera matrix
    ///
    TYPE_MATRIX_PROJECTION,
    ///
    TYPE_MATRIX_MVP,
    ///
    TYPE_MATRICES_COUNT
    };

  // ----- Constructors and destructors -----

  ///
  MTransformation();

  ///
  ~MTransformation();

  // ----- Operators -----

  // ----- Accessors and mutators -----

  /// The method returns a specified matrix from the current transformation
  const MMatrix4f& getMatrix(MMatrixType aMatrixType) const;

  // ----- Miscellaneous -----

  /// The method sets projection parameters for the transformation
  void setProjection(float aFieldOfViewAngle,
                     float aAspect,
                     float aNear,
                     float aFar);

  /// The method sets a camera position for the current transformation
  void setCameraPosition(const MVector3f& aCameraPosition);

  /// The method sets a camera rotation for the current transformation
  void setCameraRotation(const MVector4f& aCameraRotation);

  /// The method sets an object position for the current transformation
  void setObjectPosition(const MVector3f& aObjectPosition);

  /// The method set an object scale for the current transformation
  void setObjectScale(const MVector3f& aObjectScale);

  /// The method set an object rotation for the current transformation
  void setObjectRotation(const MVector4f& aObjectRotation);

  /// The method set an object rotation by using Euler angles for the current transformation
  void setObjectEulerRotation(const MVector3f& aObjectRotation);

  /// The method sets a pre multiply rotation in order to do a pivot transformation
  void setPreObjectRotation(const MVector4f& aPreObjectRotation);

  /// The method sets a pre multiply scale in order to do a pivot transformation
  void setPreObjectScale(const MVector3f& aPreObjectScale);

  /// The method sets a pre multiply rotation with Euler angles in order to do a pivot transformation
  void setPreObjectEulerRotation(const MVector3f& aPreObjectRotation);

private:

  // ----- Fields -----

  //
  mutable bool theUpdateIsNeeded;
  //
  MVector3f theCameraPosition;
  MVector4f theCameraRotation;
  MVector3f theObjectPosition;
  MVector4f theObjectRotation;
  MVector3f theObjectScale;
  MVector3f theObjectEulerRotation;
  MVector4f thePreObjectRotation;
  MVector3f thePreObjectScale;
  MVector3f thePreObjectEulerRotation;
  //
  mutable MMatrix4f theMatrices[TYPE_MATRICES_COUNT];

  // ----- Miscellaneous -----

  //
  void update() const;

  };


#endif
