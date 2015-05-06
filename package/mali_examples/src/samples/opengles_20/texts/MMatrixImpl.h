/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_TEXTSDR_MATRIX_IMPL_HPP
#define M_TEXTSDR_MATRIX_IMPL_HPP

#include "Mathematics.h"

template <typename Type>
MMatrix<Type>::MMatrix()
  {
  setIdentity();
  }

template <typename Type>
MMatrix<Type>::~MMatrix()
  {
  }

template <typename Type>
void MMatrix<Type>::setIdentity()
  {
  memset(theData, 0, 16 * sizeof(Type));
  theData[0]  = 1;
  theData[5]  = 1;
  theData[10] = 1;
  theData[15] = 1;
  }

template <typename Type>
void MMatrix<Type>::setRotation(
    Type aAngle,
    Type aX,
    Type aY,
    Type aZ)
  {
  double radians, c, s, c1, u[3], length;
  int i, j;

  radians = (aAngle * M_PI) / 180.0;

  c = cos(radians);
  s = sin(radians);

  c1 = 1.0 - cos(radians);

  length = sqrt(aX * aX + aY * aY + aZ * aZ);

  u[0] = aX / length;
  u[1] = aY / length;
  u[2] = aZ / length;

  for (i = 0; i < 16; i++)
    {
    theData[i] = 0.0;
    }

  theData[15] = 1.0;

  for (i = 0; i < 3; i++)
    {
    theData[i * 4 + (i + 1) % 3] = (Type) (u[(i + 2) % 3] * s);
    theData[i * 4 + (i + 2) % 3] = (Type) (-u[(i + 1) % 3] * s);
    }

  for (i = 0; i < 3; i++)
    {
    for (j = 0; j < 3; j++)
      {
      theData[i * 4 + j] += (Type) (c1 * u[i] * u[j] + (i == j ? c : 0.0));
      }
    }
  }

template <typename Type>
void MMatrix<Type>::setScale(
    Type aX,
    Type aY,
    Type aZ)
  {
  setIdentity();
  //
  theData[ 0] = aX;
  theData[ 5] = aY;
  theData[10] = aZ;
  }

template <typename Type>
void MMatrix<Type>::applyTranslation(
    Type aX,
    Type aY,
    Type aZ)
  {
  theData[12] += aX;
  theData[13] += aY;
  theData[14] += aZ;
  }

template <typename Type>
void MMatrix<Type>::multiply(
    const MMatrix<Type>& aLeft,
    const MMatrix<Type>& aRight,
    MMatrix<Type>& aOut) const
  {
  const Type *A = (const Type*)aLeft.theData;
  const Type *B = (const Type*)aRight.theData;
  Type *C = (Type*)aOut.theData;
  //
  int i, j, k;
  Type aTmp[16];
  for (i = 0; i < 4; i++)
    {
    for (j = 0; j < 4; j++)
      {
      aTmp[j * 4 + i] = 0.0;
      for (k = 0; k < 4; k++)
        {
        aTmp[j * 4 + i] += A[k * 4 + i] * B[j * 4 + k];
        }
      }
    }
  //
  for (i = 0; i < 16; i++)
    {
    C[i] = aTmp[i];
    }
  }

template <typename Type>
void MMatrix<Type>::setPerspective(
    Type aFieldOfViewAngle,
    Type aAspectRatio,
    Type aNear,
    Type aFar)
  {
  setIdentity();
  //
  Type* P = (Type*)theData;
  //
  double f = 1.0/tan(aFieldOfViewAngle * 0.5);
  //
  P[0] = (Type) (f / aAspectRatio);
  P[5] = (Type) (f);
  P[10] = (Type) ((aNear + aFar) / (aNear - aFar));
  P[11] = -1.0f;
  P[14] = (Type) ((2.0 * aNear * aFar) / (aNear - aFar));
  P[15] = 0.0f;
  }

template <typename Type>
void MMatrix<Type>::invert()
  {
  MaliSDK::Matrix* mat = (MaliSDK::Matrix*)theData;
  MaliSDK::Matrix matInv =  MaliSDK::Matrix::matrixInvert(mat);
  memcpy(theData, &matInv, sizeof(MaliSDK::Matrix));
  }

template <typename Type>
void MMatrix<Type>::transpose()
  {
  float fTemp;

  fTemp = theData[1];
  theData[1] = theData[4];
  theData[4] = fTemp;

  fTemp = theData[2];
  theData[2] = theData[8];
  theData[8] = fTemp;

  fTemp = theData[3];
  theData[3] = theData[12];
  theData[12] = fTemp;

  fTemp = theData[6];
  theData[6] = theData[9];
  theData[9] = fTemp;

  fTemp = theData[7];
  theData[7] = theData[13];
  theData[13] = fTemp;

  fTemp = theData[11];
  theData[11] = theData[14];
  theData[14] = fTemp;
  }

#endif
