/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "AABB.h"

AABB::AABB(const vec3& base, const vec3& offset)
{
    this->base = base;
    this->offset = offset;
}

vec3 AABB::center() const
{
    return base + vec3(0.5f) * offset;
}

vec3 AABB::corner(unsigned int index) const
{
    vec3 ret = base;
    if (index & 1)
        ret.c.x += offset.c.x;
    if (index & 2)
        ret.c.y += offset.c.y;
    if (index & 4)
        ret.c.z += offset.c.z;

    return ret;
}
