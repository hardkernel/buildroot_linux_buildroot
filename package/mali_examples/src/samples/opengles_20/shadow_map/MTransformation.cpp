/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MTransformation.h"

MTransformation::MTransformation()
    :
    theUpdateIsNeeded(false),
    theCameraPosition(0.0f, 0.0f, 0.0f),
    theCameraRotation(0.0f, 0.0f, 0.0f, 0.0f),
    theObjectPosition(0.0f, 0.0f, 0.0f),
    theObjectRotation(0.0f, 0.0f, 0.0f, 1.0f),
    theObjectScale(1.0f, 1.0f, 1.0f),
    theObjectEulerRotation(0.0f, 0.0f, 0.0f),
    thePreObjectRotation(0.0f, 0.0f, 0.0f, 1.0f),
    thePreObjectScale(1.0f, 1.0f, 1.0f),
    thePreObjectEulerRotation(0.0f, 0.0f, 0.0f)
  {
  }

MTransformation::~MTransformation()
  {
  }

const MMatrix4f& MTransformation::getMatrix(
    MMatrixType aMatrixType) const
  {
  if (theUpdateIsNeeded &&
      aMatrixType != TYPE_MATRIX)
    update();
  return theMatrices[aMatrixType];
  }

void MTransformation::setProjection(
    float aFieldOfViewAngle,
    float aAspect,
    float aNear,
    float aFar)
  {
  theMatrices[TYPE_MATRIX_PROJECTION].setPerspective(aFieldOfViewAngle, aAspect, aNear, aFar);
  theUpdateIsNeeded = true;
  }

void MTransformation::setCameraPosition(
    const MVector3f& aCameraPosition)
  {
  theCameraPosition = aCameraPosition;
  theUpdateIsNeeded = true;
  }

void MTransformation::setCameraRotation(
    const MVector4f& aCameraRotation)
  {
  theCameraRotation = aCameraRotation;
  theUpdateIsNeeded = true;
  }

void MTransformation::setObjectPosition(
    const MVector3f& aObjectPosition)
  {
  theObjectPosition = aObjectPosition;
  theUpdateIsNeeded = true;
  }

void MTransformation::setObjectScale(
    const MVector3f& aObjectScale)
  {
  theObjectScale = aObjectScale;
  theUpdateIsNeeded = true;
  }

void MTransformation::setPreObjectScale(
    const MVector3f& aPreObjectScale)
  {
  thePreObjectScale = aPreObjectScale;
  theUpdateIsNeeded = true;
  }

void MTransformation::setObjectRotation(
    const MVector4f& aObjectRotation)
  {
  theObjectRotation = aObjectRotation;
  theUpdateIsNeeded = true;
  }

void MTransformation::setPreObjectRotation(
    const MVector4f& aPreObjectRotation)
  {
  thePreObjectRotation = aPreObjectRotation;
  theUpdateIsNeeded = true;
  }

  void MTransformation::setObjectEulerRotation(
    const MVector3f& aObjectEulerRotation)
  {
  theObjectEulerRotation = aObjectEulerRotation;
  theUpdateIsNeeded = true;
  }

void MTransformation::setPreObjectEulerRotation(
    const MVector3f& aPreObjectEulerRotation)
  {
  thePreObjectEulerRotation = aPreObjectEulerRotation;
  theUpdateIsNeeded = true;
  }

void MTransformation::update() const
  {
  MMatrix4f tmpMatPos;
  MMatrix4f tmpMatRot;
  MMatrix4f tmpMatRotPre;
  MMatrix4f tmpMatSca;
  MMatrix4f tmpMatScaPre;
  MMatrix4f tmpMatRotEuler;
  MMatrix4f tmpMatRotEulerPre;
  MMatrix4f tmpMat;

  // Evaluate matrix of object position
  tmpMatRotEuler.setIdentity();
  tmpMat.setIdentity();
  tmpMat.setRotation(theObjectEulerRotation[0], 1.0f, 0.0f, 0.0f);
  tmpMatRotEuler.multiplyRight(tmpMat);
  tmpMat.setRotation(theObjectEulerRotation[1], 0.0f, 1.0f, 0.0f);
  tmpMatRotEuler.multiplyRight(tmpMat);
  tmpMat.setRotation(theObjectEulerRotation[2], 0.0f, 0.0f, 1.0f);
  tmpMatRotEuler.multiplyRight(tmpMat);

    // Evaluate matrix of object position
  tmpMatRotEulerPre.setIdentity();
  tmpMat.setIdentity();
  tmpMat.setRotation(thePreObjectEulerRotation[0], 1.0f, 0.0f, 0.0f);
  tmpMatRotEulerPre.multiplyRight(tmpMat);
  tmpMat.setRotation(thePreObjectEulerRotation[1], 0.0f, 1.0f, 0.0f);
  tmpMatRotEulerPre.multiplyRight(tmpMat);
  tmpMat.setRotation(thePreObjectEulerRotation[2], 0.0f, 0.0f, 1.0f);
  tmpMatRotEulerPre.multiplyRight(tmpMat);

  tmpMatRot.setIdentity();
  tmpMatRot.setRotation(theObjectRotation[0],
                        theObjectRotation[1],
                        theObjectRotation[2],
                        theObjectRotation[3]);
  // Include Euler rotation
  tmpMatRot.multiplyRight(tmpMatRotEuler);

  tmpMatRotPre.setIdentity();
  tmpMatRotPre.setRotation(thePreObjectRotation[0],
                           thePreObjectRotation[1],
                           thePreObjectRotation[2],
                           thePreObjectRotation[3]);
  tmpMatRotPre.multiplyRight(tmpMatRotEulerPre);

  tmpMatPos.setIdentity();
  tmpMatPos.applyTranslation(theObjectPosition);
  tmpMatSca.setIdentity();
  tmpMatSca.setScale(theObjectScale);
  tmpMatScaPre.setIdentity();
  tmpMatScaPre.setScale(thePreObjectScale);
  theMatrices[TYPE_MATRIX].setIdentity();
  theMatrices[TYPE_MATRIX].multiplyRight(tmpMatRotPre);
  theMatrices[TYPE_MATRIX].multiplyRight(tmpMatScaPre);
  theMatrices[TYPE_MATRIX].multiplyRight(tmpMatPos);
  theMatrices[TYPE_MATRIX].multiplyRight(tmpMatRot);
  theMatrices[TYPE_MATRIX].multiplyRight(tmpMatSca);
  //
  theMatrices[TYPE_MATRIX_INV] = theMatrices[TYPE_MATRIX];
  theMatrices[TYPE_MATRIX_INV].invert();
  theMatrices[TYPE_MATRIX_INV_TRANS] = theMatrices[TYPE_MATRIX_INV];
  theMatrices[TYPE_MATRIX_INV_TRANS].transpose();


  // Evaluate view (camera) matrix
  theMatrices[TYPE_MATRIX_VIEW].setIdentity();
  theMatrices[TYPE_MATRIX_VIEW].applyTranslation(theCameraPosition);
  tmpMatRot.setIdentity();
  tmpMatRot.setRotation(theCameraRotation[0],
                        theCameraRotation[1],
                        theCameraRotation[2],
                        theCameraRotation[3]);
  theMatrices[TYPE_MATRIX_VIEW].multiplyLeft(tmpMatRot);
  theMatrices[TYPE_MATRIX_VIEW].invert();


  // Now it is possible to evaluate MVP matrix
  theMatrices[TYPE_MATRIX_MVP] = theMatrices[TYPE_MATRIX];
  theMatrices[TYPE_MATRIX_MVP].multiplyLeft(theMatrices[TYPE_MATRIX_VIEW]);
  theMatrices[TYPE_MATRIX_MVP].multiplyLeft(theMatrices[TYPE_MATRIX_PROJECTION]);
  //
  theUpdateIsNeeded = false;
  }
