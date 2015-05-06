/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef AABB_H__
#define AABB_H__

#include "vector_math.h"

// Axis-aligned bounding box. Used for frustum-culling.
// Represents a box with corners base and base + offset which encapsulate an entire mesh.

class AABB
{
public:
    AABB(const vec3& base, const vec3& offset);
    vec3 center() const;

    vec3 corner(unsigned int index) const;

private:
    vec3 base;
    vec3 offset;
};

#endif
