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

#include "Mathematics.h"
#include "Platform.h"
#include "VectorTypes.h"

/**
* \brief Functions for generating cube shapes.
*/
class CubeModel
{
public:
   /**
    * \brief Create normals for a cube.
    *
    * \param normalsPtrPtr          Deref will be used to store generated coordinates.
    *                               Cannot be null.
    * \param numberOfCoordinatesPtr Number of generated coordinates.
    */
    static void getNormals(float** normalsPtrPtr, int* numberOfCoordinatesPtr);

   /**
    * \brief Compute coordinates of points which make up a ube shape.
    *
    * \param coordinatesPtrPtr      Deref will be used to store generated coordinates.
    *                               Cannot be null.
    * \param numberOfCoordinatesPtr Number of generated coordinates.
    * \param scalingFactor          Scaling factor indicating size of a cube.
    */
    static void getTriangleRepresentation(float** coordinatesPtrPtr,
                                          int*    numberOfCoordinatesPtr,
                                          float   scalingFactor);
};
#endif /* CUBE_MODEL_H */