/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "Geometry.h"
#include "Platform.h"

#include <cstdlib>
#include <cmath>
#include <cassert>

namespace MaliSDK
{
    /* See header for specification. */
    void Geometry::calculateTorusControlPointsIndices(unsigned int patchDimension, unsigned int patchInstancesCount, unsigned int controlPointsIndicesCount, unsigned int* controlPointsIndices)
    {
        if (controlPointsIndices == NULL)
        {
            LOGE("Cannot use null pointer while calculating control points indices.");
    
            return;
        }
    
        /* Definition of needed constants. Torus continuity cannot be guaranteed with other parameters. */
        const unsigned int pointsPerCircleCount = 12;
        const unsigned int circlesCount         = 12;
        const unsigned int torusVerticesCount   = pointsPerCircleCount * circlesCount;
    
        /* Index of a vertex from which a patch starts. */
        unsigned int startIndex = 0;
        /* Index of a circle from which vertex indices are currently taken. */
        unsigned int currentCircle = 0;
    
        /* Index variable. */
        unsigned int index = 0;
    
        /* Loop that creates patches for each instance of patch primitives. Successive patches wrap around torus horizontally. */
        for (unsigned int instanceIndex = 0; instanceIndex < patchInstancesCount; ++instanceIndex)
        {
            /* Iterate in horizontal axis. */
            for (unsigned int x = 0; x < patchDimension; ++x)
            {
                /* Determine index of current circle from which the vertex indices are taken. */
                currentCircle = startIndex / pointsPerCircleCount;
    
                /* Iterate in vertical axis. */
                for (unsigned int y = 0; y < patchDimension; ++y)
                {
                    unsigned int currentIndex = startIndex + y;
    
                    /* Make closing patches end up at the very first vertex of each circle. */
                    if (currentIndex >= pointsPerCircleCount * (currentCircle + 1))
                    {
                        currentIndex -= pointsPerCircleCount;
                    }
    
                    controlPointsIndices[index++] = currentIndex;
    
                    assert(index <= controlPointsIndicesCount);
                }
    
                /* Get indices from the next circle. */
                startIndex += pointsPerCircleCount;
    
                /* Make closing patches end up at the very first circle. */
                if (startIndex >= torusVerticesCount)
                {
                    startIndex -= torusVerticesCount;
                }
            }
    
            /* Neighbouring patches always share one edge, so start index of the next patch should start from the last column of the previous patch. */
            startIndex -= pointsPerCircleCount;
    
            /* When the whole row is finished, move to the next one. */
            if (currentCircle == 0)
            {
                startIndex += patchDimension - 1;
            }
        }
    }
    
    /* See header for specification. */
    void Geometry::calculateTorusPatchData(unsigned int patchDensity, float* patchVertices, unsigned int* patchTriangleIndices)
    {
        if (patchVertices == NULL || patchTriangleIndices == NULL)
        {
            LOGE("Cannot use null pointers while calculating patch data.");
    
            return;
        }

        /* Total number of components describing a patch (only U/V components are definied). */
        const unsigned int patchComponentsCount = patchDensity * patchDensity * 2;
        /* Number of indices that needs to be defined to draw quads consisted of triangles (6 points per quad needed) over the entire patch. */
        const unsigned int patchTriangleIndicesCount = (patchDensity - 1) * (patchDensity - 1) * 6;

        /* Number of components in a single vertex. */
        const unsigned int uvComponentsCount = 2;
        /* Number of vertices needed to draw a quad as two separate triangles. */
        const unsigned int verticesPerQuadCount = 6;
    
        /* Current index of a patch vertex. */
        unsigned int uvIndex = 0;
        /* Current index for indices array. */
        unsigned int triangleVertexIndex = 0;
    
        for (unsigned int x = 0; x < patchDensity; ++x)
        {
            /* Horizontal component. */
            float u = (float) x / (patchDensity - 1);
    
            for (unsigned int y = 0; y < patchDensity; ++y)
            {
                /* Vertical component. */
                float v = (float) y / (patchDensity - 1);
    
                patchVertices[uvIndex++] = u;
                patchVertices[uvIndex++] = v;
    
                assert(uvIndex <= patchComponentsCount);
            }
        }
    
        /*
         * Indices are determined in the following manner:
         * 
         * 0 -> 1 -> 16 -> 16 -> 1 -> 17 -> 1 -> 2 -> 17 -> 17 -> 2 -> 18 -> ...
         *
         * 2----18----34---...
         * |  /  |  /  |
         * | /   | /   |
         * 1----17----33---...
         * |  /  |  /  |
         * | /   | /   |
         * 0----16----32----...
         */
        for (unsigned int x = 0; x < patchDensity - 1; ++x)
        {
            for (unsigned int y = 0; y < patchDensity - 1; ++y)
            {
                patchTriangleIndices[triangleVertexIndex++] = patchDensity *  x      + y;
                patchTriangleIndices[triangleVertexIndex++] = patchDensity *  x      + y + 1;
                patchTriangleIndices[triangleVertexIndex++] = patchDensity * (x + 1) + y;
    
                patchTriangleIndices[triangleVertexIndex++] = patchDensity * (x + 1) + y;
                patchTriangleIndices[triangleVertexIndex++] = patchDensity *  x      + y + 1;
                patchTriangleIndices[triangleVertexIndex++] = patchDensity * (x + 1) + y + 1;
    
                assert(triangleVertexIndex <= patchTriangleIndicesCount);
            }
        }
    }

    void Geometry::calculateTorusWireframeIndices(unsigned int circlesCount, unsigned int pointsPerCircleCount, unsigned int* indices)
    {
        const unsigned int torusVerticesCount = circlesCount * pointsPerCircleCount;

        for (unsigned int i = 0; i < circlesCount; ++i)
        {
            for (unsigned int j = 0; j < pointsPerCircleCount; ++j)
            {
                /* Starting point for vertical and horizontal lines. */
                unsigned int lineStart     = i * pointsPerCircleCount + j;
                /* Horiznotal end of the currently determined line. */
                unsigned int horizontalEnd = (i + 1) * pointsPerCircleCount + j;
                /* Vertical end of the currently determined line. */
                unsigned int verticalEnd   = i * pointsPerCircleCount + j + 1;

                /* From the last circle, horizontal lines go to the first one. */
                if (horizontalEnd >= torusVerticesCount)
                {
                    horizontalEnd -= torusVerticesCount;
                }

                /* From the last point in the circle, vertical lines go to the first one. */
                if (verticalEnd >= (i + 1) * pointsPerCircleCount)
                {
                    verticalEnd -= pointsPerCircleCount;
                }

                /* Determine horizontal line indices. */
                indices[(i * pointsPerCircleCount + j) * 4    ] = lineStart;
                indices[(i * pointsPerCircleCount + j) * 4 + 1] = horizontalEnd;

                /* Determine vertical line indices. */
                indices[(i * pointsPerCircleCount + j) * 4 + 2] = lineStart;
                indices[(i * pointsPerCircleCount + j) * 4 + 3] = verticalEnd;
            }
        }
    }

    void Geometry::generateTorusVertices(float torusRadius, float circleRadius, unsigned int circlesCount, unsigned int pointsPerCircleCount, float* torusVertices)
    {
        if (torusVertices == NULL)
        {
            LOGE("Cannot use null pointer while calculating torus vertices.");

            return;
        }

        const float pi = 3.1415926535f;

        /* Index variable. */
        unsigned int componentIndex = 0;

        for (unsigned int horizontalIndex = 0; horizontalIndex < circlesCount; ++horizontalIndex)
        {
            /* Angle in radians on XZ plane. */
            float phi = (float) horizontalIndex * 2.0f * pi / circlesCount;

            for (unsigned int verticalIndex = 0; verticalIndex < pointsPerCircleCount; ++verticalIndex)
            {
                /* Angle in radians on XY plane. */
                float theta  = (float) verticalIndex * 2.0f * pi / pointsPerCircleCount;

                /* X coordinate. */
                torusVertices[componentIndex++] = (torusRadius + circleRadius * cosf(theta)) * cosf(phi);
                /* Y coordinate. */
                torusVertices[componentIndex++] = circleRadius * sinf(theta);
                /* Z coordinate. */
                torusVertices[componentIndex++] = (torusRadius + circleRadius * cosf(theta)) * sinf(phi);
                /* W coordinate. */
                torusVertices[componentIndex++] = 1.0f;
            }
        }
    }

    void Geometry::generateBezierTorusVertices(float torusRadius, float circleRadius, float* torusVertices)
    {
        if (torusVertices == NULL)
        {
            LOGE("Cannot use null pointer while calculating torus vertices.");

            return;
        }

        /* Pi approximation. */
        const float pi = 3.1415926535f;
        /* Coefficient relating radius of a circle to the distance between middle patch control point and the closest edge point. */
        const float kappa = 4.0f * (sqrtf(2.0f) - 1.0f) / 3.0f;
        /* Angle between circle radius connecting a patch edge point and a line segment connecting the circle center and a middle patch control point. */
        const float alpha = atanf(kappa);
        /* Length of a line segment connecting circle center and a middle control point. */
        const float distortedCircleRadius = circleRadius * sqrt(1.0f + kappa * kappa);
        /* Length of a line segment connecting torus center and a middle control poin. */
        const float distortedTorusRadius = torusRadius  * sqrt(1.0f + kappa * kappa);
        /* Each circle is divided into 4 quadrants to simplify calculations. */
        const int quadrantsCount = 4;
        /* Number of circles in torus model. */
        const int circlesCount = 12;
        /* Number of points in one circle. */
        const int pointsPerCircleCount = 12;

        /* Angle in horizontal plane XZ, used only to point on edge points. */
        float phi = 0.0f;
        /* Angle in vertical plane XY, used only to point on edge points. */
        float theta = 0.0f;

        /* Index of currently calculated component. */
        unsigned int componentIndex = 0;

        /* Iterate through all circles. */
        for (int horizontalIndex = 0; horizontalIndex < circlesCount; ++horizontalIndex)
        {
            /* Index of a circle in a torus quadrant. */
            const int currentCircleModulo = horizontalIndex % (quadrantsCount - 1);

            /* Temporary variables holding current values of radii and angles. */
            float currentTorusRadius;
            float currentCircleRadius;
            float currentPhi;
            float currentTheta;

            switch (currentCircleModulo)
            {
                case 0:
                    /* Edge points take non-distorted parameters. */
                    currentTorusRadius = torusRadius;
                    currentPhi         = phi;
                    break;
                case 1:
                    /* 1st middle point. Angle value is related to the angle of preceding edge point. */
                    currentTorusRadius = distortedTorusRadius;
                    currentPhi         = phi + alpha;
                    break;
                case 2:
                    /* Second middle point. Angle value is related to the angle of the following edge point. */
                    phi                = (float) (horizontalIndex + 1) * pi / (2 * (quadrantsCount - 1));
                    currentTorusRadius = distortedTorusRadius;
                    currentPhi         = phi - alpha;
                    break;
            }

            for (int verticalIndex = 0; verticalIndex < pointsPerCircleCount; ++verticalIndex)
            {
                /* Index of a point in a circle quadrant. */
                const int currentPointModulo = verticalIndex   % (quadrantsCount - 1);

                switch (currentPointModulo)
                {
                    case 0:
                        /* Edge points take non-distorted parameters. */
                        currentCircleRadius = circleRadius;
                        currentTheta        = theta;
                        break;
                    case 1:
                        /* 1st middle point. Angle value is related to the angle of preceding edge point. */
                        currentCircleRadius = distortedCircleRadius;
                        currentTheta        = theta + alpha;
                        break;
                    case 2:
                        /* Second middle point. Angle value is related to the angle of the following edge point. */
                        theta               = (float) (verticalIndex + 1) * pi / (2 * (quadrantsCount - 1));
                        currentCircleRadius = distortedCircleRadius;
                        currentTheta        = theta - alpha;
                }

                /* Store values in the array. */
                torusVertices[componentIndex++] = (currentTorusRadius + currentCircleRadius * cosf(currentTheta)) * cosf(currentPhi);
                torusVertices[componentIndex++] =  currentCircleRadius * sinf(currentTheta);
                torusVertices[componentIndex++] = (currentTorusRadius + currentCircleRadius * cosf(currentTheta)) * sinf(currentPhi);
                torusVertices[componentIndex++] = 1.0f;
            }
        }
    }

    /* See header for specification. */
    void Geometry::getSpherePointRepresentation(const float radius, const int numberOfSamples, int* numberOfCoordinates, float** sphereCoordinates)
    {
       /* Sphere vertices are created according to rule: 
        * Create a circle at north pole of the sphere, consisting of numberOfSample points.
        * Create following circles, ending at south pole of sphere. The sphere now consists of numberOfSample circles.
        * Theta value (indicating longitude) runs from 0 to 2*pi.
        * Radius value (indicating latitude) runs from -radius to radius. 
        */

        /* Check if parameters have compatibile values. */
        if (radius <= 0.0f)
        {
            LOGE("radius value has to be greater than zero.");

            return;
        }

        if (numberOfSamples <= 0)
        {
            LOGE("numberOfSamples value has to be greater than zero.");

            return;
        }
        if (sphereCoordinates == NULL)
        {
            LOGE("Cannot use null pointer while calculating coordinates.");

            return;
        }

        /* Pi approximation. */
        const float pi = 3.1415926535f;
        /* Maximum value of an error (used to compare float values). */
        const float epsilon = 0.001f;
        /* Index of an array we will put new point coordinates at. */
        int indexOfSphereArray = 0;
        /* Maximum longitude. */
        float maxTheta = (2.0f * pi);
        /* Value of longitude step. */
        float thetaStep = maxTheta / float(numberOfSamples);
        /* Value of latitude step. */ 
        float radiusStep = (2 * radius) / float(numberOfSamples-1);
        /* Index of longitude loop. */
        int thetaIndex = 0;
        /* Index of latitude loop. */
        int radiusIndex = 0;
        /* Number of coordinates which a sphere consists of. Each point (which a sphere consists of) consists of 3 coordinates: x, y, z. */
        const int numberOfSphereCoordinates = numberOfSamples * numberOfSamples * 3;

        /* Allocate memory for result array. */
        *sphereCoordinates = (float*) malloc (numberOfSphereCoordinates * sizeof(float));

        if (*sphereCoordinates == NULL)
        {
           LOGE("Could not allocate memory for result array.");

            return;
        }

        /* Loop through circles from north to south. */
        for (float r = -radius; 
                   r < radius + epsilon; 
                   r = -radius + radiusIndex * radiusStep)
        {
            thetaIndex = 0;

            /* Protect against rounding errors.. */
            if (r > radius)
            {
                r = radius;
            }

            /* Loop through all points of the circle. */
            for (float theta = 0.0f; 
                       theta < maxTheta; 
                       theta = thetaIndex * thetaStep)
            {
                /* Compute x, y and z coordinates for the considered point. */
                float x = sqrt((radius * radius) - (r * r)) * cosf(theta);
                float y = sqrt((radius * radius) - (r * r)) * sinf(theta);
                float z = r;

                (*sphereCoordinates)[indexOfSphereArray] = x;
                indexOfSphereArray++;

                (*sphereCoordinates)[indexOfSphereArray] = y;
                indexOfSphereArray++;

                (*sphereCoordinates)[indexOfSphereArray] = z;
                indexOfSphereArray++;

                thetaIndex ++;
            }

            radiusIndex ++;
        }

        if (numberOfCoordinates != NULL)
        {
            *numberOfCoordinates = numberOfSphereCoordinates;
        }
    }

    /* See header for specification. */
    void Geometry::getSphereTriangleRepresentation(const float radius, const int numberOfSamples, int* numberOfCoordinates, float** sphereTrianglesCoordinates)
    {
        /* Check if parameters have compatibile values. */
        if (radius <= 0.0f)
        {
            LOGE("radius value has to be greater than zero.");

            return;
        }

        if (numberOfSamples <= 0)
        {
            LOGE("numberOfSamples value has to be greater than zero.");

            return;
        }
        if (sphereTrianglesCoordinates == NULL)
        {
            LOGE("Cannot use null pointer while calculating coordinates.");

            return;
        }
        /* Array holding coordinates of points which a sphere consists of. */
        float* sphereCoordinates = NULL;
        /* Current index used for accessing sphereTrianglesCoordinates array. */
        int sphereTriangleCoordinateIndex = 0;
        /* Index of current loop iteration. */
        int iterationIndex = 1;
        /* There are 18 coordinates created in one loop: 3 coordinates * 3 triangle vertices * 2 triangles. */
        int numberOfCoordinatesCreatedInOneLoop = 18;
        /* Number of coordinates of all triangles which a sphere consists of.
        * (numberOfSamples - 1) - for each circle (excluding last one)
        * numberOfSamples - for each point on a single circle
        * 2 - number of triangles that start at given point
        * 3 - number of points that make up a triangle
        * 3 - coordinates per each point.
        */
        const int numberOfSphereTriangleCoordinates = (numberOfSamples - 1) * numberOfSamples * 2 * 3 * 3;

        /* Compute coordinates of points which make up a sphere. */
        Geometry::getSpherePointRepresentation(radius, numberOfSamples, NULL, &sphereCoordinates);

        if (sphereCoordinates == NULL)
        {
            LOGE("Could not get coordinates of points which make up a sphere.");

            return;
        }

        *sphereTrianglesCoordinates = (float*) malloc (numberOfSphereTriangleCoordinates * sizeof(float));

        if (*sphereTrianglesCoordinates == NULL)
        {
            LOGE("Could not allocate memory for result array.");

            return;
        }

        /* Each point has 3 coordinates: x, y, z. */
        for (int pointIndex = 0; 
                 pointIndex < (numberOfSphereTriangleCoordinates / numberOfCoordinatesCreatedInOneLoop) * 3; 
                 pointIndex += 3)
        {
            /* First triangle. ==> */

            /* Building of a triangle is started from point at index described by pointIndex. 
             * Values of x, y and z coordinates are written one after another in sphereCoordinates array starting at index of point value. 
             */
            /* Coordinate x. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex];
            sphereTriangleCoordinateIndex++;
            /* Coordinate y. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 1];
            sphereTriangleCoordinateIndex++;
            /* Coordinate z. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 2];
            sphereTriangleCoordinateIndex++;

            /* Check if this is the last point of circle. */
            if ((iterationIndex % numberOfSamples) == 0 )
            {
                /* Second point of triangle.
                 * If this is the last point lying on a circle,
                 * second point that makes up a triangle is the point with coordinates taken from the first point lying on current circle.
                 * According to example above: the second point for first triangle starting at point D1 (last point on a current circle) is point A1.
                 * Index (to receive x coordinate): [point - 3 * (numberOfSamples)] stands for x coordinate of point D0 (corresponding point from previous circle), 
                 * [+ 3] stands for next point (first point of current circle - A1).  
                 */
                /* Coordinate x. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex - 3 * (numberOfSamples) + 3];
                sphereTriangleCoordinateIndex++;                                                       
                /* Coordinate y. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex - 3 * (numberOfSamples) + 3 + 1];
                sphereTriangleCoordinateIndex++;                                                       
                /* Coordinate z. */                                     
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex - 3 * (numberOfSamples) + 3 + 2];
                sphereTriangleCoordinateIndex++;

                /* Third point of triangle.
                 * If this is the last point lying on a circle,
                 * third point that makes up a triangle is the point with coordinates taken from the first point lying on next circle.
                 * According to example above: the third point for first triangle starting at point D1 (last point on a current circle) is point A2.
                 * Index (to receive x coordinate): [point + 3] stands for x coordinate of point A2 (first point on next circle).  
                 */
                /* Coordinate x. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3];
                sphereTriangleCoordinateIndex++;                                                          
                /* Coordinate y. */                                     
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 + 1];
                sphereTriangleCoordinateIndex++;                                                          
                /* Coordinate z. */                                     
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 + 2];
                sphereTriangleCoordinateIndex++;
            }
            else
            {
                /* Second point of triangle.
                 * If this is not the last point lying on a circle,
                 * second point that makes up a triangle is the point with coordinates taken from the next point lying on current circle. 
                 * According to example above: the second point for first triangle starting at (for example) point A1 (not last point on a circle) is point B1.
                 * Index (to receive x coordinate): [point + 3] stands for x coordinate of point B1 (next point in array).  
                 */
                /* Coordinate x. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3];
                sphereTriangleCoordinateIndex++;
                /* Coordinate y. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 + 1];
                sphereTriangleCoordinateIndex++;
                /* Coordinate z. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 + 2];
                sphereTriangleCoordinateIndex++;

                /* Third point of triangle.
                 * If this is not the last point lying on a circle,
                 * third point that makes up a triangle is the point with coordinates taken from the next from corresponding point lying on next circle.
                 * According to example above: the third point for first triangle starting at (for example) point A1 (not last point on a circle) is point B2.
                 * Index (to receive x coordinate): [point + numberOfSamples * 3] stands for x coordinate of point A2 (corresponding point lying on next circle), 
                 * [+ 3] stands for nex point on that circle - B2.  
                 */
                /* Coordinate x. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + numberOfSamples * 3 + 3];
                sphereTriangleCoordinateIndex++;                                                  
                /* Coordinate y. */                                     
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + numberOfSamples * 3 + 3 + 1];
                sphereTriangleCoordinateIndex++;                                                  
                /* Coordinate z. */                                     
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + numberOfSamples * 3 + 3 + 2];
                sphereTriangleCoordinateIndex++;
            }
            /* <== First triangle. */

            /* Second triangle. ==> */

            /* Building of a triangle is started from point at index described by pointIndex. 
             * Values of x, y and z coordinates are written one after another in sphereCoordinates array starting at index of point value. 
             */
            /* Coordinate x. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex];
            sphereTriangleCoordinateIndex++;
            /* Coordinate y. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 1];
            sphereTriangleCoordinateIndex++;
            /* Coordinate z. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 2];
            sphereTriangleCoordinateIndex++;

            /* Check if this is a last point of circle. */
            if ((iterationIndex % numberOfSamples) == 0 )
            {   /* Second point of triangle.
                 * If this is the last point lying on a circle,
                 * second point that makes up a triangle is the point with coordinates taken from the first point lying on next circle.
                 * According to example above: the second point for first triangle starting at point D1 (last point on a current circle) is point A2.
                 * Index (to receive x coordinate): [point + 3] stands for x coordinate of point A2 (first point on next circle).  
                 */
                /* Coordinate x. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3];
                sphereTriangleCoordinateIndex++;
                /* Coordinate y. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 + 1];
                sphereTriangleCoordinateIndex++;
                /* Coordinate z. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 + 2];
                sphereTriangleCoordinateIndex++;
            }
            else
            {
                /* Second point of triangle.
                 * If this is not the last point lying on a circle,
                 * second point that makes up a triangle is the point with coordinates taken from the next from corresponding point lying on next circle.
                 * According to example above: the second point for first triangle starting at (for example) point A1 (not last point on a circle) is point B2.
                 * Index (to receive x coordinate): [point + numberOfSamples * 3] stands for x coordinate of point A2 (corresponding point lying on next circle), 
                 * [+ 3] stands for nex point on that circle - B2.  
                 */
                /* Coordinate x. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 * numberOfSamples + 3];
                sphereTriangleCoordinateIndex++;
                /* Coordinate y. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 * numberOfSamples + 3 + 1];
                sphereTriangleCoordinateIndex++;
                /* Coordinate z. */
                (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 * numberOfSamples + 3 + 2];
                sphereTriangleCoordinateIndex++;
            }

            /* Third point of triangle that makes up a triangle is the point with coordinates taken from the corresponding point lying on next circle.
             * According to example above: the third point for second triangle starting at point A1 (for example) is point A2.
             * Index (to receive x coordinate): [point + 3 * (numberOfSamples)] stands for x coordinate of point A2 (corresponding point from next circle).  
             */
            /* Coordinate x. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 * numberOfSamples];
            sphereTriangleCoordinateIndex++;
            /* Coordinate y. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3  *numberOfSamples + 1];
            sphereTriangleCoordinateIndex++;
            /* Coordinate z. */
            (*sphereTrianglesCoordinates)[sphereTriangleCoordinateIndex] = sphereCoordinates[pointIndex + 3 * numberOfSamples + 2];
            sphereTriangleCoordinateIndex++;

            /* <== Second triangle. */

            iterationIndex++;
        }

        if (numberOfCoordinates != NULL)
        {
            *numberOfCoordinates = numberOfSphereTriangleCoordinates;
        }

        /* Deallocate memory. */
        free(sphereCoordinates);
        sphereCoordinates = NULL;
    }

    /* See header for specification. */
    void Geometry::getCubeTriangleRepresentation(float scalingFactor, int* numberOfCoordinates, float** cubeTrianglesCoordinates)
    {
        if (cubeTrianglesCoordinates == NULL)
        {
            LOGE("Cannot use null pointer while calculating coordinates.");

            return;
        }

        /* 6 faces, 2 triangles for each face, 3 points of triangle, 3 coordinates for each point. */
        const int numberOfCubeTriangleCoordinates = 6 * 2 * 3 * 3;
        /* Index of an array we will put new point coordinates at. */
        int currentIndex = 0;

        /* Allocate memory for result array. */
        *cubeTrianglesCoordinates = (float*) malloc (numberOfCubeTriangleCoordinates * sizeof(float));

        /* Is allocation successfu?. */
        if (*cubeTrianglesCoordinates == NULL)
        {
            LOGE("Could not allocate memory for result array.");

            return;
        }

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

        /* Fill the array with coordinates. */
        /* Top face. */
        /*A*/ 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*B*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*C*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;

        /*A*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*C*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*D*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;

        /* Bottom face. */
        /*E*/                              
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*F*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*G*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;

        /*E*/                              
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*G*/ 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*H*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;

        /* Back face. */
        /*G*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*C*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*B*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;

        /*G*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*B*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /*F*/                            
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;

        /* Front face. */
        /*E*/                            
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*A*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*D*/
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f; 
        currentIndex++;

        /*E*/                            
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*D*/
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f; 
        currentIndex++;
        /*H*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;

        /* Right face. */
        /*H*/                             
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /*D*/
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = 1.0f; 
        currentIndex++;
        /*C*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;

        /*H*/                              
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f;
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;
        /*C*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        /*G*/
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;

        /* Left face. */
        /*F*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;  
        currentIndex++;
        /*B*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;
        currentIndex++;  
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        /*A*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;

        /*F*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f;  
        currentIndex++;
        /*A*/
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++; 
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;
        /*E*/                            
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] = -1.0f; 
        currentIndex++;
        (*cubeTrianglesCoordinates)[currentIndex] =  1.0f; 
        currentIndex++;

        /* Calculate size of a cube. */
        for (int i = 0; i < numberOfCubeTriangleCoordinates; i++)
        {
            (*cubeTrianglesCoordinates)[i] = scalingFactor * (*cubeTrianglesCoordinates)[i];
        }
        
        if (numberOfCoordinates != NULL)
        {
            *numberOfCoordinates = numberOfCubeTriangleCoordinates;
        }
    }

    /* See header for specification. */
    void Geometry::getCubeNormals(int* numberOfCoordinates, float** cubeNormals)
    {
        /* Set the same normals for both triangles from each face.
         * For details: see example for getCubeTriangleRepresentation() function. 
         */

        if (cubeNormals == NULL)
        {
            LOGE("Cannot use null pointer while calculating coordinates.");

            return;
        }

        /* 6 faces, 2 triangles for each face, 3 points of triangle, 3 coordinates for each point. */
        const int numberOfCubeNormalsCoordinates = 6 * 2 * 3 * 3;
        /* Index of an array we will put new point coordinates at. */
        int currentIndex = 0;

        /* Allocate memory for result array. */
        *cubeNormals = (float*) malloc (numberOfCubeNormalsCoordinates * sizeof(float));

        /* Is allocation successfu?. */
        if (*cubeNormals == NULL)
        {
            LOGE("Could not allocate memory for result array.");

            return;
        }

        /* There are 2 triangles for each face. Each triangle consists of 3 vertices. */
        int numberOfCoordinatesForOneFace = 2 * 3;

        /* Top face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 1;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
        }

        /* Bottom face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            (*cubeNormals)[currentIndex] =  0;
            currentIndex++;
            (*cubeNormals)[currentIndex] = -1;
            currentIndex++;
            (*cubeNormals)[currentIndex] =  0;
            currentIndex++;
        }

        /* Back face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            (*cubeNormals)[currentIndex] =  0;
            currentIndex++;
            (*cubeNormals)[currentIndex] =  0;
            currentIndex++;
            (*cubeNormals)[currentIndex] =  -1;
            currentIndex++;
        }

        /* Front face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 1;
            currentIndex++;
        }

        /* Right face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            (*cubeNormals)[currentIndex] = 1;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
        }

        /* Left face. */
        for (int i = 0; i < numberOfCoordinatesForOneFace; i++)
        {
            (*cubeNormals)[currentIndex] = -1;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
            (*cubeNormals)[currentIndex] = 0;
            currentIndex++;
        }

        if (numberOfCoordinates != NULL)
        {
            *numberOfCoordinates = numberOfCubeNormalsCoordinates;
        }
    }

    /* See header for specification. */
    void Geometry::getSquareTriangleRepresentationInXZSpace(float scalingFactor, int* numberOfCoordinates, float** squareCoordinates)
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

        if (squareCoordinates == NULL)
        {
            LOGE("Cannot use null pointer while calculating coordinates.");

            return;
        }

        /* 2 triangles, 3 points of triangle, 3 coordinates for each point. */
        const int numberOfSquareCoordinates = 2 * 3 * 3;

        /* Allocate memory for result array. */
        *squareCoordinates = (float*) malloc (numberOfSquareCoordinates * sizeof(float));

        /* Is allocation successfu?. */
        if (*squareCoordinates == NULL)
        {
            LOGE("Could not allocate memory for result array.");

            return;
        }
        
        /* Index of an array we will put new point coordinates at. */
        int currentIndex = 0;

        /* First triangle. */
        /* A */
        /* x */ 
        (*squareCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /* y */ 
        (*squareCoordinates)[currentIndex] =  0.0f;
        currentIndex++;
        /* z */ 
        (*squareCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /* B */
        /* x */ 
        (*squareCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /* y */ 
        (*squareCoordinates)[currentIndex] =  0.0f;
        currentIndex++;
        /* z */ 
        (*squareCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /* C */
        /* x */ 
        (*squareCoordinates)[currentIndex] =  1.0f;
        currentIndex++;
        /* y */ 
        (*squareCoordinates)[currentIndex] =  0.0f;
        currentIndex++;
        /* z */ 
        (*squareCoordinates)[currentIndex] =  1.0f;
        currentIndex++;

        /* Second triangle. */
        /* A */
        /* x */
        (*squareCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        /* y */
        (*squareCoordinates)[currentIndex] =  0.0f;
        currentIndex++; 
        /* z */
        (*squareCoordinates)[currentIndex] = -1.0f;
        currentIndex++;
        /* C */
        /* x */
        (*squareCoordinates)[currentIndex] = 1.0f;
        currentIndex++; 
        /* y */
        (*squareCoordinates)[currentIndex] = 0.0f;
        currentIndex++; 
        /* z */
        (*squareCoordinates)[currentIndex] = 1.0f;
        currentIndex++;
        /* D */
        /* x */
        (*squareCoordinates)[currentIndex] = -1.0f;
        currentIndex++; 
        /* y */
        (*squareCoordinates)[currentIndex] =  0.0f;
        currentIndex++; 
        /* z */
        (*squareCoordinates)[currentIndex] =  1.0f;
        currentIndex++;

        /* Calculate size of a square. */
        for (int i = 0; i < numberOfSquareCoordinates; i++)
        {
            (*squareCoordinates)[i] = scalingFactor * (*squareCoordinates)[i];
        }

        if (numberOfCoordinates != NULL)
        {
            *numberOfCoordinates = numberOfSquareCoordinates;
        }
    }

    /* See header for specification. */
    void Geometry::getSquareXZNormals(int* numberOfCoordinates, float** squareNormals)
    {
        if (squareNormals == NULL)
        {
            LOGE("Cannot use null pointer while calculating coordinates.");

            return;
        }

        /* 3 coordinates for square's normal. */
        const int numberOfNormalsCoordinates = 3;

        /* Allocate memory for result array. */
        *squareNormals = (float*) malloc (numberOfNormalsCoordinates * sizeof(float));

        /* Is allocation successfu?. */
        if (*squareNormals == NULL)
        {
            LOGE("Could not allocate memory for result array.");

            return;
        }

        (*squareNormals)[0] = 0.0f;
        (*squareNormals)[1] = 1.0f;
        (*squareNormals)[2] = 0.0f;

        if (numberOfCoordinates != NULL)
        {
            *numberOfCoordinates = numberOfNormalsCoordinates;
        }
    }
}