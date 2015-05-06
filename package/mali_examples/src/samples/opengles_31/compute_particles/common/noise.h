/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef NOISE_H
#define NOISE_H

float clamp(float x, float min, float max);

// Returns a random integer with a period of 2^128 - 1
unsigned int xor128();

// Returns a single precision floating-point value uniformly over the interval [0.0, 1.0]
float frand();

#endif
