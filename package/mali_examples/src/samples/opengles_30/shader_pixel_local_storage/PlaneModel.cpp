/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "PlaneModel.h"

namespace MaliSDK
{
    void PlaneModel::getTriangleRepresentationUVCoordinates(coordinates_array& uvCoordinates)
    {
        /* Example:
         *  v   D __________ C
         *  .    |        / |
         * / \   |     /    |
         *  |    |  /       |
         *  |    |/_________|
         *  |   A            B
         *  |----------> u
         */

        /* 2 triangles, 3 points of triangle, 2 coordinates for each point. */
        const int numberOfUVCoordinates = 2 * 3 * 2;

        /* Allocate memory for result array. */
        uvCoordinates.reserve(numberOfUVCoordinates);

        /*** First triangle. ***/
        uvCoordinates.push_back(0); /* A.u */
        uvCoordinates.push_back(0); /* A.v */
        uvCoordinates.push_back(1); /* B.u */
        uvCoordinates.push_back(0); /* B.v */
        uvCoordinates.push_back(1); /* C.u */
        uvCoordinates.push_back(1); /* C.v */

        /*** Second triangle. ***/
        uvCoordinates.push_back(0); /* A.u */
        uvCoordinates.push_back(0); /* A.v */
        uvCoordinates.push_back(1); /* C.u */
        uvCoordinates.push_back(1); /* C.v */
        uvCoordinates.push_back(0); /* D.u */
        uvCoordinates.push_back(1); /* D.v */
    }

    void PlaneModel::getTriangleRepresentation(coordinates_array& coordinates)
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

        /* 2 triangles, 3 points of triangle, 4 coordinates for each point. */
        const int numberOfSquareCoordinates = 2 * 3 * 4;

        /* Allocate memory for result array. */
        coordinates.reserve(numberOfSquareCoordinates);

        /* First triangle. */
        coordinates.push_back( 1); /* C.x */
        coordinates.push_back( 0); /* C.y */
        coordinates.push_back( 1); /* C.z */

        coordinates.push_back( 1); /* B.x */
        coordinates.push_back( 0); /* B.y */
        coordinates.push_back(-1); /* B.z */

        coordinates.push_back(-1); /* A.x */
        coordinates.push_back( 0); /* A.y */
        coordinates.push_back(-1); /* A.z */

        /* Second triangle. */
        coordinates.push_back(-1); /* D.x */
        coordinates.push_back( 0); /* D.y */
        coordinates.push_back( 1); /* D.z */

        coordinates.push_back( 1); /* C.x */
        coordinates.push_back( 0); /* C.y */
        coordinates.push_back( 1); /* C.z */

        coordinates.push_back(-1); /* A.x */
        coordinates.push_back( 0); /* A.y */
        coordinates.push_back(-1); /* A.z */
    }

    void PlaneModel::getNormals(coordinates_array& normals)
    {
        /* 2 triangles, 3 points of triangle, 3 coordinates for each point. */
        const int numberOfNormalsCoordinates = 2 * 3 * 3;

        /* Allocate memory for result array. */
        normals.reserve(numberOfNormalsCoordinates);

        for (int i = 0; i < numberOfNormalsCoordinates; i+=3)
        {
            normals.push_back(0);
            normals.push_back(1);
            normals.push_back(0);
        }
    }
}
