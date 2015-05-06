/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "CubeModel.h"

namespace MaliSDK
{
    void CubeModel::append(coordinates_array& target, coordinates_array& appendee)
    {
        target.insert(target.end(), appendee.begin(), appendee.end());
    }

    void CubeModel::getTriangleRepresentation(float scalingFactor, coordinates_array& coordinates)
    {
        /* 6 faces, 2 triangles for each face, 3 points of triangle, 3 coordinates for each point. */
        const int numberOfCubeTriangleCoordinates = 6 * 2 * 3 * 3;

        /* Allocate memory for result array. */
        coordinates.reserve(numberOfCubeTriangleCoordinates);

        /* Example:
         * Coordinates for cube points:
         * A -1.0f,  1.0f,  1.0f
         * B -1.0f,  1.0f, -1.0f
         * C  1.0f,  1.0f, -1.0f
         * D  1.0f,  1.0f,  1.0f
         * E -1.0f, -1.0f,  1.0f
         * F -1.0f, -1.0f, -1.0f
         * G  1.0f, -1.0f, -1.0f
         * H  1.0f, -1.0f,  1.0f
         * Create 2 triangles for each face of the cube. Vertices are written in clockwise order.
         *       B ________ C
         *      / |     /  |
         *  A ......... D  |
         *    .   |   .    |
         *    .  F|_ _.___ |G
         *    . /     .  /
         *  E ......... H
         */
        /* Scaled cube points */
        const int coordinates_count = 3;
        float vertex_A_coordinates_data[coordinates_count] = {-scalingFactor,  scalingFactor,  scalingFactor};
        float vertex_B_coordinates_data[coordinates_count] = {-scalingFactor,  scalingFactor, -scalingFactor};
        float vertex_C_coordinates_data[coordinates_count] = { scalingFactor,  scalingFactor, -scalingFactor};
        float vertex_D_coordinates_data[coordinates_count] = { scalingFactor,  scalingFactor,  scalingFactor};
        float vertex_E_coordinates_data[coordinates_count] = {-scalingFactor, -scalingFactor,  scalingFactor};
        float vertex_F_coordinates_data[coordinates_count] = {-scalingFactor, -scalingFactor, -scalingFactor};
        float vertex_G_coordinates_data[coordinates_count] = { scalingFactor, -scalingFactor, -scalingFactor};
        float vertex_H_coordinates_data[coordinates_count] = { scalingFactor, -scalingFactor,  scalingFactor};

        coordinates_array vertexA(vertex_A_coordinates_data, vertex_A_coordinates_data + coordinates_count);
        coordinates_array vertexB(vertex_B_coordinates_data, vertex_B_coordinates_data + coordinates_count);
        coordinates_array vertexC(vertex_C_coordinates_data, vertex_C_coordinates_data + coordinates_count);
        coordinates_array vertexD(vertex_D_coordinates_data, vertex_D_coordinates_data + coordinates_count);
        coordinates_array vertexE(vertex_E_coordinates_data, vertex_E_coordinates_data + coordinates_count);
        coordinates_array vertexF(vertex_F_coordinates_data, vertex_F_coordinates_data + coordinates_count);
        coordinates_array vertexG(vertex_G_coordinates_data, vertex_G_coordinates_data + coordinates_count);
        coordinates_array vertexH(vertex_H_coordinates_data, vertex_H_coordinates_data + coordinates_count);

        /* Fill the array with coordinates. */
        /* Top face. */
        /* CBA */
        append(coordinates, vertexC);
        append(coordinates, vertexB);
        append(coordinates, vertexA);

        /* DCA */
        append(coordinates, vertexD);
        append(coordinates, vertexC);
        append(coordinates, vertexA);

        /* Bottom face. */
        /* EFG */
        append(coordinates, vertexE);
        append(coordinates, vertexF);
        append(coordinates, vertexG);

        /* EGH */
        append(coordinates, vertexE);
        append(coordinates, vertexG);
        append(coordinates, vertexH);

        /* Back face. */
        /* BCG */
        append(coordinates, vertexB);
        append(coordinates, vertexC);
        append(coordinates, vertexG);

        /* FBG */
        append(coordinates, vertexF);
        append(coordinates, vertexB);
        append(coordinates, vertexG);

        /* Front face. */
        /* DAE */
        append(coordinates, vertexD);
        append(coordinates, vertexA);
        append(coordinates, vertexE);

        /* HDE */
        append(coordinates, vertexH);
        append(coordinates, vertexD);
        append(coordinates, vertexE);

        /* Right face. */
        /* CDH */
        append(coordinates, vertexC);
        append(coordinates, vertexD);
        append(coordinates, vertexH);

        /* GCH */
        append(coordinates, vertexG);
        append(coordinates, vertexC);
        append(coordinates, vertexH);

        /* Left face. */
        /* ABF */
        append(coordinates, vertexA);
        append(coordinates, vertexB);
        append(coordinates, vertexF);

        /* EAF */
        append(coordinates, vertexE);
        append(coordinates, vertexA);
        append(coordinates, vertexF);
    }

    void CubeModel::getNormals(coordinates_array& normals)
    {
        /* Set the same normals for both triangles from each face.
         * For details: see example for getCubeTriangleRepresentation() function.
         */

        /* 6 faces, 2 triangles for each face, 3 points of triangle, 3 coordinates for each point. */
        const int numberOfCubeNormalsCoordinates = 6 * 2 * 3 * 3;

        /* Allocate memory for result array. */
        normals.reserve(numberOfCubeNormalsCoordinates);

        /* There are 2 triangles for each face. Each triangle consists of 3 vertices. */
        int numberOfCoordinatesForOneFace = 2 * 3;

        /* Top face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            normals.push_back( 0.0f);
            normals.push_back( 1.0f);
            normals.push_back( 0.0f);
        }

        /* Bottom face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            normals.push_back( 0.0f);
            normals.push_back(-1.0f);
            normals.push_back( 0.0f);
        }

        /* Back face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            normals.push_back( 0.0f);
            normals.push_back( 0.0f);
            normals.push_back(-1.0f);
        }

        /* Front face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            normals.push_back( 0.0f);
            normals.push_back( 0.0f);
            normals.push_back( 1.0f);
        }

        /* Right face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            normals.push_back( 1.0f);
            normals.push_back( 0.0f);
            normals.push_back( 0.0f);
        }

        /* Left face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            normals.push_back(-1.0f);
            normals.push_back( 0.0f);
            normals.push_back( 0.0f);
        }
    }
}