/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef FRUSTUM_H__
#define FRUSTUM_H__

#include "vector_math.h"

// Representation of a frustum using 6 plane equations.

class AABB;
class Frustum
{
public:
    Frustum();
    Frustum(const mat4& view_projection);
    bool intersects_aabb(const AABB& aabb) const;

private:
    vec4 planes[6];
};

#endif