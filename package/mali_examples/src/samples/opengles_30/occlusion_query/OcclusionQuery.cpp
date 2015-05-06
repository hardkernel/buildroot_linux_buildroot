/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * \file OcclusionQuery.cpp
 * Demonstration of Occlusion Query functionality in OpenGL ES 3.0.
 * A scene is drawn show a number of complex cubes. Without occlusion queries, 
 * all the cubes a rendered even if they are occluded by other cubes.
 * To increase performance, we can test whether the cubes are actually
 * visible in the scene using occlusion queries and then only draw the visible ones.
 */

#include "OcclusionQuery.h"

#include "EGLRuntime.h"
#include "ETCHeader.h"
#include "PlaneModel.h"
#include "CubeModel.h"
#include "SuperEllipsoidModel.h"
#include "Mathematics.h"
#include "Matrix.h"
#include "Platform.h"
#include "Shader.h"
#include "Timer.h"
#include "VectorTypes.h"
#include "Text.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <string>
#include <ctime>

using std::string;
using namespace MaliSDK;

/* Asset directory. */
string resourceDirectory  = "assets/";

/* Window properties. */
const int windowWidth  = 800;
const int windowHeight = 600;

/* Timer */
Timer timer;

/* Interval expressed in seconds in which we change between modes. */
const float intervalInSeconds = 10.0f;

/* Id of OpenGL program we use for rendering. */
GLuint programId = 0;

/* Super ellipsoid */
/* Determines number of cubes that are going to be rendered per frame. */
const int numberOfCubes = 50;

/* Determines accuracy of rounded cubes - number of sample triangles that will make up a super ellipsoid. */
const int numberOfSamples = 256;

/* 
 * These two "squareness" parameters determine what kind of figure we will get.
 * Different values can create for example a sphere, rounded cube, something like a star, cyllinder, etc.
 * These given values (0.3f and 0.3f) will create a rounded cube.
 */
const float squareness1	= 0.3f;
const float squareness2	= 0.3f;

/*
 * These variables are used to scale up cubes (normal and rounded). 
 * normalCubeScaleFactor has to be smaller than roundedCubeScaleFactor
 * to avoid blinking effect (some cubes disappear some appear).
 */
const float roundedCubeScaleFactor = 2.5f;
const float normalCubeScaleFactor = 2.49f;

/* Pointer to an array that stores vertices of rounded cube. */
float* roundedCubeCoordinates = NULL; 

/* Pointer to an array that stores normal vectors of rounded cube. */
float* roundedCubeNormalVectors = NULL;

/* This value represents number of rounded cube vertices. */
int numberOfRoundedCubesVertices = 0;

/* Represents a number of rounded cube's coordinates. */
int numberOfRoundedCubeCoordinates = 0;

/* Represents a number of rounded cube's normal vectors. */
int numberOfRoundedCubeNormalVectors = 0;

/* Array that stores random position of each cube. */
Vec2f randomCubesPositions[numberOfCubes];

/* Array that stores flags indicating which cubes were visible in previous frame. */
bool previouslyVisible[numberOfCubes] = { false };

/* Minimum distance between cubes. */
const float minimumDistance = 4.0f;

/* Array of queries for each of numberOfCubes cubes. */
GLuint cubeQuery[numberOfCubes] = { 0 };

/* Flag that informs us what mode is turned on (if it's occlusion query mode or not). */
bool occlusionQueriesOn = false;

/* This is the angle that is used to rotate camera around Y axis. */
float angleY = 0.0f;

/* This value informs us how far camera is located from point (0, 0, 0). */
const float cameraRadius = 22.0f;

/* This value is used to translate camera along the Y axis. */
const float yCameraTranslation = 1.25f;

/* This matrix will be used to calculate camera view. */
Matrix viewMatrix;

/* This matrix will be used to store yRotationMatrix * viewMatrix. */
Matrix rotatedViewMatrix;

/* 
 * This uniform informs shader if we want to draw a plane or a cube. 
 * If we want to draw a plane this value is 1, otherwise it's 0.
 */
GLint renderPlaneUniformLocation = 0;

/* Points out the uniform location of cube's locations on the plane (translation vectors). */
GLint cubesLocationsUniformLocation = 0;

/* Points out the location of uniform storing a view matrix. */
GLuint viewMatrixUniformLocation = 0;

/* Array to store sorted positions of the cubes. Each cube has 2 coordinates. */
float sortedCubesPositions[2 * numberOfCubes] = { 0.0f };

/* Scaling factor to scale up the plane. */
const float planeScalingFactor = 40.0f;

/* Determines how many times an area (where the cubes are located) should be smaller than original plane. */
const float planeDividend = 3.0f;

/* Vertex Array Objects for plane, normal cube and rounded cube. */
GLuint planeVertexArrayObjectId		  = 0;
GLuint normalCubeVertexArrayObjectId  = 0;
GLuint roundedCubeVertexArrayObjectId = 0;

/* This buffer object holds the plane's vertices. */
GLuint planeVerticesBufferId = 0;

/* This buffer object holds the plan's normal vectors. */
GLuint planeNormalVectorsBufferId = 0;

/* This buffer object holds the normal cubes' vertices. */
GLuint normalCubeBufferId = 0;

/* This buffer object holds the rounded cubes' vertices. */
GLuint roundedCubeVerticesBufferId = 0;

/* This buffer object holds the rounded cubes' normal vectors. */
GLuint roundedCubeNormalVectorsBufferId = 0;

/* Pointer to an array that stores the cubes' vertices. */
float* normalCubeVertices = NULL;

/* Determines the size of the dynamically allocated normalCubeVertices array. */
int sizeOfNormalCubeVerticesArray = 0;

/* Pointer to an array that stores the plane's vertices. */
float* planeVertices = NULL;

/* Determines the size of the dynamically allocated planeVertices array. */
int sizeOfPlaneVerticesArray = 0;

/* Pointer to an array that stores the plane's normal vectors. */
float* planeNormalVectors = NULL;

/* Determines the size of the dynamically allocated plane Normals array. */
int sizeOfPlaneNormalsArray = 0;

/* Flag indicating that the rendering mode has changed (occlusion query OFF => ON) */
bool modeChanged = false;

/* Counter for the number of rounded cubes drawn each frame. */
int numberOfRoundedCubesDrawn = 0;

/* Text object to indicate whether occlusion queries are turned on or not. */
Text* text;

/**
 * \brief Check if the rounded cubes are a proper distance from each other to prevent cubes overlapping. 
 * \param[in] point The point for which we check distance.
 * \param[in] minDistance Determines minimum distance between points.
 * \param[in] j Tells how many points are already saved in the randomCubesPositions array.
 * \return True if the point is in neighbourhood of any other point that is already saved in randomCubesPotisitions array and false otherwise.
 */
inline bool inNeighbourhood(const Vec2f& point, float minDistance, int j)
{
	for (int i = 0; i < j; i++)
	{
		if (distanceBetweenPoints(point, randomCubesPositions[i]) < minDistance)
		{
			return true;
		}
	}
	return false;
}

/**
 * \brief Generate random cubes' center locations. 
 * 
 * This algorithm ensures that cube will be the required distance apart. 
 *
 * \param[in] planeWidth  Width of the plane.
 * \param[in] planeHeight Height of the plane.
 * \param[in] minDistance Determines minimum distance between cubes.
 */
void generateCubesLocations(float planeWidth, float planeHeight, float minDistance)
{	
	/*
     * xRange and zRange are both minimum (-xRange, -zRange) and maximum (+xRange, +zRange) values respectively for X axis and Z axes.
	 * These two values ensure that cubes will not partially land outside the plane. To prevent such a situation we have to
	 * restrict the bounds of our plane. We also want camera to fly around the cubes and
	 * have the plane still visible (we don't want to see the edges of the plane - "end of the world").
	 * That's why we divide planeWidth and planeHeight by planeDividend - this will give us a smaller plane
	 * that a part of the bigger, original plane. 
	 */
	float xRange = planeWidth / planeDividend;
	float zRange = planeHeight / planeDividend;

	/* 
     * This variable is used to prevent the situation that this function will iterate for
	 * infinite number of times to find locations for a cube - we can't locate infinite number of cubes
     * that don't overlap on a finite plane.
	 */
	float loopsCounter = 0;

	/*
     * We generate the first point which can be randomly chosen from points on the plane
	 * and we save it to the array.
	 * Generation of this point is based on equation for generating random numbers
	 * in given interval. If we want to get a random number in an interval [a, b]
	 * we can use equation: rand()%(b - a) + a, where rand() generates a number
	 * between 0 and 1.
	 */
	Vec2f firstRandomPoint = {(xRange + xRange) * uniformRandomNumber() - xRange,
							  (zRange + zRange) * uniformRandomNumber() - zRange};
	
	randomCubesPositions[0] = firstRandomPoint;
	
	for (int i = 1; i < numberOfCubes; i++)
	{
		/* Check if the loop has not iterated too many times. */
		if (loopsCounter > numberOfCubes * numberOfCubes)
		{
			return;
		}

		/* We choose another random point on a plane. */
		Vec2f randomPoint = {(xRange + xRange) * MaliSDK::uniformRandomNumber() - xRange,
							 (zRange + zRange) * MaliSDK::uniformRandomNumber() - zRange};

		/* And check if it's a proper distance from any other point that is already stored in the array. */
		if (!inNeighbourhood(randomPoint, minDistance, i))
		{
			/* If it is, we can save it to the array of random cubes positions. */
            randomCubesPositions[i] = randomPoint;
		}
		else
		{
			/* Otherwise, decrement the i counter and try again. */
			i -= 1;
		}

		/* Increase loop counter. */
		loopsCounter++;
	}
}

/**
 * \brief Function that is used to sort cubes' center positons from the nearest to the furthest, relative to the camera position.
 *
 * We use bubble sort algorithm that checks whether an array is sorted or not.
 *
 * \param[in,out] arrayToSort An array to be sorted.
 */
void sortCubePositions(float* arrayToSort)
{
    /*
     * The upper limit position of sorted elements. We substract 4 because we will refer to the 4 array's 
     * cells ahead and we don't want to go out of bounds of the array.
     */
    int max = (numberOfCubes * 2) - 4;

    bool swapped = true;

    while(swapped)
    {
        swapped = false;

        for (int i = 0; i <= max; i += 2)
        {
            /* Temporarily store location points. */
            Vec2f firstCubeLocation, secondCubeLocation;

            firstCubeLocation.x = arrayToSort[i];
            firstCubeLocation.y = arrayToSort[i + 1];

            secondCubeLocation.x = arrayToSort[i + 2];
            secondCubeLocation.y = arrayToSort[i + 3];

            Vec3f first_cube_location              = {firstCubeLocation.x,  1, firstCubeLocation.y};
            Vec3f second_cube_location             = {secondCubeLocation.x, 1, secondCubeLocation.y};
            Vec3f transformed_first_cube_location  = Matrix::vertexTransform(&first_cube_location,  &rotatedViewMatrix);
            Vec3f transformed_second_cube_location = Matrix::vertexTransform(&second_cube_location, &rotatedViewMatrix);

            if (transformed_first_cube_location.z < transformed_second_cube_location.z)
            {
                /* Swap coordinates' positions. */
                arrayToSort[i]     = secondCubeLocation.x;
                arrayToSort[i + 1] = secondCubeLocation.y;

                arrayToSort[i + 2] = firstCubeLocation.x;
                arrayToSort[i + 3] = firstCubeLocation.y;

                /* Swap the cubes visible status to match. */
                bool temp = previouslyVisible[i / 2];
                previouslyVisible[i / 2]     = previouslyVisible[i / 2 + 1];
                previouslyVisible[i / 2 + 1] = temp;

                swapped = true;
            }
        }
    }
}

/**
 * \brief convert Vec2f array to an array of floats.
 *
 * Rewrite coordinates from randomCubesPositions array which type is Vec2f
 * to a simple array of floats (sortedCubesPositions) that we can easily pass to the uniform.
 */
void rewriteVec2fArrayToFloatArray()
{
	for (int i = 0; i < numberOfCubes; i++)
	{
	    sortedCubesPositions[i * 2]     = randomCubesPositions[i].x;
	    sortedCubesPositions[i * 2 + 1] = randomCubesPositions[i].y;
	}
}

/**
 * \brief Sends center position of a cube to the vertex shader's uniform.
 * \param[in] whichCube Determines which cube's center position we should send to the vertex shader.
 */
inline void sendCubeLocationVectorToUniform(int whichCube)
{
    /* Array to be sent to the vertex shader.*/
    float tempArray[3];

    tempArray[0] = sortedCubesPositions[2 * whichCube];

    /* We send roundedCubeScaleFactor to translate cubes a little bit up so they won't intersect with the plane.*/
    tempArray[1] = roundedCubeScaleFactor;

    tempArray[2] = sortedCubesPositions[2 * whichCube + 1];

    /* Send array to the shader. */
    GL_CHECK(glUniform3fv(cubesLocationsUniformLocation, 1, tempArray));
}

/**
 * \brief Function that sets up shaders, programs, uniforms locations, generates buffer objects and query objects. 
 * \return True if everything goes fine, false otherwise.
 */
bool setupGraphics()
{	
    /* Set up the text object. */
    text = new Text(resourceDirectory.c_str(), windowWidth, windowHeight);
    text->addString(0, 0, "Occlusion query OFF", 255, 0, 0, 255);

	/* Set clear color. */
	GL_CHECK(glClearColor(0.3f, 0.6f, 0.70f, 1.0f));

    /* Enable depth test and set depth function to GL_LEQUAL. */
	GL_CHECK(glEnable   (GL_DEPTH_TEST));
	GL_CHECK(glDepthFunc(GL_LEQUAL));

    /* Enable blending (required for Text drawing). */
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	/* Set shaders up. */
	GLuint vertexShaderId = 0;
    string vertexShaderPath = resourceDirectory + "vertex.vert";
	Shader::processShader(&vertexShaderId,   vertexShaderPath.c_str(),   GL_VERTEX_SHADER);

	GLuint fragmentShaderId = 0;
    string fragmentShaderPath = resourceDirectory + "fragment.frag";
    Shader::processShader(&fragmentShaderId, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);
	
	programId = GL_CHECK(glCreateProgram());

	GL_CHECK(glAttachShader(programId, vertexShaderId));
	GL_CHECK(glAttachShader(programId, fragmentShaderId));

	GL_CHECK(glLinkProgram(programId));
	GL_CHECK(glUseProgram (programId));

	/* Set projection matrix up. */
    Matrix projectionMatrix	= Matrix::matrixPerspective(degreesToRadians(45.0f), float(windowWidth)/float(windowHeight), 0.1f, 50.0f);
	GLuint projectionMatrixLocation = GL_CHECK(glGetUniformLocation(programId, "projectionMatrix"));

	if (projectionMatrixLocation == -1)
    {
        LOGE("Could not retrieve uniform location: projectionMatrixLocation");
        return false;
    }
	else
	{
		GL_CHECK(glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix.getAsArray()));
	}

	/* Attributes, uniforms locations. */
	GLuint verticesAttributeLocation = GL_CHECK(glGetAttribLocation(programId, "vertex"));
	GLuint normalAttributeLocation	 = GL_CHECK(glGetAttribLocation(programId, "normal"));
	
	renderPlaneUniformLocation       = GL_CHECK(glGetUniformLocation(programId, "renderPlane"));
	cubesLocationsUniformLocation    = GL_CHECK(glGetUniformLocation(programId, "cubesLocations"));
	viewMatrixUniformLocation        = GL_CHECK(glGetUniformLocation(programId, "viewMatrix"));

	/* Check if all uniforms and attributes were found in the vertex shaders. */
    if (verticesAttributeLocation == -1)
    {
        LOGE("Could not retrieve attribute location: verticesAttributeLocation");
        return false;
    }

	if (normalAttributeLocation == -1)
    {
        LOGE("Could not retrieve attribute location: normalAttributeLocation");
        return false;
    }

	if (renderPlaneUniformLocation == -1)
    {
        LOGE("Could not retrieve uniform location: renderPlaneUniformLocation");
        return false;
    }

	if (viewMatrixUniformLocation == -1)
    {
        LOGE("Could not retrieve uniform location: viewMatrixUniformLocation");
        return false;
    }
    else
    {
        /* Initialize vectors that will be passed to matrixCameralookAt function. */
        Vec3f eyeVector	  = {0.0f, yCameraTranslation, cameraRadius};
	    Vec3f lookAtPoint = {0.0f, 0.0f,               0.0f};
	    Vec3f upVector    = {0.0f, 1.0f,               0.0f};

	    /* Calculate view matrix. */
	    viewMatrix = Matrix::matrixCameraLookAt(eyeVector, lookAtPoint, upVector);
    }

	if (cubesLocationsUniformLocation == -1)
    {
        LOGE("Could not retrieve uniform location: cubesLocationsUniformLocation");
        return false;
    }

	/* Generate super ellipsoid. */
    SuperEllipsoidModel::create(numberOfSamples, 
                                squareness1, 
                                squareness2, 
                                roundedCubeScaleFactor, 
                                &roundedCubeCoordinates, 
                                &roundedCubeNormalVectors, 
                                &numberOfRoundedCubesVertices, 
                                &numberOfRoundedCubeCoordinates,
                                &numberOfRoundedCubeNormalVectors);

    /* Make sure super ellipsoid was created successfully. */
    if (roundedCubeCoordinates == NULL)
    {
        LOGE("Could not create super ellipsoid's coordinates.");
        return false;
    }

    if (roundedCubeNormalVectors == NULL)
    {
        LOGE("Could not create super ellipsoid's normal vectors.");
        return false;
    }

    /* Generate cubes' center locations. */
	generateCubesLocations(planeScalingFactor, planeScalingFactor, minimumDistance);	

    /* Rewrite Vec2f randomCubesPosition array to simple array of floats. */
    rewriteVec2fArrayToFloatArray();

    /* Generate triangular representation of a cube. */
    CubeModel::getTriangleRepresentation(normalCubeScaleFactor, &sizeOfNormalCubeVerticesArray, &normalCubeVertices); 

    /* Make sure triangular representation of the cube was created successfully. */
    if (normalCubeVertices == NULL)
    {
        LOGE("Could not create triangular representation of a cube.");
        return false;
    }

    /* Generate triangular representation of a plane. */
    PlaneModel::getTriangleRepresentation(&sizeOfPlaneVerticesArray, &planeVertices);

    /* Scale the plane up to fill the screen. */
    Matrix scaling = Matrix::createScaling(planeScalingFactor, planeScalingFactor, planeScalingFactor);
    PlaneModel::transform(scaling, sizeOfPlaneVerticesArray, &planeVertices);

    /* Make sure triangular representation of the plane was created successfully. */
    if (planeVertices == NULL)
    {
        LOGE("Could not create triangular representation of a plane.");
        return false;
    }

    /* Generate plane's normal vectors. */
    PlaneModel::getNormals(&sizeOfPlaneNormalsArray, &planeNormalVectors);

    /* Make sure plane's normal vectors were created successfully. */
    if (planeVertices == NULL)
    {
        LOGE("Could not create plane's normal vectors.");
        return false;
    }

	/* Generate buffer objects. */
	GL_CHECK(glGenBuffers(1, &planeVerticesBufferId));
	GL_CHECK(glGenBuffers(1, &planeNormalVectorsBufferId));
	GL_CHECK(glGenBuffers(1, &normalCubeBufferId));
	GL_CHECK(glGenBuffers(1, &roundedCubeVerticesBufferId));
	GL_CHECK(glGenBuffers(1, &roundedCubeNormalVectorsBufferId));

	/* Generate vertex array objects. */
	GL_CHECK(glGenVertexArrays(1, &planeVertexArrayObjectId));
	GL_CHECK(glGenVertexArrays(1, &normalCubeVertexArrayObjectId));
	GL_CHECK(glGenVertexArrays(1, &roundedCubeVertexArrayObjectId));

	/* This vertex array object stores the plane's vertices and normal vectors. */
	GL_CHECK(glBindVertexArray(planeVertexArrayObjectId));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, planeVerticesBufferId));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeOfPlaneVerticesArray * sizeof(float), planeVertices, GL_STATIC_DRAW));
	GL_CHECK(glVertexAttribPointer(verticesAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, planeNormalVectorsBufferId));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeOfPlaneNormalsArray * sizeof(float), planeNormalVectors, GL_STATIC_DRAW));
	GL_CHECK(glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

	GL_CHECK(glEnableVertexAttribArray(verticesAttributeLocation));
	GL_CHECK(glEnableVertexAttribArray(normalAttributeLocation));

	/* This vertex array object stores the normal cubes' vertices. */
	GL_CHECK(glBindVertexArray(normalCubeVertexArrayObjectId));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, normalCubeBufferId));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeOfNormalCubeVerticesArray * sizeof(float), normalCubeVertices, GL_STATIC_DRAW));
	GL_CHECK(glVertexAttribPointer(verticesAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

	GL_CHECK(glEnableVertexAttribArray(verticesAttributeLocation));

	/* This vertex array object stores rounded cube's vertices and normal vectors. */
	GL_CHECK(glBindVertexArray(roundedCubeVertexArrayObjectId));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, roundedCubeVerticesBufferId));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, numberOfRoundedCubeCoordinates * sizeof(float), roundedCubeCoordinates, GL_STATIC_DRAW));
	GL_CHECK(glVertexAttribPointer(verticesAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, roundedCubeNormalVectorsBufferId));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, numberOfRoundedCubeNormalVectors * sizeof(float), roundedCubeNormalVectors, GL_STATIC_DRAW));
	GL_CHECK(glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

	GL_CHECK(glEnableVertexAttribArray(verticesAttributeLocation));
	GL_CHECK(glEnableVertexAttribArray(normalAttributeLocation));

	/* Generate query objects. */
	GL_CHECK(glGenQueries(numberOfCubes, cubeQuery));

    /* Release allocated memory. */
    if (normalCubeVertices != NULL)
    {
        free(normalCubeVertices);
        normalCubeVertices = NULL;
    }

    if (planeVertices != NULL)
    {
        free(planeVertices);
        planeVertices = NULL;
    }

    if (planeNormalVectors != NULL)
    {
        free(planeNormalVectors);
        planeNormalVectors = NULL;
    }

    if(roundedCubeCoordinates != NULL)
    {
        delete[] roundedCubeCoordinates;
        roundedCubeCoordinates = NULL;
    }

    if(roundedCubeNormalVectors != NULL)
    {
        delete[] roundedCubeNormalVectors;
        roundedCubeNormalVectors = NULL;
    }

	return true;
}

/**
 * \brief Drwa the plane and cubes.
 * 
 * Renders all rounded cubes if occlusion quereies turned on,
 * otherwise just draws those visible.
 */
void draw(void)
{
    numberOfRoundedCubesDrawn = 0;

	/* Draw the plane. */
	GL_CHECK(glBindVertexArray(planeVertexArrayObjectId));
	GL_CHECK(glUniform1i(renderPlaneUniformLocation, 1));	
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, sizeOfPlaneVerticesArray));

    /* Draw the cubes. */
    GL_CHECK(glUniform1i(renderPlaneUniformLocation, 0));
	if (occlusionQueriesOn)
	{	
        for (int i = 0; i < numberOfCubes; i++)
        {  
            sendCubeLocationVectorToUniform(i);

		    /* Begin occlusion query. */
		    GL_CHECK(glBeginQuery(GL_ANY_SAMPLES_PASSED, cubeQuery[i]));
		    {
                if (previouslyVisible[i])
                {
                    /* If figure was visible in previous frame, enable color mask and draw the rounded cube. */
                    GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
                    GL_CHECK(glBindVertexArray(roundedCubeVertexArrayObjectId));
                    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, numberOfRoundedCubesVertices));
                    numberOfRoundedCubesDrawn++;
                }
                else
                {
                    /* Disable color mask to prevent showing normal cubes. */
                    GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
                    GL_CHECK(glBindVertexArray(normalCubeVertexArrayObjectId));
			        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, sizeOfNormalCubeVerticesArray));
		        }
            }
		    GL_CHECK(glEndQuery(GL_ANY_SAMPLES_PASSED));
		    /* End occlusion query. */
	    }

		/* Clear depth buffer and enable color mask to make rounded cubes visible. */
		GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        if (modeChanged)
        {
            GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
        }

        /* Draw rounded cubes. */
        GL_CHECK(glBindVertexArray(roundedCubeVertexArrayObjectId));	
        for(int i = 0; i < numberOfCubes; i++)
        {
            GLuint queryResult = GL_FALSE;
            /* Check query result. */
            GL_CHECK(glGetQueryObjectuiv(cubeQuery[i], GL_QUERY_RESULT, &queryResult));

            /* Update the 'previously visible' array. */
            bool wasVisible = previouslyVisible[i];
            previouslyVisible[i] = (queryResult == GL_TRUE);

            /* If the cube has become visible in this frame, render it again as a rounded cube. */
            if((queryResult == GL_TRUE) && !wasVisible)
            {
                sendCubeLocationVectorToUniform(i);
                GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, numberOfRoundedCubesVertices));
                numberOfRoundedCubesDrawn++;
            }
        }
	}
	else
	{
		/* Draw all rounded cubes without using occlusion queries. */
		GL_CHECK(glBindVertexArray(roundedCubeVertexArrayObjectId));
		for(int i = 0; i < numberOfCubes; i++)
		{
            sendCubeLocationVectorToUniform(i);
			GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, numberOfRoundedCubesVertices));
		}
        numberOfRoundedCubesDrawn = numberOfCubes;
	}
}

/**
 * \brief Render one frame. 
 */
void renderFrame(void)
{
    /* Clear color and depth buffers. */
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
 
    GL_CHECK(glUseProgram(programId));

    /* Rotation matrix used to rotate the view. */
	Matrix yRotationMatrix = Matrix::createRotationY(-angleY);

    /* Multiply viewMatrix and yRotationMatrix to make camera rotate around the scene. */
    rotatedViewMatrix = viewMatrix * yRotationMatrix;

	GL_CHECK(glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, rotatedViewMatrix.getAsArray()));

	/* 
     * Sort the cubes' positions. We have to do it in every frame because camera constantly moves around the scene. 
     * It is important that the cubes are rendered front to back because the occlusion test is done per draw call.
     * If the cubes are draw out of order then some cubes may pass the pcclusion test even when they end up being 
     * occluded by geometry drawn later.
     */
    sortCubePositions(sortedCubesPositions);

    modeChanged = false;

	/* Check timer to know if we should turn off/on occlusion queries. */
    if(timer.isTimePassed(intervalInSeconds))
	{
        occlusionQueriesOn = !occlusionQueriesOn;

		if(occlusionQueriesOn)
		{
            /* Unset the flags in 'previously visible' array */
            for (int i  = 0; i < numberOfCubes; i++)
            {
                previouslyVisible[i] = true;
            }

            /* Mark that mode has changed */
            modeChanged = true;
            
			LOGI("\nOcclusion query ON\n");
            text->clear();
            text->addString(0, 0, "Occlusion query ON", 255, 0, 0, 255);
		}
		else
		{
			LOGI("\nOcclusion query OFF\n");
            text->clear();
            text->addString(0, 0, "Occlusion query OFF", 255, 0, 0, 255);
		}
	}

	/* Increase angleY. */
	angleY += 0.25f;

	if(angleY >= 360)
    {
        angleY = 0.0f;
    }

	draw(); 

    text->draw();
}

/**
 * \brief Releases all OpenGL objects that were created with glGen*() or glCreate*() functions.
 */
void releaseOpenGlObjects()
{
	/* Delete the program. */
	GL_CHECK(glDeleteProgram(programId));

	/* Delete the buffer objects. */
	GL_CHECK(glDeleteBuffers(1, &planeVerticesBufferId));
	GL_CHECK(glDeleteBuffers(1, &planeNormalVectorsBufferId));
	GL_CHECK(glDeleteBuffers(1, &normalCubeBufferId));
	GL_CHECK(glDeleteBuffers(1, &roundedCubeVerticesBufferId));
	GL_CHECK(glDeleteBuffers(1, &roundedCubeNormalVectorsBufferId));

	/* Delete the vertex array objects. */
	GL_CHECK(glDeleteVertexArrays(1, &planeVertexArrayObjectId));
	GL_CHECK(glDeleteVertexArrays(1, &normalCubeVertexArrayObjectId));
	GL_CHECK(glDeleteVertexArrays(1, &roundedCubeVertexArrayObjectId));

	/* Delete the query objects. */
	GL_CHECK(glDeleteQueries(numberOfCubes, cubeQuery));
}

int main(int argc, char **argv)
{
	/* This line ensures that everytime we run the new instance of the program, rand() will generate different numbers. */
	srand((unsigned int)time(NULL));

    /* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

	if(platform != NULL)
	{
		/* Initialize windowing system. */
		platform->createWindow(windowWidth, windowHeight);

		/* Initialize EGL. */
		EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
        EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

		/* Prepare GL objects. */
		if(setupGraphics())
		{
            /* Timer variable to calculate FPS. */
            Timer fpsTimer;
            fpsTimer.reset();

			bool end = false;

			/* The rendering loop to draw the scene. */
			while(!end)
			{
				/* If something happened to the window, end the loop */        
				if(platform->checkWindow() != Platform::WINDOW_IDLE)
				{
					end = true;
				}

                /* Calculate FPS. */
                float FPS = fpsTimer.getFPS();
                if(fpsTimer.isTimePassed(1.0f))
                {
                    LOGI("FPS:\t%.1f", FPS);
                    LOGI("Number of Cubes drawn: %d\n", numberOfRoundedCubesDrawn);
                }

				/* Render a single frame */
				renderFrame();

				/* 
				 * Push the EGL surface color buffer to the native window.
				 * Causes the rendered graphics to be displayed on screen.
				 */
				eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
			}

			/* Release OpenGL objects. */
			releaseOpenGlObjects();
		}
		else
		{
			LOGE("Could not prepare GL objects.\n");
		}

		/* Shut down OpenGL ES. */
		/* Shut down EGL. */
		EGLRuntime::terminateEGL();

		/* Shut down windowing system. */
		platform->destroyWindow();

		/* Shut down the Platform object*/
		delete platform;
	}
	else
	{
		LOGE("Could not create platform");
	}
	return 0;
}