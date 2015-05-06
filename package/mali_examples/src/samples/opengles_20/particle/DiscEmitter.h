/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef DISC_EMITTER_H
#define DISC_EMITTER_H

#include "VectorTypes.h"

namespace MaliSDK
{
    /**
     * \brief A structure used to hold particle data.
     */
    typedef struct
    {
        /**
         * \brief Particle lifetime.
         */
        float lifetime;
        /**
         * \brief Particle creation delay time.
         */
        float delay;
        /**
         * \brief Particle initial position.
         */
        Vec3f initPos;
        /**
         * \brief Particle initial velocity.
         */
        Vec3f initVel;
    } Particle;

    /**
     * \brief A class used to represent an emitter of random particles from a disc.
     */
    class DiscEmitter
    {
    private:
        /**
         * \brief The radius of the disc.
         */
        float discRadius;
        /**
         * \brief Maximum emission angle in degrees relative to disc normal (0,0,1).
         */
        float maxEmissionAngle;
    public:
        /**
         * \brief Default constructor.
         */
        DiscEmitter(void);
        /**
         * \brief Constructor.
         * param[in] rad The radius of the emitter disc.
         * param[in] angle Maximum emission angle in degrees [0, 90].
         */
        DiscEmitter(float discRadius, float angle);
        /**
         * \brief Produces a random particle.
         * \param[out] part A particle.
         */
        void getParticle(Particle &part);
    };
}

#endif /* DISC_EMITTER_H */
