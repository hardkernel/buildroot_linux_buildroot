/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef FETCH_DEPTH_H
#define FETCH_DEPTH_H

#define GLES_VERSION 2

#include <GLES2/gl2.h>

/* 3D data. Vertex range -0.5..0.5 in all axes.
* Z -0.5 is near, 0.5 is far. */
const float verticesCube[] =
{
    /* Front face. */
    /* Bottom left */
    -0.5,  0.5, -0.5,
    0.5, -0.5, -0.5,
    -0.5, -0.5, -0.5,
    /* Top right */
    -0.5,  0.5, -0.5,
    0.5,  0.5, -0.5,
    0.5, -0.5, -0.5,
    /* Left face */
    /* Bottom left */
    -0.5,  0.5,  0.5,
    -0.5, -0.5, -0.5,
    -0.5, -0.5,  0.5,
    /* Top right */
    -0.5,  0.5,  0.5,
    -0.5,  0.5, -0.5,
    -0.5, -0.5, -0.5,
    /* Top face */
    /* Bottom left */
    -0.5,  0.5,  0.5,
    0.5,  0.5, -0.5,
    -0.5,  0.5, -0.5,
    /* Top right */
    -0.5,  0.5,  0.5,
    0.5,  0.5,  0.5,
    0.5,  0.5, -0.5,
    /* Right face */
    /* Bottom left */
    0.5,  0.5, -0.5,
    0.5, -0.5,  0.5,
    0.5, -0.5, -0.5,
    /* Top right */
    0.5,  0.5, -0.5,
    0.5,  0.5,  0.5,
    0.5, -0.5,  0.5,
    /* Back face */
    /* Bottom left */
    0.5,  0.5,  0.5,
    -0.5, -0.5,  0.5,
    0.5, -0.5,  0.5,
    /* Top right */
    0.5,  0.5,  0.5,
    -0.5,  0.5,  0.5,
    -0.5, -0.5,  0.5,
    /* Bottom face */
    /* Bottom left */
    -0.5, -0.5, -0.5,
    0.5, -0.5,  0.5,
    -0.5, -0.5,  0.5,
    /* Top right */
    -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,
    0.5, -0.5,  0.5,
};

const float colorsCube[] =
{
    /* Front face */
    /* Bottom left */
    1.0, 0.0, 0.0, /* red */
    1.0, 0.0, 0.0, /* red */
    1.0, 0.0, 0.0, /* red */
    /* Top right */
    1.0, 0.0, 0.0, /* red */
    1.0, 0.0, 0.0, /* red */
    1.0, 0.0, 0.0, /* red */

    /* Left face */
    /* Bottom left */
    0.0, 0.0, 1.0, /* blue */
    0.0, 0.0, 1.0, /* blue */
    0.0, 0.0, 1.0, /* blue */
    /* Top right */
    0.0, 0.0, 1.0, /* blue */
    0.0, 0.0, 1.0, /* blue */
    0.0, 0.0, 1.0, /* blue */

    /* Top face */
    /* Bottom left */
    1.0, 1.0, 1.0, /* white */
    1.0, 1.0, 1.0, /* white */
    1.0, 1.0, 1.0, /* white */
    /* Top right */
    1.0, 1.0, 1.0, /* white */
    1.0, 1.0, 1.0, /* white */
    1.0, 1.0, 1.0, /* white */

    /* Right face */
    /* Bottom left */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 1.0, 0.0, /* yellow */
    /* Top right */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 1.0, 0.0, /* yellow */

    /* Back face */
    /* Bottom left */
    0.0, 1.0, 1.0, /* cyan */
    0.0, 1.0, 1.0, /* cyan */
    0.0, 1.0, 1.0, /* cyan */
    /* Top right */
    0.0, 1.0, 1.0, /* cyan */
    0.0, 1.0, 1.0, /* cyan */
    0.0, 1.0, 1.0, /* cyan */

    /* Bottom face */
    /* Bottom left */
    1.0, 0.0, 1.0, /* magenta */
    1.0, 0.0, 1.0, /* magenta */
    1.0, 0.0, 1.0, /* magenta */
    /* Top right */
    1.0, 0.0, 1.0, /* magenta */
    1.0, 0.0, 1.0, /* magenta */
    1.0, 0.0, 1.0, /* magenta */
};


/* 3D data. Vertex range -0.5..0.5 in all axes.
* Z -0.5 is near, 0.5 is far. */
const float verticesGlass[] =
{
    0.3, -0.5,  0,
    0.3,  0.8,  0,
    -0.3, -0.5,  0,
    -0.3,  0.8,  0,
};

const float colorsGlass[] =
{
    0.0, 1.0, 0.0, 0.3, /* green */
    0.0, 1.0, 0.0, 0.3, /* green */
    0.0, 1.0, 0.0, 0.3, /* green */
    0.0, 1.0, 0.0, 0.3, /* green */
};

#endif /* FETCH_DEPTH_H */
