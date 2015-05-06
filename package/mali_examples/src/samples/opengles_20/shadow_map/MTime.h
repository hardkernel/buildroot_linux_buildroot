/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_TIME_HPP
#define M_SHADOWMAPDR_TIME_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"
#include "Timer.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

class MTime
  {
public:

  // ----- Types -----

  // ----- Constructors and destructors -----

  ///
  MTime(double aSeconds = 0.0);

  ///
  ~MTime()
    { }

  // ----- Operators -----

  ///
  MTime& operator = (const MTime& aOther);

  ///
  MTime operator - (const MTime& aRight) const;

  ///
  MTime operator + (const MTime& aRight) const;

  ///
  bool operator >= (const MTime& aRight) const
    { return (theSeconds >= aRight.theSeconds); }

  ///
  bool operator <= (const MTime& aRight) const
    { return (theSeconds <= aRight.theSeconds); }

  // ----- Accessors and mutators -----

  ///
  double getSeconds() const
    { return theSeconds; }

  // ----- Miscellaneous -----

  /* Reset the time to 0.0 seconds */
  void reset();

  /* Read current time */
  void update();

private:

  // ----- Fields -----

  //
  const double theInitSeconds;
  double theSeconds;
  //
  MaliSDK::Timer theTimer;

  };

#endif
