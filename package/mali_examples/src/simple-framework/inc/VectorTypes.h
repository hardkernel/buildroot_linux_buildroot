/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef VECTORTYPES_H
#define VECTORTYPES_H

/**
 * \file VectorTypes.h
 * \brief Vector types
 */

namespace MaliSDK
{
    /**
     * \brief A 2D integer vector
     *
     * Struct containing two integers, useful for representing 2D coordinates.
     */
    typedef struct
    {
        int x, y;
    } Vec2;

    /**
     * \brief A 3D integer vector
     *
     * Struct containing three integers, useful for representing 3D coordinates.
     */
    typedef struct
    {
        int x, y, z;
    } Vec3;

    /**
     * \brief A 4D integer vector
     *
     * Struct containing four integers.
     */
    typedef struct
    {
        int x, y, z, w;
    } Vec4;


    /**
     * \brief A 2D floating point vector
     *
     * Struct containing two floating point numbers, useful for representing 2D coordinates.
     */
    typedef struct
    {
        float x, y;
    } Vec2f;

    /**
     * \brief A 3D floating point vector
     *
     * Struct containing three floating point numbers, useful for representing 3D coordinates.
     */
    typedef struct
    {
        float x, y, z;
    } Vec3f;


    /**
     * \brief A 4D floating point vector
     *
     * Struct containing four floating point numbers.
     */
    typedef struct
    {
        float x, y, z, w;
    } Vec4f;
}
#endif /* VECTORTYPES_H */

