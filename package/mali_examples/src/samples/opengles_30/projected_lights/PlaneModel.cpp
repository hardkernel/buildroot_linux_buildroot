/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "PlaneModel.h"
#include "Mathematics.h"
#include "Platform.h"

#include <cassert>

using namespace MaliSDK;

/** Please see header for the specification. */
void PlaneModel::getNormals(float** normalsPtrPtr,
                            int*    numberOfCoordinatesPtr)
{
    ASSERT(normalsPtrPtr != NULL,
           "Cannot use null pointer while calculating coordinates.");

    /* 2 triangles, 3 points of triangle, 3 coordinates for each point. */
    const int numberOfNormalsCoordinates = NUMBER_OF_TRIANGLES_IN_QUAD *
                                           NUMBER_OF_TRIANGLE_VERTICES *
                                           NUMBER_OF_POINT_COORDINATES;

    /* Allocate memory for result array. */
    *normalsPtrPtr = (float*) malloc(numberOfNormalsCoordinates * sizeof(float));

    /* Is allocation successful? */
    ASSERT(*normalsPtrPtr != NULL,
           "Could not allocate memory for result array.");

    /* Index of an array we will put new point coordinates at. */
    int currentIndex = 0;

    for (int i = 0; i < numberOfNormalsCoordinates; i += NUMBER_OF_TRIANGLE_VERTICES)
    {
        (*normalsPtrPtr)[currentIndex++] = 0.0f;
        (*normalsPtrPtr)[currentIndex++] = 1.0f;
        (*normalsPtrPtr)[currentIndex++] = 0.0f;
    }

    if (numberOfCoordinatesPtr != NULL)
    {
        *numberOfCoordinatesPtr = numberOfNormalsCoordinates;
    }
}

/** Please see header for the specification. */
void PlaneModel::getTriangleRepresentation(float** coordinatesPtrPtr,
                                           int*    numberOfCoordinatesPtr,
                                           float   scalingFactor)
{
    /* Example:
     *  z   D __________ C
     *  .    |        / |
     * / \   |     /    |
     *  |    |  /       |
     *  |    |/_________|
     *  |   A            B
     *  |----------> x
     */
    /* Coordinates of a points: A, B, C and D as shown in a schema above. */
    const Vec3f coordinatesOfPointA = {-1.0f, 0.0f, -1.0f };
    const Vec3f coordinatesOfPointB = { 1.0f, 0.0f, -1.0f };
    const Vec3f coordinatesOfPointC = { 1.0f, 0.0f,  1.0f };
    const Vec3f coordinatesOfPointD = {-1.0f, 0.0f,  1.0f };

    ASSERT(coordinatesPtrPtr != NULL,
           "Cannot use null pointer while calculating plane coordinates.")

    /* 2 triangles, 3 points of triangle, 3 coordinates for each point. */
    const int numberOfSquareCoordinates = NUMBER_OF_TRIANGLES_IN_QUAD *
                                          NUMBER_OF_TRIANGLE_VERTICES *
                                          NUMBER_OF_POINT_COORDINATES;

    /* Allocate memory for result array. */
    *coordinatesPtrPtr = (float*) malloc(numberOfSquareCoordinates * sizeof(float));

    /* Is allocation successful? */
    ASSERT(*coordinatesPtrPtr != NULL,
           "Could not allocate memory for plane coordinates result array.")

    /* Index of an array we will put new point coordinates at. */
    int currentIndex = 0;

    /* First triangle. */
    /* A */
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointA.x;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointA.y;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointA.z;

    /* B */
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointB.x;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointB.y;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointB.z;

    /* C */
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointC.x;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointC.y;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointC.z;

    /* Second triangle. */
    /* A */
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointA.x;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointA.y;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointA.z;
    /* C */
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointC.x;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointC.y;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointC.z;
    /* D */
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointD.x;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointD.y;
    (*coordinatesPtrPtr)[currentIndex++] = coordinatesOfPointD.z;

    if (scalingFactor != 1.0f)
    {
        for (int i = 0; i < numberOfSquareCoordinates; i++)
        {
            (*coordinatesPtrPtr)[i] *= scalingFactor;
        }
    }

    if (numberOfCoordinatesPtr != NULL)
    {
        *numberOfCoordinatesPtr = numberOfSquareCoordinates;
    }
}