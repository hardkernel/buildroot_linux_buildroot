/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef SORT_H
#define SORT_H

#include "common/common.h"
const uint32_t BLOCK_SIZE = 32;
const uint32_t NUM_KEYS = 1 << 14;
const uint32_t NUM_BLOCKS = NUM_KEYS / BLOCK_SIZE;

bool sort_init();
void sort_free();
void radix_sort(GLuint particles, vec3 axis, float z_min, float z_max);

#endif
