/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef TERRAIN_H
#define TERRAIN_H

namespace MaliSDK
{
    // Implements a geometry clipmap algorithm.
    // Paper: http://research.microsoft.com/en-us/um/people/hoppe/geomclipmap.pdf

    /* Window resolution: width. */
    #define WINDOW_WIDTH (800)
    /* Window resolution: width. */
    #define WINDOW_HEIGHT (600)

    // Sets the size of clipmap blocks, NxN vertices per block. Should be power-of-two and no bigger than 64.
    // A clipmap-level is organized roughly as 4x4 blocks with some padding. A clipmap level is a (4N-1) * (4N-1) grid.
    #define CLIPMAP_SIZE 64

    // Number of LOD levels for clipmap.
    #define CLIPMAP_LEVELS 10

    // Distance between vertices.
    #define CLIPMAP_SCALE 0.25f
}
#endif  /* TERRAIN_H */