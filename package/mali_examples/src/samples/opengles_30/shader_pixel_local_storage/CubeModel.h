/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#ifndef CUBE_MODEL_H
#define CUBE_MODEL_H

#include <vector>

namespace MaliSDK
{
    /**
     * \brief Functions for generating cube shapes.
     */
    class CubeModel
    {
    public:
        typedef std::vector<float> coordinates_array;

    private:
        /**
         * \brief Appends content of appendee to target
         *
         * \param      target   Container to append data to
         * \param[out] appendee Container to append data from
         */
        static void append(coordinates_array& target, coordinates_array& appendee);

    public:
        /**
         * \brief Compute coordinates of points which make up a cube.
         *
         * \param[in] scalingFactor Scaling factor indicating size of a cube.
         * \param[out] coordinates Container will be used to store generated coordinates. Cannot be null.
         */
        static void getTriangleRepresentation(float scalingFactor, coordinates_array& coordinates);

        /**
         * \brief Create normals for a cube which was created with getTriangleRepresentation() function.
         *
         * \param[out] normals Container will be used to store generated normal vectors. Cannot be null.
         */
        static void getNormals(coordinates_array& normals);
    };
}
#endif /* CUBE_MODEL_H */