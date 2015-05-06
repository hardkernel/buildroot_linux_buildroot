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
 * \file Boids.cpp
 * \brief Demonstration of Transform Feedback functionality in OpenGL ES 3.0.
 *
 * Also demonstrates the use of uniform buffers.
 * The application displays 30 spheres on a screen. Locations and velocities of the spheres in 3D space are regularly updated to simulate bird flock.
 * There is 1 leader sphere (red) and 29 followers (green). The leader follows a set looping path and the followers 'flock' in relation to the leader 
 * and the other followers.
 * The calculation of the locations of the boids is done on the GPU each frame using a vertex shader prior to rendering the scene.
 * All of the data for the boids stays in GPU memory (by using buffers) and is not transferred back to the CPU.
 * Transform feedback buffers are used to store the output of the movement vertex shader, this data is then used as the input data on the next pass.
 * The same data is used when rendering the scene.
 */

#include "Boids.h"

#include "EGLRuntime.h"
#include "Platform.h"
#include "SphereModel.h"
#include "Shader.h"
#include "Timer.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <string>

using namespace MaliSDK;
using std::string;

/* Asset directory. */
string resourceDirectory = "assets/";

/* Instance of a timer used as input for path generation for leader. It is also used to keep the leader's velocity constant across different GPUs. */
Timer timer; 

/* Program used for transforming vertices into world space. */
/* Fragment shader name. */
GLuint fragmentShaderId = 0; 
string fragmentShaderPath = resourceDirectory + "fragment_shader_source.frag";
/* Vertex shader name. */      
GLuint vertexShaderId = 0; 
string vertexShaderPath = resourceDirectory + "vertex_shader_source.vert";
/* Program name. */
GLuint renderingProgramId = 0;     

/* Program is used for iteratively calculating translation and velocity of spheres by means of transform feedback. */
/* Fragment shader name. */
GLuint movementFragmentShaderId = 0; 
string movementFragmentShaderPath = resourceDirectory + "movement.frag";
/* Vertex shader name. */
GLuint movementVertexShaderId = 0; 
string movementVertexShaderPath = resourceDirectory + "movement.vert";
/* Program name. */
GLuint movementProgramId = 0;     

/* Spheres. */
/* A sphere consists of @param numberOfSamples circles and @param numberOfSamples points lying on one circle. */
const int numberOfSamples = 20;
/* Number of spheres that are drawn on a screen. */
const int numberOfSpheresToGenerate = 30;
/* Number of coordinates written to sphereTrianglesCoordinates array*/
int numberOfSphereTriangleCoordinates = 0;
/* Array holding coordinates of triangles which a sphere consists of. */
float* sphereTrianglesCoordinates = NULL;
/* Size of vertexColors array. */
int colorArraySize = 0;
/* Array holding color values for each vertex of sphere triangle. */
float* vertexColors = NULL; 

/* Window. */
/* Height of window. */
const int windowHeight = 600;
/* Width of window. */
const int windowWidth = 800;   

/* Uniform and attribute locations. */
/* "Camera position" shader uniform which is used to set up a view. */
GLint cameraPositionLocation = 0;
/* Shader uniform block index. */
GLuint movementUniformBlockIndex = 0;
/* "Perspective matrix" shader uniform's location. */
GLint perspectiveMatrixLocation = 0;
/* "Position" shader attribute's location. */
GLint positionLocation = 0;
/* "Scaling matrix" shader uniform's location. */
GLint scalingMatrixLocation = 0;
/* "Color" shader attribute's location. */
GLint sphereVertexColorLocation = 0;
/* "Time" shader uniform is used to hold timer value. timeLocation provides information about the uniform's location. */
GLint timeLocation = 0; 

/* Buffer objects. */
/* If true - ping buffer object is used as transform feedback output. Otherwise pong buffer object should be used. */
bool usePingBufferForTransformFeedbackOutput = true;
/* Constant telling number of buffer objects that should be generated. */
const GLuint numberOfBufferObjectIds = 4;
/* Array of buffer object names. */
GLuint bufferObjectIds[numberOfBufferObjectIds] = {0};
/* There are 4 coordinates for each uniform, and 2 uniforms (location and velocity) for each sphere. */
const int spherePositionsAndVelocitiesLength = 4 * 2 * numberOfSpheresToGenerate;
/* Name of buffer object which holds color of triangle vertices. */
GLuint sphereColorsBufferObjectId  = 0;
/* Name of buffer object which holds coordinates of triangles making sphere. */
GLuint sphereCoordinatesBufferObjectId = 0;
/* Name of buffer object which holds newly generated data containing location and velocity of spheres. */
GLuint spherePingPositionAndVelocityBufferObjectId = 0;
/* Name of buffer object which holds location and velocity data from previous iteration. */
GLuint spherePongPositionAndVelocityBufferObjectId = 0;     

/* Positions and velocities of spheres in 3D space. */
/* Array holding positions and velocities of spheres in 3D space which are used to draw spheres for the first time. */
float startPositionAndVelocity[spherePositionsAndVelocitiesLength] = {0};  

/**
* \brief Generate random positions and velocities of spheres which are used during first draw call.
*/
void generateStartPositionAndVelocity(void)
{
    /* Fill the array with position data (starting at index 0). */
    for (int allComponents = 0; allComponents < 4 * numberOfSpheresToGenerate; allComponents++)
    {
        /* Random data with range -20 to -10. */
        startPositionAndVelocity[allComponents] = 10.0f * (float(rand()) / float(RAND_MAX)) - 20.0f;
    }

    /* Fill array with velocity data (which follows position data). */
    for (int allComponents = 4 * numberOfSpheresToGenerate; allComponents < 4 * 2 * numberOfSpheresToGenerate; allComponents++)
    {
        startPositionAndVelocity[allComponents] = 0;
    }
}

/** 
* \brief Fill vertexColors array with random color for each triangle vertex.
*
* \return false if an error appeared, true otherwise.
*/
bool fillVertexColorsArray(void)
{
    /* 
    * Number of vertices for all the spheres is equal to numberOfSphereTriangleCoordinates / 3 (3 verticies per triangle).
    * For each vertex there are 4 color components (R, G, B and A values). 
    */
    colorArraySize = int (numberOfSphereTriangleCoordinates / 3.0f * 4.0f);

    /* Allocate memory for vertexColors array. */
    vertexColors = (float*) malloc (colorArraySize * sizeof(float));

    if (vertexColors != NULL)
    {
        for (int i = 0; i < colorArraySize; i++)
        {
            vertexColors[i] = float(rand()%RAND_MAX)/float(RAND_MAX);
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
* \brief Initialize data for spheres. 
*
* \return false if an error appeared, true otherwise.
*/
bool createSpheresData(void)
{
    /* Radius of the spheres. */
    const float radius = 10.0f;

    SphereModel::getTriangleRepresentation(radius, numberOfSamples, &numberOfSphereTriangleCoordinates, &sphereTrianglesCoordinates);
    generateStartPositionAndVelocity();

    if (!fillVertexColorsArray())
    {
        return false;
    }

    return true;
}

/** 
* \brief Initializes data used for rendering.
*
* \return false if an error appeared, true otherwise.
*/
bool initializeData(void)
{
    /* Create all data needed to draw sphere. */
    if(createSpheresData())
    {
        /* Enable blending. */
        GL_CHECK(glEnable(GL_BLEND));

        /* Settings for 3D shape drawing. */
        GL_CHECK(glDisable(GL_CULL_FACE));
        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glDepthFunc(GL_LEQUAL));

        /* Generate buffers. */
        GL_CHECK(glGenBuffers(numberOfBufferObjectIds, bufferObjectIds));

        sphereCoordinatesBufferObjectId             = bufferObjectIds[0];
        sphereColorsBufferObjectId                  = bufferObjectIds[1];
        spherePingPositionAndVelocityBufferObjectId = bufferObjectIds[2];
        spherePongPositionAndVelocityBufferObjectId = bufferObjectIds[3];

        /* Fill buffer object with vertex data. */
        /* Buffer holding coordinates of triangles which create a sphere. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, sphereCoordinatesBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, numberOfSphereTriangleCoordinates, sphereTrianglesCoordinates, GL_STATIC_DRAW));

        /* Buffer holding RGBA values of color for each vertex. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, sphereColorsBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, colorArraySize, vertexColors, GL_STATIC_DRAW));

        /* Buffers holding coordinates of sphere positions and velocities which are used by transform feedback (to read from or write computed data). */
        /* Set buffers' size and usage, but do not fill them with any data. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, spherePingPositionAndVelocityBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, spherePositionsAndVelocitiesLength * sizeof(float), NULL, GL_STATIC_DRAW));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, spherePongPositionAndVelocityBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, spherePositionsAndVelocitiesLength * sizeof(float), NULL, GL_STATIC_DRAW));

        /* Deallocate memory (data are now saved in buffer objects). */
        free(vertexColors);
        vertexColors = NULL;

        free(sphereTrianglesCoordinates);
        sphereTrianglesCoordinates = NULL;
    }
    else
    {
        LOGE("Could not initialize data used for rendering.");
        return false;
    }

    return true;
}

/* 
* \brief Create programs that will be used to rasterize the geometry and transforming the spheres.
*
* \return false if an error appeared, true otherwise.
*/
bool setupPrograms(void)
{
    /* Boolean value to be returned. */
    bool functionCallResult = true;

    /* Array of varyings' names which are used by shader for transform feedback. */ 
    const GLchar* varyingNames[] = {"location", "velocity"}; 

    /* Create program objects. */
    movementProgramId  = GL_CHECK(glCreateProgram());
    renderingProgramId = GL_CHECK(glCreateProgram());

    /* Initialize movement program. */
    Shader::processShader(&movementVertexShaderId,   movementVertexShaderPath.c_str(),   GL_VERTEX_SHADER);
    Shader::processShader(&movementFragmentShaderId, movementFragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    /* Attach vertex and fragment shaders to the program which is used for transform feedback. */
    GL_CHECK(glAttachShader(movementProgramId, movementVertexShaderId));
    GL_CHECK(glAttachShader(movementProgramId, movementFragmentShaderId));

    /*
     * Specify varyings which are used with transform feedback buffer.
     * In shader we are using uniform block for holding location and velocity data.
     * Uniform block takes data from buffer object. Buffer object is filled with position data for each sphere first, and then with velocity data for each sphere.
     * Setting mode to GL_SEPARATE_ATTRIBS indicates that data are written to output buffer in exactly the same way as in input buffer object. 
     */
    GL_CHECK(glTransformFeedbackVaryings(movementProgramId, 2, varyingNames, GL_SEPARATE_ATTRIBS));

    /* Link and use movement porgram object. */
    GL_CHECK(glLinkProgram(movementProgramId));
    GL_CHECK(glUseProgram(movementProgramId));

    /* Get uniform locations from current program. */
    timeLocation = GL_CHECK(glGetUniformLocation(movementProgramId, "time"));

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
    sphereVertexColorLocation = GL_CHECK(glGetAttribLocation   (renderingProgramId, "attributeColor"));
    scalingMatrixLocation     = GL_CHECK(glGetUniformLocation  (renderingProgramId, "scalingVector"));
    perspectiveMatrixLocation = GL_CHECK(glGetUniformLocation  (renderingProgramId, "perspectiveVector"));
    cameraPositionLocation    = GL_CHECK(glGetUniformLocation  (renderingProgramId, "cameraVector"));
    movementUniformBlockIndex = GL_CHECK(glGetUniformBlockIndex(renderingProgramId, "BoidsUniformBlock"));

    /* Check if all uniforms, attributes and uniform blocks were found in vertex shaders. */
    if (scalingMatrixLocation == -1)
    {
        LOGE("Could not retrieve uniform location: scalingMatrixLocation");
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

    /* Fill position and velocity buffer with data. */
    if (functionCallResult)
    {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, spherePongPositionAndVelocityBufferObjectId));
        GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(startPositionAndVelocity), startPositionAndVelocity));
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

    /* Value of time returned by timer used for determining leader's position and to keep the leader's velocity constant across different GPUs. */
    float timerTime = timer.getTime(); 
    /* Scale factor for displaying 3D shape. */
    const float scalingFactor = 0.1f;
    /* Array used for setting scale matrix in vertex shader. */
    float scalingVector[] = {scalingFactor, scalingFactor, scalingFactor};
    /* Array used for setting perspective matrix in vertex shader. */
    float perspectiveVector[] = {45.0f, float(windowWidth)/float(windowHeight), 0.1f, 1000.0f};
    /* Array used for view configuration in vertex shader. */
    float cameraVector[] = {0.0f, 0.0f, -60.0f};

    /* Set values for uniforms for model-view program. */
    GL_CHECK(glUseProgram(renderingProgramId));
    GL_CHECK(glUniform3fv(scalingMatrixLocation,     1, scalingVector));
    GL_CHECK(glUniform4fv(perspectiveMatrixLocation, 1, perspectiveVector));
    GL_CHECK(glUniform3fv(cameraPositionLocation,    1, cameraVector));

    /* Set values for uniforms for transform feedback program. */
    GL_CHECK(glUseProgram(movementProgramId));
    GL_CHECK(glUniform1f(timeLocation, timerTime));

    /* 
     * Transform feedback is used for setting position and velocity for each of the spheres.
     * You cannot read from and write to the same buffer object at a time, so we use a ping-pong approach.
     * During first call, ping buffer is used for reading and pong buffer for writing.
     * During second call, pong buffer is used for reading and ping buffer for writing. 
     */

    /* 
     * Configure transform feedback.
     * Bind buffer object to first varying (location) of GL_TRANSFORM_FEEDBACK_BUFFER - binding point index equal to 0.
     * Use the first half of the data array, 0 -> sizeof(float) * 4 * numberOfSpheresToGenerate (4 floating point position coordinates per sphere).
     * Bind buffer object to first varying (velocity) of GL_TRANSFORM_FEEDBACK_BUFFER - binding point index equal to 1.
     * Use the second half of the data array, from the end of the position data until the end of the velocity data.
     * The size of the velocity data is sizeof(float) * 4 * numberOfSpheresToGenerate values (4 floating point velocity coordinates per sphere).
     * 
     * The buffer bound here is used as an output from the movement vertex shader. The output variables in the shader that are bound to this buffer are 
     * given by the call to glTransformFeedbackVaryings earlier.
     */
    if (usePingBufferForTransformFeedbackOutput)
    {
        GL_CHECK(glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, spherePingPositionAndVelocityBufferObjectId, 0,                                             sizeof(float) * 4 * numberOfSpheresToGenerate));
        GL_CHECK(glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, spherePingPositionAndVelocityBufferObjectId, sizeof(float) * 4 * numberOfSpheresToGenerate, sizeof(float) * 4 * numberOfSpheresToGenerate));
    }
    else
    {
        GL_CHECK(glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, spherePongPositionAndVelocityBufferObjectId, 0,                                             sizeof(float) * 4 * numberOfSpheresToGenerate));
        GL_CHECK(glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, spherePongPositionAndVelocityBufferObjectId, sizeof(float) * 4 * numberOfSpheresToGenerate, sizeof(float) * 4 * numberOfSpheresToGenerate));
    }

    /* Set binding point for uniform block. */
    GLuint transformationUniformBlockIndex = GL_CHECK(glGetUniformBlockIndex(movementProgramId, "inputData"));
    GL_CHECK(glUniformBlockBinding(movementProgramId, transformationUniformBlockIndex, 0));

    /*
     * The buffer bound here is used as the input to the movement vertex shader. The data is mapped to the uniform block, and as the size of the 
     * arrays inside the uniform block is known, the data is mapped to the correct variables.
     */
    if (usePingBufferForTransformFeedbackOutput)
    {
        GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, spherePongPositionAndVelocityBufferObjectId));
    }
    else
    {
        GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, spherePingPositionAndVelocityBufferObjectId));
    }             

    /* 
     * Perform the boids transformation. 
     * This takes the current boid data in the buffers and passes it through the movement vertex shader.
     * This fills the output buffer with the updated location and velocity information for each boid.
     */
    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    {
        GL_CHECK(glUseProgram(movementProgramId));
        GL_CHECK(glBeginTransformFeedback(GL_POINTS));
        {
            GL_CHECK(glDrawArraysInstanced(GL_POINTS, 0, 1, numberOfSpheresToGenerate));
        }
        GL_CHECK(glEndTransformFeedback());
    }
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));

    /* Clean up. */
    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0));
    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, 0));
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER,            0, 0));

    GL_CHECK(glUseProgram(0));

    /* 
     * Rasterizer pass. 
     * Render the scene using the calculated locations of the boids.
     */
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, sphereCoordinatesBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(positionLocation));
    GL_CHECK(glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, sphereColorsBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(sphereVertexColorLocation));
    GL_CHECK(glVertexAttribPointer(sphereVertexColorLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));

    /* Set input buffer for rendering program. */ 
    GL_CHECK(glUseProgram(renderingProgramId));

    /* Set binding point for uniform block. */
    GLuint renderingUniformBlockIndex = GL_CHECK(glGetUniformBlockIndex(renderingProgramId, "BoidsUniformBlock"));
    GL_CHECK(glUniformBlockBinding(renderingProgramId, renderingUniformBlockIndex, 0));

    /* Bind the data calculated during transform feedback to the input of the shader. */
    if (usePingBufferForTransformFeedbackOutput)
    {
        GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, spherePingPositionAndVelocityBufferObjectId));
    }
    else
    {
        GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, spherePongPositionAndVelocityBufferObjectId));
    }

    /* Draw spheres. */
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLES, 0, numberOfSphereTriangleCoordinates, numberOfSpheresToGenerate));

    /* Swap the ping and pong buffer objects. */
    usePingBufferForTransformFeedbackOutput = !usePingBufferForTransformFeedbackOutput;
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
            /* Create programs. */
            if(setupPrograms())
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
                LOGE("Could not create programs successfully.");
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