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
 * \file Instancing.cpp
 * \brief Demonstration of instanced drawing and uniform buffers in OpenGL ES 3.0.
 * 
 * There is only one copy of the cube vertex data in memory, each of the cubes drawn is an instance of that data.
 * This reduces the ammount of memory which needs to be transfered to the GPU.
 * By using gl_instanceID in the shader, each of the cubes can have a different position, rotation speed and colour.
 * This technique can be used everywhere repeated geometry is used in a scene.
 */

#include "Instancing.h"

#include "EGLRuntime.h"
#include "CubeModel.h"
#include "Platform.h"
#include "Shader.h"
#include "Timer.h"
#include "Mathematics.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <string>

using namespace MaliSDK;
using std::string;

/* Asset directory */
string resourceDirectory = "assets/";

/* Instance of a timer. Used for setting positions and rotations of cubes. */
Timer timer; 

/* Program used for transforming vertices into world space. */
/* Fragment shader name. */
GLuint      fragmentShaderId   = 0; 
string fragmentShaderPath = resourceDirectory + "fragment_shader_source.frag";
/* Vertex shader name. */      
GLuint      vertexShaderId   = 0; 
string vertexShaderPath = resourceDirectory + "vertex_shader_source.vert";
/* Program name. */
GLuint renderingProgramId = 0;     

/* Cubes. */
/* Number of cubes that are drawn on a screen. */
const int numberOfCubesToGenerate = 10;
/* Number of coordinates written to cubeTrianglesCoordinates array*/
int numberOfCubeTriangleCoordinates = 0;
/* Array holding coordinates of triangles which a cube consists of. */
float* cubeTrianglesCoordinates = NULL;
/* Number of values written to vertexColors array. */
int numberOfValuesInVertexColorsArray = 0;
/* Array holding color values for each vertex of cube triangle. */
float* vertexColors = NULL;
/* Number of values written to cubeColors array. RGBA values for each cube. */
const int numberOfValuesInCubeColorsArray = 4 * numberOfCubesToGenerate;
/* Array holding color values for each cube. */
float cubeColors[numberOfValuesInCubeColorsArray] = {0};
/* Scaling factor indicating size of a cube. */
const float cubeSize = 2.5f;

/* Window. */
/* Height of window. */
const int windowHeight = 600;
/* Width of window. */
const int windowWidth = 800;   

/* Uniform and attribute locations. */
/* "Camera position" shader uniform which is used to set up a view. */
GLint cameraPositionLocation = 0;
/* Shader uniform block index. */
GLuint uniformBlockIndex = 0;
/* "Perspective matrix" shader uniform's location. */
GLint perspectiveMatrixLocation = 0;
/* "Position" shader attribute's location. */
GLint positionLocation = 0;
/* "Color" shader attribute's location. */
GLint cubeVertexColorLocation = 0;
/* "Time" shader uniform that is used to hold timer value. */
GLint timeLocation = 0;

/* Buffer objects. */
/* Constant telling number of buffer objects that should be generated: 
*  - buffer object holding cube coordinates,
*  - buffer object holding per-vertex cube colors,
*  - buffer object holding data used in uniform block.
*/
const GLuint numberOfBufferObjectIds = 3;
/* Array of buffer object names. */
GLuint bufferObjectIds[numberOfBufferObjectIds] = {0};
/* Name of buffer object which holds color of triangle vertices. */
GLuint cubeColorsBufferObjectId = 0;
/* Name of buffer object which holds coordinates of triangles making cube. */
GLuint cubeCoordinatesBufferObjectId = 0;
/* Name of buffer object which holds start positions of cubes and values of color for each cube. */
GLuint uniformBlockDataBufferObjectId = 0;    

/* Start positions of cubes in 3D space. */
/* Array holding start position of cubes in 3D space which are used to draw cubes for the first time. */
float startPosition[numberOfCubesToGenerate] = {0};  

/* 
 * Arrays holding data used for setting perspective and view.
 * 45.0f - field of view angle (in degrees) in the y direction
 * (float)windowWidth / (float)windowHeight - ratio used to calculate the field of view in the x direction.
 * 0.1f - distance from the camera to the near clipping plane.
 * 1000.0f - distance from the camera to the far clipping plane.
*/
const float perspectiveVector[] = {45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 1000.0f};
/* Array used for view configuration in vertex shader. */
const float cameraVector[] = {0.0f, 0.0f, -60.0f};

/**
 * \brief Generate positions of cubes which are used during first draw call. Cubes are located on a circular curve. 
 */
void generateStartPosition(void)
{
    float spaceBetweenCubes = (2 * M_PI) / (numberOfCubesToGenerate);

    /* Fill array with startPosition data. */
    for (int allCubes = 0; allCubes < numberOfCubesToGenerate; allCubes++)
    {
        startPosition[allCubes] = allCubes * spaceBetweenCubes;
    }
}

/**
 * \brief Fill cubeColors array with random color (used for setting random color for each cube). 
 */
void fillCubeColorsArray(void)
{
    for (int allComponents = 0; allComponents < numberOfValuesInCubeColorsArray; allComponents++)
    {
        cubeColors[allComponents] = (float)rand() / (float)RAND_MAX;
    }
}

/* 
 * \brief Fill vertexColors array with random color for each triangle vertex.
 *
 * \return false if an error appeared, true otherwise.
 */
bool fillVertexColorsArray(void)
{
    /* 
     * Number of cube triangle vertices is equal to numberOfCubeTriangleCoordinates / 3.0. 
     * For each vertex there is one color described (each color consists of R, G, B and A values). 
     */
    numberOfValuesInVertexColorsArray = int (numberOfCubeTriangleCoordinates * 4 / 3);

    /* Allocate memory for vertexColors array. */
    vertexColors = (float*) malloc (numberOfValuesInVertexColorsArray * sizeof(float));

    if (vertexColors != NULL)
    {
        for (int allComponents = 0; allComponents < numberOfValuesInVertexColorsArray; allComponents++)
        {
            vertexColors[allComponents] = (float)rand() / (float)RAND_MAX;
        }
    }
    else
    {
        LOGE ("Could not allocate memory for vertexColors array.");
        return false;
    }

    return true;
}

/** 
 * Initialize data for cubes. 
 *
 * \return false if an error appeared, true otherwise.
 */
bool createCubesData(void)
{
    /* Get triangular representation of a cube. Save data in cubeTrianglesCoordinates array. */
    CubeModel::getTriangleRepresentation(cubeSize, &numberOfCubeTriangleCoordinates, &cubeTrianglesCoordinates);

    /* Make sure triangular representation of a cube was created successfully. */
    if (cubeTrianglesCoordinates == NULL)
    {
        return false;
    }

    /* Set start values of positions. */
    generateStartPosition();

    /* Calculate color for each cube. */
    fillCubeColorsArray();

    /* Calculate color for each vertex of a cube. */
    if (!fillVertexColorsArray())
    {
        free(cubeTrianglesCoordinates);
        cubeTrianglesCoordinates = NULL;

        return false;
    }

    return true;
}

/* 
 * Initializes data used for rendering.
 *
 * \return false if an error appeared, true otherwise.
 */
bool initializeData(void)
{
    /* Create all data needed to draw cube. */
    if(createCubesData())
    { 
        /* Settings for 3D shape drawing. */
        GL_CHECK(glDisable(GL_CULL_FACE));
        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glDepthFunc(GL_LEQUAL));

        /* Generate buffers. */
        GL_CHECK(glGenBuffers(numberOfBufferObjectIds, bufferObjectIds));

        cubeCoordinatesBufferObjectId  = bufferObjectIds[0];
        cubeColorsBufferObjectId       = bufferObjectIds[1];
        uniformBlockDataBufferObjectId = bufferObjectIds[2];

        /* Fill buffer object with vertex data. */
        /* Buffer holding coordinates of triangles which create a cube. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cubeCoordinatesBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, numberOfCubeTriangleCoordinates * sizeof(float), cubeTrianglesCoordinates, GL_STATIC_DRAW));

        /* Buffer holding RGBA values of color for each vertex. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cubeColorsBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, numberOfValuesInVertexColorsArray * sizeof(float), vertexColors, GL_STATIC_DRAW));

        /* Buffer holding coordinates of start positions of cubes and RGBA values of colors. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, uniformBlockDataBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(startPosition) + sizeof(cubeColors), NULL, GL_STATIC_DRAW));

        GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0,                     sizeof(startPosition), startPosition));
        GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, sizeof(startPosition), sizeof(cubeColors),    cubeColors));

        /* Deallocate memory (data is now stored in buffer objects). */
        free(vertexColors);
        vertexColors = NULL;

        free(cubeTrianglesCoordinates);
        cubeTrianglesCoordinates = NULL;
    }
    else
    {
        LOGE("Could not initialize data used for rendering.");
        return false;
    }

    return true;
}

/* 
 * Create programs that will be used to rasterize the geometry of cubes.
 *
 * \return false if an error appeared, true otherwise.
 */
bool setupProgram()
{
    bool functionCallResult = true; 

    /* Create program object. */
    renderingProgramId = GL_CHECK(glCreateProgram());

    /* Initialize rendering program. */
    Shader::processShader(&vertexShaderId,   vertexShaderPath.c_str(),   GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderId, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    /* Attach vertex and fragment shaders to rendering program. */
    GL_CHECK(glAttachShader(renderingProgramId, vertexShaderId));
    GL_CHECK(glAttachShader(renderingProgramId, fragmentShaderId));

    /* Link and use rendering program object. */
    GL_CHECK(glLinkProgram(renderingProgramId));
    GL_CHECK(glUseProgram(renderingProgramId));

    /* Get uniform, attribute and uniform block locations from current program. */
    positionLocation          = GL_CHECK(glGetAttribLocation   (renderingProgramId, "attributePosition"));
    cubeVertexColorLocation   = GL_CHECK(glGetAttribLocation   (renderingProgramId, "attributeColor"));
    perspectiveMatrixLocation = GL_CHECK(glGetUniformLocation  (renderingProgramId, "perspectiveVector"));
    cameraPositionLocation    = GL_CHECK(glGetUniformLocation  (renderingProgramId, "cameraVector"));
    uniformBlockIndex         = GL_CHECK(glGetUniformBlockIndex(renderingProgramId, "CubesUniformBlock"));
    timeLocation              = GL_CHECK(glGetUniformLocation  (renderingProgramId, "time"));

    if (positionLocation == -1)
    {
        LOGE("Could not retrieve attribute location: positionLocation");
        functionCallResult = false;
    }

    if (cubeVertexColorLocation == -1)
    {
        LOGE("Could not retrieve attribute location: cubeVertexColorLocation");
        functionCallResult = false;
    }

    if (perspectiveMatrixLocation == -1)
    {
        LOGE("Could not retrieve uniform location: perspectiveMatrixLocation");
        functionCallResult = false;
    }

    if (cameraPositionLocation == -1)
    {
        LOGE("Could not retrieve uniform location: cameraPositionLocation");
        functionCallResult = false;
    }

    if (timeLocation == -1)
    {
        LOGE("Could not retrieve uniform location: timeLocation");
        functionCallResult = false;
    }

    if (uniformBlockIndex == GL_INVALID_INDEX)
    {
        LOGE("Could not retrieve uniform block index: uniformBlockIndex");
        functionCallResult = false;
    }

    return functionCallResult;
}

/**
 * \brief Render new frame's contents into back buffer. 
 */
void renderFrame(void)
{
    /* Clear contents of back buffer. */
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Value of time returned by timer used for setting cubes rotations and positions. */
    const float time = timer.getTime();

    GL_CHECK(glUseProgram(renderingProgramId));

    /* Set values for uniforms for model-view program. */
    GL_CHECK(glUniform4fv(perspectiveMatrixLocation, 1, perspectiveVector));
    GL_CHECK(glUniform3fv(cameraPositionLocation,    1, cameraVector));
    GL_CHECK(glUniform1f (timeLocation,              time));

    /* Rasterizer pass. */
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cubeCoordinatesBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(positionLocation));
    GL_CHECK(glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cubeColorsBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(cubeVertexColorLocation));
    GL_CHECK(glVertexAttribPointer(cubeVertexColorLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));

    /* Set binding point for uniform block. */
    GL_CHECK(glUniformBlockBinding(renderingProgramId, uniformBlockIndex, 0));
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBlockDataBufferObjectId));

    /* Draw cubes on a screen. */
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLES, 0, numberOfCubeTriangleCoordinates, numberOfCubesToGenerate));
}

int main(int argc, char **argv)
{
    /* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if(platform != NULL)
    {
        /* Initialize windowing system. */
        platform->createWindow(windowWidth, windowHeight);

        /* Initialize EGL. */
        EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
        EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

        /* Initialize data used for rendering. */
        if(initializeData())
        {
            /* Create program. */
            if(setupProgram())
            {
                /* Start counting time. */
                timer.reset();

                /* Rendering loop to draw the scene starts here. */
                bool shouldContinueTheLoop = true;

                while (shouldContinueTheLoop)
                {
                    /* If something happened to the window, leave the loop. */
                    if(platform->checkWindow() != Platform::WINDOW_IDLE)
                    {
                        shouldContinueTheLoop = false;              
                    }

                    /* Render a single frame */
                    renderFrame();

                    eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
                }
            }
            else
            {
                LOGE("Could not create program successfully.");
            }

            /* Delete buffers. */
            GL_CHECK(glDeleteBuffers(numberOfBufferObjectIds, bufferObjectIds));
        }
        else
        {
            LOGE("Could not initialize data used for rendering.");
        }
        /* Shut down EGL. */
        EGLRuntime::terminateEGL();

        /* Shut down windowing system. */
        platform->destroyWindow();

        /* Shut down the Platform object. */
        delete platform;
    }
    else
    {
        LOGE("Could not create platform.");
    }

    return 0;
}