/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#ifndef PLANE_MODEL_H
#define PLANE_MODEL_H

#include <vector>

namespace MaliSDK
{
    /**
     * \brief Functions for generating Plane shapes.
     */
    class PlaneModel
    {
    public:
        typedef std::vector<float> coordinates_array;

        /**
         * \brief Get coordinates of points which make up a plane. The plane is located in XZ space.
         *
         * Triangles are made up of 4 components per vertex.
         *
         * \param[out] coordinates Container will be used to store generated coordinates. Cannot be null.
         */
        static void getTriangleRepresentation(coordinates_array& coordinates);

        /**
         * \brief Get U/V 2D texture coordinates that can be mapped onto a plane generated from this class.
         *
         * \param[out] uvCoordinates Container will be used to store generated coordinates. Cannot be null.
         */
        static void getTriangleRepresentationUVCoordinates(coordinates_array& uvCoordinates);

        /**
         * \brief Get normals for plane placed in XZ space.
         *
         * \param[out] normals Container will be used to store generated normals. Cannot be null.
         */
        static void getNormals(coordinates_array& normals);
    };
}
#endif /* PLANE_MODEL_H */