/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#ifndef MATHEMATICS_H
    #define MATHEMATICS_H

    #include "VectorTypes.h"
    
    #include <cmath>
    #include <cstdlib>

    #ifndef M_PI
        /** \brief The value of pi approximation. */
        #define M_PI 3.14159265358979323846f
    #endif /* M_PI */

    #ifndef NUMBER_OF_CUBE_FACES
        /** \brief Number of faces which make up a cubic shape. */
        #define NUMBER_OF_CUBE_FACES (6)
    #endif /* NUMBER_OF_CUBE_FACES */

    #ifndef NUMBER_OF_POINT_COORDINATES
        /** \brief Number of coordinates for a point in 3D space. */
        #define NUMBER_OF_POINT_COORDINATES (3)
    #endif /* NUMBER_OF_POINT_COORDINATES */

    #ifndef NUMBER_OF_TRIANGLE_VERTICES
        /** \brief Number of vertices which make up a traingle shape. */
        #define NUMBER_OF_TRIANGLE_VERTICES (3)
    #endif /* NUMBER_OF_TRIANGLE_VERTICES */

    #ifndef NUMBER_OF_TRIANGLES_IN_QUAD
       /** \brief Number of triangles which make up a quad. */
        #define NUMBER_OF_TRIANGLES_IN_QUAD (2)
    #endif /* NUMBER_OF_TRIANGLES_IN_QUAD */

    /**
     * \brief Convert an angle in degrees to radians.
     *
     * \param degrees The angle (in degrees) to convert to radians.
     *
     * \return As per description.
     */
    inline float degreesToRadians(float degrees)
    {
        return M_PI * degrees / 180.0f;
    }

    /**
    * \brief Convert an angle in radians to degrees.
    *
    * \param radians The angle (in radians) to convert to degrees.
    *
    * \return As per description.
    */
    inline float radiansToDegrees(float radians)
    {
        return radians * 180.0f / M_PI;
    }

    /**
    * \brief Calculates the Euclidean distance between two points in 3D space.
    *
    * \param startPoint Coordinates of a start point in 3D space.
    * \param endPoint   Coordinates of an end point in 3D space.
    *
    * \return As per description.
    */
    inline float getDistanceBetweenPointsIn3DSpace(Vec3f startPoint, Vec3f endPoint)
    {
        return sqrt(((startPoint.x - endPoint.x) * (startPoint.x - endPoint.x)) +
                    ((startPoint.y - endPoint.y) * (startPoint.y - endPoint.y)) +
                    ((startPoint.z - endPoint.z) * (startPoint.z - endPoint.z))
                    );;
    }
#endif /* MATHEMATICS_H */