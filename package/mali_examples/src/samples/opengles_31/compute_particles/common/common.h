/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <GLES3/gl31.h>
#include "EGLRuntime.h"
#include "Platform.h"

#include <string>
#include <vector>
#include <stdint.h>
#include "matrix.h"
using namespace MaliSDK;
using std::string;
using std::vector;

#define ASSERT(x, s)                                                \
if (!(x))                                                           \
{                                                                   \
    LOGE("Assertion failed at %s:%i\n%s\n", __FILE__, __LINE__, s); \
    exit(1);                                                        \
}

#endif /* COMMON_H */
