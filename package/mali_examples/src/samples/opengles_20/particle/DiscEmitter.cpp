/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "DiscEmitter.h"
#include "Platform.h"

#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "Mathematics.h"

namespace MaliSDK
{
    DiscEmitter::DiscEmitter(void)
    {
        discRadius = 0.5f;
        maxEmissionAngle = M_PI/10;
    }

    DiscEmitter::DiscEmitter(float rad, float angle):discRadius(rad),maxEmissionAngle(angle * M_PI / 180.0f)
    {}

    /* [Particle generation] */
    void DiscEmitter::getParticle(Particle &part)
    {
        /* Generate  a random number in the interval [0,1]. */
        float rad = (float)rand()/((float)RAND_MAX);

        /* Generate  a random number in the interval [0,1]. */
        float polarAngle = (float)rand()/((float)RAND_MAX);
        polarAngle *= 2 * M_PI;

        part.initPos.x = discRadius * rad * cos(polarAngle);
        part.initPos.y = discRadius * rad * sin(polarAngle);
        part.initPos.z = 0.0f;

        /* Generate  a random number in the interval [0,1]. */
        float azimuthAngle = (float)rand()/((float)RAND_MAX);
        azimuthAngle *= maxEmissionAngle;

        part.initVel.x = sin(azimuthAngle) * cos(polarAngle);
        part.initVel.y = sin(azimuthAngle) * sin(polarAngle);
        part.initVel.z = cos(azimuthAngle);

        part.lifetime = (float)rand()/((float)RAND_MAX);

        /* Generate  a random number in the interval [0,1]. */
        part.delay = (float)rand()/((float)RAND_MAX);

    }
    /* [Particle generation] */
}
