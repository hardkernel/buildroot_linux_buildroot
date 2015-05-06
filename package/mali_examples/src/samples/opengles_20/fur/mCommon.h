/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FUR_COMMON_HPP
#define M_FUR_COMMON_HPP

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef _MSC_VER
#include <time.h>
#include <Winsock2.h>
#else
#include <sys/time.h>
#endif

#define GLES_VERSION 2

#include "Text.h"
#include "Shader.h"
#include "Texture.h"
#include "Matrix.h"
#include "Timer.h"

/* Windows, GNU Linux (x86 and ARM) */
#include <EGL/egl.h>
#include <assert.h>
#include "Platform.h"
#include "EGLRuntime.h"

template <class Type>
int getPtrDiff(Type a1,
               Type a2)
  {
  return (int) (a1 - a2);
  }

template <class Type>
void deleteAndNULL(Type& aValue)
  {
  if (aValue != NULL)
    {
    delete aValue;
    aValue = NULL;
    }
  }

#endif
