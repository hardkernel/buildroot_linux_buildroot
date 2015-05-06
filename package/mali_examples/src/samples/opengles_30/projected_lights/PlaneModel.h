/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#ifndef PLANE_MODEL_H
#define PLANE_MODEL_H

#include "VectorTypes.h"
#include "Matrix.h"

/**
 * \brief Functions for generating Plane shapes.
 */
class PlaneModel
{
public:
    /**
    * \brief Get normals for plane placed in XZ space.
    *
    * \param normalsPtrPtr          Deref will be used to store generated normals. Cannot be null.
    * \param numberOfCoordinatesPtr Number of generated coordinates.
    */
    static void getNormals(float** normalsPtrPtr, int* numberOfCoordinatesPtr);

    /** 
     * \brief Get coordinates of points which make up a plane. The plane is located in XZ space.
     *
     * \param coordinatesPtrPtr      Deref will be used to store generated coordinates. Cannot be null.
     * \param numberOfCoordinatesPtr Number of generated coordinates.
     * \param scalingFactor          Scaling factor indicating size of a plane.
     */
    static void getTriangleRepresentation(float** coordinatesPtrPtr, int* numberOfCoordinatesPtr, float scalingFactor);
};
#endif /* PLANE_MODEL_H */