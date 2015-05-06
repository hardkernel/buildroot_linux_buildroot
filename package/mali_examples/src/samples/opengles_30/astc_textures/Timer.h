/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef TIMER_H
#define TIMER_H

#include <cstdio>

#if defined(_WIN32)
#else
#include <sys/time.h>
#endif

namespace MaliSDK
{
    /**
     * \brief Provides a platform independent high resolution timer.
     * \note The timer measures real time, not CPU time.
     */
    class Timer
    {
    private:
        int frameCount;
        float fps;
        float lastTime;
    #if defined(_WIN32)
        double resetStamp;
        double invFreq;
        float lastInterval;
        float lastFpsUpdate;
    #else
        timeval startTime;
        timeval currentTime;
        float lastIntervalTime;
        float fpsTime;
    #endif
    public:
        /**
         * \brief Default Constructor
         */
        Timer();

        /**
         * \brief Resets the timer to 0.0f.
         */
        void reset();

        /**
         * \brief Returns the time passed since object creation or since reset() was last called.
         * \return Float containing the current time.
         */
        float getTime();

        /**
         * \brief Tests if 'seconds' seconds have passed since reset() or this method was called.
         *
         * \param[in] seconds number of seconds passed default is 1.0
         * \return bool true if a 'seconds' seconds are passed and false otherwise.
         */
        bool isTimePassed(float seconds = 1.0f);
    };
}
#endif /* TIMER_H */