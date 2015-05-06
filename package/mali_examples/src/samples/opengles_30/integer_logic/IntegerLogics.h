/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef INTEGERLOGICS_H
#define INTEGERLOGICS_H

#define GLES_VERSION 3

/* Window width. */
#define WINDOW_W (800)

/* Window height. */
#define WINDOW_H (600)

/* 
 * Vertex array, storing coordinates for a single line filling whole row.
 * The 4th coordinate is set to 0.5 to reduce clip coordinates to [-0.5, 0.5].
 * It is necessary so the offsets are the same as for UV coordinatesin Rule30 shader.
 */
static const float lineVertices[] =
{
    -0.5f, 1.0f, 0.0f, 0.5f,
     0.5f, 1.0f, 0.0f, 0.5f,
};

/*
 * Vertex array, storing coordinates for the quad in the fullscreen.
 * The 4th coordinate is set to 0.5 to keep compatibility with line vertices.
 */
static const float quadVertices[] =
{
    -0.5f,  1.0f, 0.0f, 0.5f,
     0.5f,  1.0f, 0.0f, 0.5f,
    -0.5f, -1.0f, 0.0f, 0.5f,
     0.5f, -1.0f, 0.0f, 0.5f,
};

/* UV array. Used to map texture on both line ends. */
static const float lineTextureCoordinates[] =
{
    0.0f, 1.0f,
    1.0f, 1.0f
};

/* UV array. Used to map texture on the whole quad. */
static const float quadTextureCoordinates[] =
{
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
};

#endif /* INTEGERLOGICS_H */