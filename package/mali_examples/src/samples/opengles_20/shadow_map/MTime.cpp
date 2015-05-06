/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MTime.h"

MTime::MTime(
    double aSeconds /*= 0.0*/)
    :
    theInitSeconds(aSeconds),
    theSeconds(theInitSeconds),
    //
    theTimer()
  {
  theTimer.reset();
  }

MTime& MTime::operator = (
    const MTime& aOther)
  {
  theSeconds = aOther.theSeconds;
  return *this;
  }

MTime MTime::operator - (
    const MTime& aRight) const
  {
  MTime t(*this);
  t.theSeconds -= aRight.theSeconds;
  return t;
  }

MTime MTime::operator + (
    const MTime& aRight) const
  {
  MTime t(*this);
  t.theSeconds += aRight.theSeconds;
  return t;
  }

void MTime::update()
  {
  theSeconds = theInitSeconds + theTimer.getTime();
  }
