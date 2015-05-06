/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "noise.h"

float clamp(float x, float min, float max)
{
    return x < min ? min : x > max ? max : x;
}

float max(float x, float y)
{
    return x > y ? x : y;
}

float min(float x, float y)
{
    return x < y ? x : y;
}

// See http://en.wikipedia.org/wiki/Xorshift
unsigned int xor128()
{
    static unsigned int x = 123456789;
    static unsigned int y = 362436069;
    static unsigned int z = 521288629;
    static unsigned int w = 88675123;
    unsigned int t;

    t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = w ^ (w >> 19) ^ (t ^ (t >>8));
}

float frand()
{
    return xor128() / float(4294967295.0f);
}
