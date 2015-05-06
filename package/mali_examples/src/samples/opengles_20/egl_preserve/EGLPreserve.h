/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef EGLPRESERVE_H
#define EGLPRESERVE_H

#define GLES_VERSION 2

#include <GLES2/gl2.h>

/* These indices describe the cube triangle strips, separated by degenerate triangles where necessary. */
static const GLubyte cubeIndices[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1,   1, 1,   1, 7, 3, 5,   5, 6,   6, 0, 4, 2,
};

/* Tri strips, so quads are in this order:
 *
 * 2 ----- 3
 * | \     |
 * |   \   |6 - 7
 * |     \ || \ |
 * 0 ----- 14 - 5
 */
static const float cubeVertices[] =
{
    -0.5f, -0.5f,  0.5f, /* 0 */
     0.5f, -0.5f,  0.5f, /* 1 */
    -0.5f,  0.5f,  0.5f, /* 2 */
     0.5f,  0.5f,  0.5f, /* 3 */
    -0.5f,  0.5f, -0.5f, /* 4 */
     0.5f,  0.5f, -0.5f, /* 5 */
    -0.5f, -0.5f, -0.5f, /* 6 */
     0.5f, -0.5f, -0.5f, /* 7 */
};

static const float cubeColors[] =
{
    0.0f, 0.0f, 0.0f, 1.0f, /* 0 */
    1.0f, 0.0f, 0.0f, 1.0f, /* 1 */
    0.0f, 1.0f, 0.0f, 1.0f, /* 2 */
    1.0f, 1.0f, 0.0f, 1.0f, /* 3 */
    0.0f, 0.0f, 1.0f, 1.0f, /* 4 */
    1.0f, 0.0f, 1.0f, 1.0f, /* 5 */
    0.0f, 1.0f, 1.0f, 1.0f, /* 6 */
    1.0f, 1.0f, 1.0f, 1.0f, /* 7 */
};

#endif /* EGLPRESERVE_H */