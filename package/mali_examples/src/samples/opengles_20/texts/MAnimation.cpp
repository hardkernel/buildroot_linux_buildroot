/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MAnimation.h"

template <>
void MAnimation<float, float, float>::lerp(
    double aCoefficient,
    float aLeft,
    float aRight,
    float& aReturnValue) const
  {
  aReturnValue = (float)(((1.0 - aCoefficient) * (double)aLeft) + (aCoefficient * (double)aRight));
  }

template <>
void MAnimation<int, int, int>::lerp(
    double aCoefficient,
    int aLeft,
    int aRight,
    int& aReturnValue) const
  {
  aReturnValue = (int)(ceil((1.0 - aCoefficient) * (double)aLeft) + (aCoefficient * (double)aRight));
  }
