/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

namespace MaliSDK
{
    /**
     * \brief Functions for generating geometrical shapes.
     */
    class Geometry
    {
    public:
        /**
         * \brief Determines an array of indices defining a mesh of control points for instanced torus patches.
         *        To simplify mathemathics, it is assumed that torus model consists of 12 circles, each built of
         *        12 points, so it is easy to divide each circle into 4 quadrants and define Bezier surfaces
         *        approximating perfectly round torus.
         *
         * \param patchDimension            [in]  Number of control points in one dimension for a patch.
         * \param patchInstancesCount       [in]  Number of instances needed to draw the whole torus.
         * \param controlPointsIndicesCount [in]  Number of indices needed to create a control mesh.
         * \param controlPointsIndices      [out] Deref will be used to store control points indices. Cannot be null.
         */
        static void calculateTorusControlPointsIndices(unsigned int patchDimension, unsigned int patchInstancesCount, unsigned int controlPointsIndicesCount, unsigned int* controlPointsIndices);

        /**
         * \brief Determines patch data for an instanced torus model.
         *
         * \param patchDensity              [in]  Number of vertices in one edge of a patch.
         * \param patchVertices             [out] Deref will be used to store patch vertices. Cannot be null.
         * \param patchTriangleIndices      [out] Deref will be used to store indices of triangle vertices. Cannot be null.
         */
        static void calculateTorusPatchData(unsigned int patchDensity, float* patchVertices, unsigned int* patchTriangleIndices);

        /**
         * \brief Determines indices for glDrawElements() call for wireframed torus.
         *
         * \param circlesCount         [in]  Number of circles in torus model.
         * \param pointsPerCircleCount [in]  Number of points in one circle.
         * \param indices              [out] Deref will be used to store calculated indices.
         */
        static void calculateTorusWireframeIndices(unsigned int circlesCount, unsigned int pointsPerCircleCount, unsigned int* indices);

        /**
         * \brief Generate vertices of the torus model. The vertices are grouped in circlesCount circles,
         *        where each circle consists of pointsPerCircleCount vertices.
         *
         * \param torusRadius          [in]  Distance between the center of torus and the center of its tube.
         * \param circleRadius         [in]  Radius of circles that model the tube.
         * \param circlesCount         [in]  Number of circles in torus model.
         * \param pointsPerCircleCount [in]  Number of points in one circle.
         * \param torusVertices        [out] Deref will be used to sotre generated vertices. Cannot be null.
         */
        static void generateTorusVertices(float torusRadius, float cirlceRadius, unsigned int circlesCount, unsigned int poitsPerCircleCount, float* torusVertices);

        /**
         * \brief Generate torus vertices applying distortions to some of them. The distortions in control mesh
         *        are needed for proper construction of Bezier surface patches. It is assumed that each patch consists of
         *        4 control rows and columns. Hence, in each column and each row, we can distinguish 2 middle control points
         *        and 2 edge control points, which are shared between patches. The middle control points have to be moved 
         *        in such a way that C1 continuity between patches is satisfied.
         *        Implemented algorithm assumes that each construction circle contains 12 points and the torus model consists
         *        of 12 circles.
         *
         * \param torusRadius   [in]  Distance between the center of torus and the center of its tube.
         * \param circleRadius  [in]  Radius of circles that model the tube.
         * \param torusVertices [out] Deref will be used to sotre generated vertices. Cannot be null.
         */
        static void generateBezierTorusVertices(float torusRadius, float circleRadius, float* torusVertices);

        /**
         * \brief Compute coordinates of points which make up a sphere. 
         *
         *  \param radius                [in]  Radius of a sphere. Has to be greater than zero.
         *  \param numberOfSamples       [in]  A sphere consists of @param numberOfSamples circles and @param numberOfSamples points lying on one circle. Has to be greater than zero.
         *  \param numberOfCoordinates   [out] Number of generated coordinates.
         *  \param sphereCoordinates     [out] Deref will be used to store generated coordinates. Cannot be null.
         */
        static void getSpherePointRepresentation(const float radius, const int numberOfSamples, int* numberOfCoordinates, float** sphereCoordinates);

        /** 
         * \brief Create triangular representation of a sphere. 
         *        For each point of each circle (excluding last circle) there are two triangles created according to rule described in example below:
         *
         *                       A2___________.B2
         *                       . \       .  / |
         *                       |. \   .    /  |
         *                       | . A1____B1   |
         *                       |  . |     |.  |
         *                       |   D1____C1 . |
         *                       |  /    .   \ .|
         *                       | /  .       \ .
         *                      D2 .___________C2
         *
         *         Points named A1, B1, C1 and D1 create a first circle of sphere and points named A2, B2, C2 and D2 create the second one 
         *         (if numberOfSamples is equal to 4). For each loop iteration, for each point lying at one circle of sphere there are 2 triangles created:
         *         for point A1:  A1 B1 B2,   A1 B2 A2
         *         for point B1:  B1 C1 C2,   B1 C2 B2
         *         for point C1:  C1 D1 D2,   C1 D2 C2
         *         for point D1:  D1 A1 A2,   D1 A2 D2
         *
         *  \param radius                     [in]  Radius of a sphere. Has to be greater than zero.
         *  \param numberOfSamples            [in]  A sphere consists of \param numberOfSamples circles and \param numberOfSamples points lying on one circle. Has to be greater than zero.
         *  \param numberOfCoordinates        [out] Number of generated coordinates. 
         *  \param sphereTrianglesCoordinates [out] Deref will be used to store generated coordinates. Cannot be null.
         */
        static void getSphereTriangleRepresentation(const float radius, const int numberOfSamples, int* numberOfCoordinates, float** sphereTrianglesCoordinates);

        /** 
         * \brief Compute coordinates of points which make up a cube.
         *
         *  \param scalingFactor            [in]  Scaling factor indicating size of a cube.
         *  \param numberOfCoordinates      [out] Number of generated coordinates.
         *  \param cubeTrianglesCoordinates [out] Deref will be used to store generated coordinates. Cannot be null.
         */
        static void getCubeTriangleRepresentation(float scalingFactor, int* numberOfCoordinates, float** cubeTrianglesCoordinates);

        /** \brief Create normals for a cube which was created with getCubeTriangleRepresentation function.
         *  \param numberOfCoordinates [out] Number of generated coordinates.
         *  \param cubeNormals         [out] Deref will be used to store generated coordinates. Cannot be null.
         */
        static void getCubeNormals(int* numberOfCoordinates, float** cubeNormals);

        /** \brief Get coordinates of points which make up a square. Square is located in XZ space.
         *  \param scalingFactor       [in]  Scaling factor indicating size of a square.
         *  \param numberOfCoordinates [out] Number of generated coordinates.
         *  \param squareCoordinates   [out] Deref will be used to store generated coordinates. Cannot be null.
         */
        static void getSquareTriangleRepresentationInXZSpace(float scalingFactor, int* numberOfCoordinates, float** squareCoordinates);

        /** \brief Get normals for square placed in XZ space.
         *  \param numberOfCoordinates [out] Number of generated coordinates.
         *  \param squareCoordinates   [out] Deref will be used to store generated coordinates. Cannot be null.
         */
        static void getSquareXZNormals(int* numberOfCoordinates, float** squareNormals);

    };
}
#endif /* GEOMETRY_H */