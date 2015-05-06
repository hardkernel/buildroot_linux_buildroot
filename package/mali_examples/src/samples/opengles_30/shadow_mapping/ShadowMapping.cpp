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
 * \file ShadowMapping.cpp
 * Demonstration of shadow mapping functionality.
 * Application displays two cubes on a plane which are lit with directional and spot lights. 
 * Location and direction of the spot light source (represented by a small yellow cube flying above the scene) 
 * in 3D space are regularly updated.
 * The cube and planes models are shadow receivers, but only the cubes are shadow casters.
 * The application uses shadow mapping for rendering and displaying shadows.
 */

#include "ShadowMapping.h"

#include "EGLRuntime.h"
#include "PlaneModel.h"
#include "Matrix.h"
#include "CubeModel.h"
#include "Platform.h"
#include "Shader.h"
#include "Timer.h"
#include "Mathematics.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <string>

using std::string;
using namespace MaliSDK;

/* Asset directory. */
string resourceDirectory = "assets/";

/* Structure holding all data needed to initialize mesh-related buffer objects. */
struct GeometryProperties
{
    float* coordinates;                        /* Array holding the geometry's coordinates. */
    float* normals;                            /* Array holding the geometry's normal vectors. */
    float* position;                           /* Array holding position of the geometry in 3D space. */ 
    int    numberOfElementsInCoordinatesArray; /* Number of elements written to coordinates array. */
    int    numberOfElementsInNormalsArray;     /* Number of elements written to an array holding geometry's normal vectors. */
    int    numberOfElementsInPositionArray;    /* Number of elements written to position array. */
    float  scalingFactor;                      /* Factor used for scaling geometry. */
} plane, cube, lightRepresentation;

/* Structure holding window's data. */
struct WindowProperties
{
    int height; /* Height of window. */
    int width; /* Width of window. */
} window;

/* Structure holding properties of a light source. */
struct LightProperties
{
    Vec3f position;
    Vec3f direction;
} light;

/* Structure holding the data describing shadow-map texture. */
struct ShadowMapTextureProperties
{
    GLuint  framebufferObjectName; /* Name of a framebuffer object used for rendering depth texture. */
    GLsizei size;                  /* Size (width * height) of the shadow map texture. */
    GLuint  textureName;           /* Name of a shadow map texture (holds depth values for cubes-plane model from the light's point of view). */
} shadowMap;

/* Structure holding the data used for configuring the program object that is responsible for drawing cubes and plane and calculating the shadow map. */
struct CubesAndPlaneProgramProperties
{
    GLuint programId;                       /* Program name. Program does the following tasks:
                                             * - moving vertices of a geometry into eye-space,
                                             * - shading the rasterized primitive considering directional and spot lights and the shadow map for the spot light.
                                             */
    GLuint colorOfGeometryLocation;         /* Shader uniform location that is used to hold color of a geometry. Use different color for cubes and plane. */
    GLuint isCameraPointOfViewLocation;     /* Shader uniform location that is used to hold a boolean value indicating whether the point of view of 
                                             * the camera (if true) or the light (if false) should be used for rendering the scene.
                                             * The camera's point of view is used for the final pass, the light's point of view is used for calculating the shadow map.
                                             */
    GLint  lightDirectionLocation;          /* Shader uniform location that is used to hold direction of light (from cubes-plane model). */
    GLint  lightPositionLocation;           /* Shader uniform location that is used to hold position of light source (from cubes-plane model). */
    GLuint lightViewMatrixLocation;         /* Shader uniform location that refers to view matrix used for rendering the scene from light point of view. */
    GLuint normalsAttributeLocation;        /* Shader attribute location that is used to hold normal vectors of the cubes or the plane. */
    GLuint positionAttributeLocation;       /* Shader attribute location that is used to hold coordinates of the cubes or the plane to be drawn. */
    GLint  shadowMapLocation;               /* Shader uniform location that is used to hold the shadow map texture unit id. */
    GLuint shouldRenderPlaneLocation;       /* Shader uniform location that is used to hold boolean value indicating whether the plane (if true) or the cubes 
                                             * (if false) are being drawn (different data is used for cubes and plane). 
                                             */
} cubesAndPlaneProgram;

/* Structure holding data used for configuring the program object that is responsible for drawing representation of a spot light (yellow cube). */
struct LightRepresentationProgramProperties
{
    GLuint programId;        /* 
                              * Program name. Program does the following tasks:
                              * - moving vertices of geometry into eye-space,
                              * - setting color of the cube.
                              */    
    GLint  positionLocation; /* Shader uniform location that is used to hold position of the light cube. */
} lightRepresentationProgram;

/* Instance of a timer. Used for setting position and direction of light source. */
Timer timer;

/* Buffer object names. */
GLuint cubeCoordinatesBufferObjectId                = 0; /* Name of buffer object which holds coordinates of the triangles making up the scene cubes. */
GLuint cubeNormalsBufferObjectId                    = 0; /* Name of buffer object which holds the scene cubes normal vectors. */
GLuint lightRepresentationCoordinatesBufferObjectId = 0; /* Name of buffer object which holds coordinates of the light cube. */
GLuint planeCoordinatesBufferObjectId               = 0; /* Name of buffer object which holds coordinates of the plane. */
GLuint planeNormalsBufferObjectId                   = 0; /* Name of buffer object which holds plane's normal vectors. */
GLuint uniformBlockDataBufferObjectId               = 0; /* Name of buffer object which holds positions of the scene cubes. */

/* Vertex array objects. */
GLuint cubesVertexArrayObjectId                          = 0; /* Name of vertex array object used when rendering the scene cubes. */
GLuint lightRepresentationCoordinatesVertexArrayObjectId = 0; /* Name of vertex array object used when rendering the light cube. */
GLuint planeVertexArrayObjectId                          = 0; /* Name of vertex array object used when rendering the scene cubes. */

/* View and Projection configuration. */
/* We use different projection matrices for both passes. */
Matrix cameraProjectionMatrix;
Matrix lightProjectionMatrix;

const Vec3f cameraPosition   = {0.0f, 0.0f, 20.0f}; /* Array holding position of camera. */ 
Vec3f       lookAtPoint      = {0.0f, 0.0f,  0.0f}; /* Coordinates of a point the camera should look at (from light point of view). */
Matrix      viewMatrixForShadowMapPass;             /* Matrix used for translating geometry relative to light position. This is used for shadow map rendering pass. */

/** 
 * \brief Initialize structure data. 
 * \return false if an error appeared, true otherwise.
 */
bool initializeStructureData(void)
{
    /* Set up window properties. */
    window.height = 600;
    window.width  = 800;

    /* Set up cube properties. */
    cube.coordinates                        = NULL;
    cube.numberOfElementsInCoordinatesArray = 0;
    cube.normals                            = NULL;
    cube.numberOfElementsInNormalsArray     = 0;
    cube.numberOfElementsInPositionArray    = 2 * 4;   /* There are 2 cubes and 4 coordinates describing position (x, y, z, w). */
    cube.scalingFactor                      = 2.0f;

    /* Allocate memory for array holding position of cubes. */
    cube.position = (float*) malloc (cube.numberOfElementsInPositionArray * sizeof(float));
    if (cube.position == NULL)
    {
        LOGE("Could not allocate memory for cube position array.");
        return false;
    }

    /* Each scene cube is placed on the same altitude described by this value. */
    const float cubesYPosition = -3.0f;

    /* Array holding position of cubes. Y coordinate value is the same for both cubes - they are lying on the same surface. */
    /* First cube. */
    cube.position[0] = -3.0f;           /* x */
    cube.position[1] =  cubesYPosition; /* y */
    cube.position[2] =  5.0f;           /* z */
    cube.position[3] =  1.0f;           /* w */
    /* Second cube. */
    cube.position[4] =  5.0f;           /* x */
    cube.position[5] =  cubesYPosition; /* y */
    cube.position[6] =  3.0f;           /* z */
    cube.position[7] =  1.0f;           /* w */

    /* Set up plane properties. */
    plane.coordinates                        = NULL;
    plane.normals                            = NULL;
    plane.numberOfElementsInCoordinatesArray = 0;
    plane.numberOfElementsInNormalsArray     = 0;
    plane.numberOfElementsInPositionArray    = 3; /* There are 3 coordinates describing position (x, y, z). */
    plane.scalingFactor                      = 10.0; /* plane.scalingFactor * 2 indicates size of square's side. */

    plane.position = (float*) malloc (plane.numberOfElementsInPositionArray * sizeof(float));
    if (plane.position == NULL)
    {
        LOGE("Could not allocate memory for plane position array.");
        return false;
    }

    /* Array holding position of plane. Y coordinate value is calculated so that cubes are sitting on the plane. */
    plane.position[0] = 0.0f;
    plane.position[1] = cubesYPosition - cube.scalingFactor;
    plane.position[2] = 0.0f;

    /* Set up light representation properties. */
    lightRepresentation.coordinates                        = NULL;
    lightRepresentation.numberOfElementsInCoordinatesArray = 0;
    lightRepresentation.scalingFactor                      = 0.3f; /* lightRepresentation.scalingFactor * 2 indicates size of cube's side. */

    /* Set up shadow map properties. */
    shadowMap.framebufferObjectName = 0;
    shadowMap.size = 1024;
    shadowMap.textureName = 0;

    /* We use different projection matrices for both passes. */
    cameraProjectionMatrix = Matrix::matrixPerspective(degreesToRadians(60.0f), float(window.width) / float(window.height), 1.0f, 50.0f);
    lightProjectionMatrix = Matrix::matrixPerspective(degreesToRadians(90.0f),  1.0f, 1.0f, 50.0f);

    return true;
}

/**
 * \brief Creates GL objects. 
 */
void createObjects(void)
{   
    /* Generate buffer objects. */
    GLuint bufferObjectIds [6] = {0};
    GL_CHECK(glGenBuffers(6, bufferObjectIds));

    /* 
     * Store buffer object names in global variables.
     * The variables have more friendly names, so that using them is easier.
     */
    cubeCoordinatesBufferObjectId                = bufferObjectIds[0];
    lightRepresentationCoordinatesBufferObjectId = bufferObjectIds[1];
    cubeNormalsBufferObjectId                    = bufferObjectIds[2];
    planeCoordinatesBufferObjectId               = bufferObjectIds[3];
    planeNormalsBufferObjectId                   = bufferObjectIds[4];
    uniformBlockDataBufferObjectId               = bufferObjectIds[5];

    /* Generate vertex array objects. */
    GLuint vertexArrayObjectsNames [3] = {0};
    GL_CHECK(glGenVertexArrays(3, vertexArrayObjectsNames));

    /*
     * Store vertex array object names in global variables.
     * The variables have more friendly names, so that using them is easier.
     */
    cubesVertexArrayObjectId                          = vertexArrayObjectsNames[0];
    lightRepresentationCoordinatesVertexArrayObjectId = vertexArrayObjectsNames[1];
    planeVertexArrayObjectId                          = vertexArrayObjectsNames[2];

    /* Generate and configure shadow map texture to hold depth values. */
    GL_CHECK(glGenTextures(1, &shadowMap.textureName));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, shadowMap.textureName));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMap.size, shadowMap.size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));                  
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));

    /* Attach texture to depth attachment point of a framebuffer object. */
    GL_CHECK(glGenFramebuffers(1, &shadowMap.framebufferObjectName));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.framebufferObjectName));
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.textureName, 0));
}

/** 
 * \brief Deletes all created GL objects. 
 */
void deleteObjects(void)
{
    /* Delete buffers. */
    GL_CHECK(glDeleteBuffers(1, &cubeCoordinatesBufferObjectId));
    GL_CHECK(glDeleteBuffers(1, &cubeNormalsBufferObjectId));
    GL_CHECK(glDeleteBuffers(1, &lightRepresentationCoordinatesBufferObjectId));
    GL_CHECK(glDeleteBuffers(1, &planeCoordinatesBufferObjectId));
    GL_CHECK(glDeleteBuffers(1, &planeNormalsBufferObjectId));
    GL_CHECK(glDeleteBuffers(1, &uniformBlockDataBufferObjectId));

    /* Delete framebuffer object. */
    GL_CHECK(glDeleteFramebuffers(1, &shadowMap.framebufferObjectName));
    
    /* Delete texture. */
    GL_CHECK(glDeleteTextures(1, &shadowMap.textureName));

    /* Delete vertex arrays. */
    GL_CHECK(glDeleteVertexArrays(1, &cubesVertexArrayObjectId));
    GL_CHECK(glDeleteVertexArrays(1, &lightRepresentationCoordinatesVertexArrayObjectId));
    GL_CHECK(glDeleteVertexArrays(1, &planeVertexArrayObjectId));
}

/** 
 * brief Cleans up. Frees all allocated memory. 
 */
void deallocateMemory(void)
{
    /* If triangular representation of a cube was created successfully, make sure memory is deallocated. */ 
    if (cube.coordinates != NULL)
    {
        free (cube.coordinates);
        cube.coordinates = NULL;
    }

    /* If representation of a cube's normal was created successfully, make sure memory is deallocated. */ 
    if (cube.normals != NULL)
    {
        free (cube.normals);
        cube.normals = NULL;
    }

    /* If triangular representation of a cube simulating a light source was created successfully, make sure memory is deallocated. */ 
    if (lightRepresentation.coordinates != NULL)
    {
        free (lightRepresentation.coordinates);
        lightRepresentation.coordinates = NULL;
    }

    /* If triangular representation of a plane was created successfully, make sure memory is deallocated. */ 
    if (plane.coordinates != NULL)
    {
        free (plane.coordinates);
        plane.coordinates = NULL;
    }
    
    /* If representation of a plane's normal was created successfully, make sure memory is deallocated. */ 
    if (plane.normals != NULL)
    {
        free (plane.normals);
        plane.normals = NULL;
    }

    /* If position of plane was created successfully, make sure memory is deallocated. */
    if (plane.position != NULL)
    {
        free (plane.position);
        plane.position = NULL;
    }

    /* If position of cube was created successfully, make sure memory is deallocated. */
    if (cube.position != NULL)
    {
        free (cube.position);
        cube.position = NULL;
    }
}

/** 
 * \brief Calculates depth values written to shadow map texture.
 *
 * Fills viewMatrixForShadowMapPass matrix with data. That matrix is then used to set the viewMatrix in the shader from the light's point of view.   
 */
void calculateLookAtMatrix(void)
{
    Vec3f upVector = {0.0f, 1.0f, 0.0f};

    /* 
     * Position of light is very close to the scene. From this point of view, we are not able to see the whole model.
     * We have to see whole model to calculate depth values in a final pass. To do that, we have to move camera away from the model. 
     */
    Vec3f cameraTranslation = {0.0f, 0.0f, -20.0f};

    /* Get lookAt matrix from the light's point of view, directed at the center of a plane. Store result in viewMatrixForShadowMapPass. */
    viewMatrixForShadowMapPass = Matrix::matrixCameraLookAt(light.position, lookAtPoint, upVector);
    viewMatrixForShadowMapPass = Matrix::createTranslation(cameraTranslation.x, cameraTranslation.y, cameraTranslation.z) * viewMatrixForShadowMapPass;
}

/** 
 * \brief Initialize data used for drawing the scene. 
 * 
 * Retrieve the coordinates of the triangles that make up the cubes and the plane and their normal vectors.
 *
 * \return false if an error appeared, true otherwise.
 */
bool createDataForObjectsToBeDrawn(void)
{
    /* Get triangular representation of the scene cube. Store the data in the cubeCoordinates array. */
    CubeModel::getTriangleRepresentation(cube.scalingFactor, &cube.numberOfElementsInCoordinatesArray, &cube.coordinates);
    if (cube.coordinates == NULL)
    {
        return false;
    }

    /* Calculate normal vectors for the scene cube created above. */
    CubeModel::getNormals(&cube.numberOfElementsInNormalsArray, &cube.normals);
    if (cube.normals == NULL)
    {
        return false;
    }

    /* Get triangular representation of the light cube. Store the data in the lightRepresentationCoordinates array. */
    CubeModel::getTriangleRepresentation(lightRepresentation.scalingFactor, &lightRepresentation.numberOfElementsInCoordinatesArray, &lightRepresentation.coordinates);
    if (lightRepresentation.coordinates == NULL)
    {
        return false;
    }

    /* Get triangular representation of a square to draw plane in XZ space. Store the data in the planeCoordinates array. */
    PlaneModel::getTriangleRepresentation(&plane.numberOfElementsInCoordinatesArray, &plane.coordinates);
    if (plane.coordinates == NULL)
    {
        return false;
    }

    /* Scale the plane up to the correct size. */
    Matrix scaling = Matrix::createScaling(plane.scalingFactor, plane.scalingFactor,plane.scalingFactor);
    PlaneModel::transform(scaling, plane.numberOfElementsInCoordinatesArray, &plane.coordinates);

    /* Calculate normal vectors for the plane. Store the data in the planeNormals array. */ 
    PlaneModel::getNormals(&plane.numberOfElementsInNormalsArray, &plane.normals);
    if (plane.normals == NULL)
    {
        return false;
    }

    return true;
}

/** 
 * \brief Initialize the data used for rendering. 
 *
 * Store data in buffer objects so that it can be used during draw calls.
 *
 * \return false if an error appeared, true otherwise.
 */
bool initializeData()
{
    /* Create all needed GL objects. */ 
    createObjects();

    /* Create all data needed to draw scene. */
    if (createDataForObjectsToBeDrawn())
    { 
        /* Fill buffer objects with data. */
        /* Buffer holding coordinates of triangles which make up the scene cubes. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cubeCoordinatesBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cube.numberOfElementsInCoordinatesArray * sizeof(float), cube.coordinates, GL_STATIC_DRAW));

        /* Buffer holding coordinates of normal vectors for each vertex of the scene cubes. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cubeNormalsBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cube.numberOfElementsInNormalsArray * sizeof(float), cube.normals, GL_STATIC_DRAW));

        /*
         * Buffer holding the positions coordinates of the scene cubes. 
         * Data is then used by a uniform buffer object to set the position of each cube drawn using glDrawArraysInstanced(). 
         */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, uniformBlockDataBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, cube.numberOfElementsInPositionArray * sizeof(float), cube.position, GL_STATIC_DRAW));

        /* Buffer holding coordinates of triangles which make up the plane. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, planeCoordinatesBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, plane.numberOfElementsInCoordinatesArray * sizeof(float), plane.coordinates, GL_STATIC_DRAW));

        /* Buffer holding coordinates of the plane's normal vectors. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, planeNormalsBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, plane.numberOfElementsInNormalsArray * sizeof(float), plane.normals, GL_STATIC_DRAW));

        /* Buffer holding coordinates of the light cube. */
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, lightRepresentationCoordinatesBufferObjectId));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, lightRepresentation.numberOfElementsInCoordinatesArray * sizeof(float), lightRepresentation.coordinates, GL_STATIC_DRAW));
    }
    else
    {
        LOGE("Could not initialize data used for rendering.");
        return false;
    }

    return true;
}

/** 
 * \brief Return locations of the uniforms used for spot light and shadow calculations. 
 * \return false if an error appeared, true otherwise.
 */
bool getLightAndShadowUniformLocations(void)
{
    /* Get the uniforms' locations. */
    cubesAndPlaneProgram.lightDirectionLocation = GL_CHECK(glGetUniformLocation(cubesAndPlaneProgram.programId, "lightDirection"));
    cubesAndPlaneProgram.lightPositionLocation  = GL_CHECK(glGetUniformLocation(cubesAndPlaneProgram.programId, "lightPosition"));
    cubesAndPlaneProgram.shadowMapLocation      = GL_CHECK(glGetUniformLocation(cubesAndPlaneProgram.programId, "shadowMap"));

    /* Check if the uniform locations were found in shaders. */
    if (cubesAndPlaneProgram.lightDirectionLocation == -1)
    {
        LOGE("Could not retrieve uniform location: lightDirectionLocation");
        return false;
    }

    if (cubesAndPlaneProgram.lightPositionLocation == -1)
    {
        LOGE("Could not retrieve uniform location: lightPositionLocation");
        return false;
    }

    if(cubesAndPlaneProgram.shadowMapLocation == -1)
    {
        LOGE("Could not retrieve uniform location: shadowMapLocation");
        return false;
    }

    return true;
}

/** 
 * \brief Create, compile and attach vertex and fragment shaders to previously created program object, link and set program object to be used. 
 * \param[in] programId Previously created program object name to be used.
 * \param[in] fragmentShaderFileName File name of a fragment shader to be attached to the  program object.
 * \param[in] vertexShaderFileName File name of a vertex shader to be attached to the program object.
 */
void setUpAndUseProgramObject(GLint programId, string fragmentShaderFileName, string vertexShaderFileName)
{
    /* Initialize vertex shader. */
    GLuint vertexShaderId = 0;
    string vertexShaderPath = resourceDirectory + vertexShaderFileName;
    Shader::processShader(&vertexShaderId,   vertexShaderPath.c_str(),   GL_VERTEX_SHADER);

    /* Initialize fragment shader. */
    GLuint fragmentShaderId = 0;
    string fragmentShaderPath = resourceDirectory + fragmentShaderFileName;
    Shader::processShader(&fragmentShaderId, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    /* Attach the vertex and fragment shaders to the rendering program. */
    GL_CHECK(glAttachShader(programId, vertexShaderId));
    GL_CHECK(glAttachShader(programId, fragmentShaderId));

    /* Link and use the rendering program object. */
    GL_CHECK(glLinkProgram(programId));
    GL_CHECK(glUseProgram (programId));
}

/**
 * \brief Create a program that will be used to convert vertices into eye-space and then rasterize cubes and plane.
 * \return false if an error appeared, true otherwise.
 */
bool setupCubesAndPlaneProgram(void)
{
    /* Create program object. */
    cubesAndPlaneProgram.programId = GL_CHECK(glCreateProgram());

    /* Call function to prepare program object to be used. */
    setUpAndUseProgramObject(cubesAndPlaneProgram.programId, "lighting_fragment_shader_source.frag", "model_vertex.vert");

    /*
     * Get uniform and attribute locations from current program. 
     * Values for those uniforms and attributes will be set later.
     */
    cubesAndPlaneProgram.positionAttributeLocation   = GL_CHECK(glGetAttribLocation  (cubesAndPlaneProgram.programId, "attributePosition"));   /* Attribute that is fed with the vertices of triangles that make up geometry (cube or plane). */
    cubesAndPlaneProgram.normalsAttributeLocation    = GL_CHECK(glGetAttribLocation  (cubesAndPlaneProgram.programId, "attributeNormals"));    /* Attribute that is fed with the normal vectors for geometry (cube or plane). */
    cubesAndPlaneProgram.isCameraPointOfViewLocation = GL_CHECK(glGetUniformLocation (cubesAndPlaneProgram.programId, "isCameraPointOfView")); /* Uniform holding boolean value that is used for setting camera or light point of view.
                                                                                                                                                * If true: the camera's point of view is used for drawing scene with light and shadows. 
                                                                                                                                                * If false: the light's point of view-used for drawing scene to calculate depth values (create shadow map).
                                                                                                                                                * Vertex shader is used for both calculating depth values to create shadow map and for drawing the scene.
                                                                                                                                                */ 
    cubesAndPlaneProgram.shouldRenderPlaneLocation   = GL_CHECK(glGetUniformLocation (cubesAndPlaneProgram.programId, "shouldRenderPlane"));   /* Uniform holding a boolean value indicating which geometry is being drawn: cube or plane. */
    cubesAndPlaneProgram.lightViewMatrixLocation     = GL_CHECK(glGetUniformLocation (cubesAndPlaneProgram.programId, "lightViewMatrix"));     /* Uniform holding the calculated view matrix used to render the scene from the light's point of view. */
    cubesAndPlaneProgram.colorOfGeometryLocation     = GL_CHECK(glGetUniformLocation (cubesAndPlaneProgram.programId, "colorOfGeometry"));     /* Uniform holding the color of a geometry. */

    /* 
     * Get uniform locations and uniform block index (index of "cubesDataUniformBlock" uniform block) for the current program. 
     * Values for those uniforms will be set now (only once, because they are constant).
     */
    GLuint uniformBlockIndex              = GL_CHECK(glGetUniformBlockIndex(cubesAndPlaneProgram.programId, "cubesDataUniformBlock"));    /* Uniform block that holds the position the scene cubes. */
    GLuint planePositionLocation          = GL_CHECK(glGetUniformLocation  (cubesAndPlaneProgram.programId, "planePosition"));            /* Uniform holding the position of plane. */
    GLuint cameraPositionLocation         = GL_CHECK(glGetUniformLocation  (cubesAndPlaneProgram.programId, "cameraPosition"));           /* Uniform holding the position of camera (which is used to render the scene from the camera's point of view). */
    GLuint cameraProjectionMatrixLocation = GL_CHECK(glGetUniformLocation  (cubesAndPlaneProgram.programId, "cameraProjectionMatrix"));   /* Uniform holding the projection matrix (which is used to render the scene from the camera's point of view). */
    GLuint lightProjectionMatrixLocation  = GL_CHECK(glGetUniformLocation  (cubesAndPlaneProgram.programId, "lightProjectionMatrix"));    /* Uniform holding the projection matrix (which is used to render the scene from the light's point of view). */

    /* Check if uniform and attribute locations were found in the shaders. */
    if (cubesAndPlaneProgram.positionAttributeLocation == -1)
    {
        LOGE("Could not retrieve attribute location: positionAttributeLocation.");
        return false;
    }

    if (cubesAndPlaneProgram.normalsAttributeLocation == -1)
    {
        LOGE("Could not retrieve attribute location: normalsAttributeLocation.");
        return false;
    }

    if (cubesAndPlaneProgram.isCameraPointOfViewLocation == -1)
    {
        LOGE("Could not retrieve uniform location: isCameraPointOfViewLocation.");
        return false;
    }

    if (cubesAndPlaneProgram.shouldRenderPlaneLocation == -1)
    {
        LOGE("Could not retrieve uniform location: shouldRenderPlaneLocation.");
        return false;
    }

    if (cubesAndPlaneProgram.lightViewMatrixLocation == -1)
    {
        LOGE("Could not retrieve uniform location: lightViewMatrixLocation.");
        return false;
    }

    if (cubesAndPlaneProgram.colorOfGeometryLocation == -1)
    {
        LOGE("Could not retrieve uniform location: colorOfGeometryLocation.");
        return false;
    }

    /* Check if uniform and attribute locations were found in the shaders. Set values for them. */
    if (uniformBlockIndex == -1)
    {
        LOGE("Could not retrieve uniform block index: uniformBlockIndex");
        return false;
    }
    else
    {
        /* 
         * Set the binding point for the uniform block. The uniform block holds the position of the scene cubes. 
         * The binding point is then used by glBindBufferBase() to bind buffer object (for GL_UNIFORM_BUFFER).
         */
        GL_CHECK(glUniformBlockBinding(cubesAndPlaneProgram.programId, uniformBlockIndex, 0));
    }

    if (planePositionLocation == -1)
    {
        LOGE("Could not retrieve uniform location: planePositionLocation");
        return false;
    }
    else
    {
        GL_CHECK(glUniform3fv(planePositionLocation, 1, plane.position));
    }

    if (cameraPositionLocation == -1)
    {
        LOGE("Could not retrieve uniform location: cameraPositionLocation");
        return false;
    }
    else
    {
        GL_CHECK(glUniform3fv(cameraPositionLocation, 1, (float*)&cameraPosition));
    }

    if (cameraProjectionMatrixLocation == -1)
    {
        LOGE("Could not retrieve uniform location: cameraProjectionMatrixLocation");
        return false;
    }
    else
    {
        GL_CHECK(glUniformMatrix4fv(cameraProjectionMatrixLocation, 1, GL_FALSE, cameraProjectionMatrix.getAsArray()));
    }

    if (lightProjectionMatrixLocation == -1)
    {
        LOGE("Could not retrieve uniform location: lightProjectionMatrixLocation");
        return false;
    }
    else
    {
        GL_CHECK(glUniformMatrix4fv(lightProjectionMatrixLocation, 1, GL_FALSE, lightProjectionMatrix.getAsArray()));
    }

    /* Get the location of uniforms describing the spot light and shadow map for thecurrent program object. */
    if (!getLightAndShadowUniformLocations())
    {
        return false;
    }

    return true;
}

/** 
 * \brief Create a program that will be used to rasterize the geometry of light cube.
 * \return false if an error appeared, true otherwise.
 */
bool setuplightRepresentationProgram(void)
{
    /* Create program object. */
    lightRepresentationProgram.programId = GL_CHECK(glCreateProgram());

    /* Call function to prepare the program object to be used. */
    setUpAndUseProgramObject(lightRepresentationProgram.programId, "cube_light_fragment_shader_source.frag", "cube_light_vertex_shader_source.vert");

    /* 
     * Get the uniform locations for the current program. 
     * Values for those uniforms will be set later.
     */
    lightRepresentationProgram.positionLocation  = GL_CHECK(glGetUniformLocation(lightRepresentationProgram.programId, "cubePosition"));   /* Uniform holding position of a cube (used to calculate translation matrix). */
    
    /* 
     * Get uniform and attribute locations for current program. 
     * Values for those uniforms and attribute will be set now (only once, because their are constant).
     */
    GLuint positionLocation         = GL_CHECK(glGetAttribLocation (lightRepresentationProgram.programId, "attributePosition")); /* Attribute holding coordinates of the triangles that make up the light cube. */
    GLuint projectionMatrixLocation = GL_CHECK(glGetUniformLocation(lightRepresentationProgram.programId, "projectionMatrix"));  /* 
                                                                                                                                  * Uniform holding projection matrix (cube is drawn only from camera point of view, 
                                                                                                                                  * not present in shadow map calculation). Camera is static, so the value for this 
                                                                                                                                  * uniform can be set just once.
                                                                                                                                  */
    GLuint cameraPositionLocation    = GL_CHECK(glGetUniformLocation(lightRepresentationProgram.programId, "cameraPosition"));   /* Uniform holding the position of camera. */

    /* Check if the uniform and attribute locations were found in the shaders. Set their values. */
    if (positionLocation != -1)
    {
        GL_CHECK(glBindVertexArray        (lightRepresentationCoordinatesVertexArrayObjectId));
        GL_CHECK(glBindBuffer             (GL_ARRAY_BUFFER, lightRepresentationCoordinatesBufferObjectId));
        GL_CHECK(glEnableVertexAttribArray(positionLocation));
        GL_CHECK(glVertexAttribPointer    (positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));
    }
    else
    {
        LOGE("Could not retrieve attribute location: positionLocation");
        return false;
    }

    if (projectionMatrixLocation != -1)
    {
        GL_CHECK(glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, cameraProjectionMatrix.getAsArray()));
    }
    else
    {
        LOGE("Could not retrieve uniform location: projectionMatrixLocation");
        return false;
    }

    if (cameraPositionLocation != -1)
    {
        GL_CHECK(glUniform3fv(cameraPositionLocation, 1, (float*)&cameraPosition));
    }
    else
    {
        LOGE("Could not retrieve uniform location: cameraPositionLocation");
        return false;
    }

    /* Check if the uniform locations were found in the shaders. */
    if (lightRepresentationProgram.positionLocation == -1)
    {
        LOGE("Could not retrieve uniform location: lightRepresentationPositionLocation");
        return false;
    }

    return true;
}

/** 
 * \brief Create all programs needed to draw the shadow-mapped scene.
 * \return false if an error appeared, true otherwise.
 */
bool setupPrograms()
{
    /* Set up the program for rendering the scene. */
    if (!setupCubesAndPlaneProgram())
    {
        return false;
    }

    /* Set up thr program for rendering the light cube. */
    if (!setuplightRepresentationProgram())
    {
        return false;
    }
    
    return true;
}

/**
 * \brief Draw geometry. 
 * \param[in] hasShadowMapBeenCalculated If true, will draw the whole scene from the camera's point of view.
 *                                       If false, will draw only the scene cubes and the plane from the light's point of view.
 */
void draw(bool hasShadowMapBeenCalculated)
{
    /* Arrays holding RGBA values of the scene cubes and the plane. */
    const float cubesColor[] = {0.8f, 0.1f, 0.2f, 0.6f};
    const float planeColor[] = {0.2f, 0.4f, 0.8f, 0.6f};

    /* Clear the depth and color buffers. */
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

    /* Let's focus on drawing the model. */
    GL_CHECK(glUseProgram(cubesAndPlaneProgram.programId));

    /* Set active texture. Shadow map texture will be passed to shader. */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture  (GL_TEXTURE_2D, shadowMap.textureName));

    /* Values are the same for both passes (creating shadow map and drawing the scene) - can be set just once. */
    if (!hasShadowMapBeenCalculated)
    {
        /* Set light and shadow values for uniforms. */
        GL_CHECK(glUniform3fv(cubesAndPlaneProgram.lightDirectionLocation, 1, (float*)&light.direction));
        GL_CHECK(glUniform3fv(cubesAndPlaneProgram.lightPositionLocation,  1, (float*)&light.position));
        GL_CHECK(glUniform1i (cubesAndPlaneProgram.shadowMapLocation,      0));
    }

    /*  Set uniform value indicating point of view: camera or light. */
    GL_CHECK(glUniform1i(cubesAndPlaneProgram.isCameraPointOfViewLocation, hasShadowMapBeenCalculated));

    /* If the light's point of view, set calculated view matrix. (View is static from camera point of view - no need to change that value). */
    if (!hasShadowMapBeenCalculated)
    {
        GL_CHECK(glUniformMatrix4fv(cubesAndPlaneProgram.lightViewMatrixLocation, 1, GL_FALSE, viewMatrixForShadowMapPass.getAsArray()));
    }

    /* Draw cubes. */
    /* Set uniform value indicating which geometry to draw: cubes or plane. Set to draw cubes. */
    GL_CHECK(glUniform1i(cubesAndPlaneProgram.shouldRenderPlaneLocation, false));
    /* Set uniform value indicating the color of geometry (set the color for cubes). */
    GL_CHECK(glUniform4f(cubesAndPlaneProgram.colorOfGeometryLocation, cubesColor[0], cubesColor[1], cubesColor[2], cubesColor[3]));

    GL_CHECK(glBindVertexArray(cubesVertexArrayObjectId));
    /* Set values for cubes' normal vectors. */
    GL_CHECK(glBindBuffer             (GL_ARRAY_BUFFER, cubeNormalsBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(cubesAndPlaneProgram.normalsAttributeLocation));
    GL_CHECK(glVertexAttribPointer    (cubesAndPlaneProgram.normalsAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));
    /* Set values for the cubes' coordinates. */
    GL_CHECK(glBindBuffer             (GL_ARRAY_BUFFER, cubeCoordinatesBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(cubesAndPlaneProgram.positionAttributeLocation));
    GL_CHECK(glVertexAttribPointer    (cubesAndPlaneProgram.positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

    /* Bind buffer with uniform data. Used to set the locations of the cubes. */
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBlockDataBufferObjectId));

    /* Draw two cubes. */
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLES, 0, cube.numberOfElementsInCoordinatesArray, 2));

    /* Draw plane. */
    /* Set uniform value indicating which shape to draw: cubes or plane. Now set to draw plane. */
    GL_CHECK(glUniform1f(cubesAndPlaneProgram.shouldRenderPlaneLocation, true));
    /* Set uniform value indicating color of the geometry (set color for plane). */
    GL_CHECK(glUniform4f(cubesAndPlaneProgram.colorOfGeometryLocation, planeColor[0], planeColor[1], planeColor[2], planeColor[3]));

    GL_CHECK(glBindVertexArray(planeVertexArrayObjectId));
    /* Set values for plane's normal vectors. */
    GL_CHECK(glBindBuffer             (GL_ARRAY_BUFFER, planeNormalsBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(cubesAndPlaneProgram.normalsAttributeLocation));
    GL_CHECK(glVertexAttribPointer    (cubesAndPlaneProgram.normalsAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));
    /* Set values for plane's coordinates. */
    GL_CHECK(glBindBuffer             (GL_ARRAY_BUFFER, planeCoordinatesBufferObjectId));
    GL_CHECK(glEnableVertexAttribArray(cubesAndPlaneProgram.positionAttributeLocation));
    GL_CHECK(glVertexAttribPointer    (cubesAndPlaneProgram.positionAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));
    
    /* Draw plane. */
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, plane.numberOfElementsInCoordinatesArray));
    
    if (hasShadowMapBeenCalculated)
    {
        /* Let's focus on drawing the light cube. */
        GL_CHECK(glUseProgram(lightRepresentationProgram.programId));
    
        /* Set the values for the uniforms (values used for translation and rotation of a cube). */
        GL_CHECK(glUniform3fv(lightRepresentationProgram.positionLocation, 1, (float*)&light.position));
        
        GL_CHECK(glBindVertexArray(lightRepresentationCoordinatesVertexArrayObjectId));
    
        /* Draw cube. */
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, lightRepresentation.numberOfElementsInCoordinatesArray));
    }
}

/**
 * \brief Draw the scene from the light's point of view to calculate depth values (calculated values are held in shadow map texture). 
 */
void createShadowMap(void)
{
    /* 
     * Bind framebuffer object.
     * There is a texture attached to depth attachment point for this framebuffer object.
     * By using this framebuffer object, calculated depth values are stored in the texture.
     */
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.framebufferObjectName));
    
    /* Set the view port to size of shadow map texture. */
    GL_CHECK(glViewport(0, 0, shadowMap.size, shadowMap.size));

    /* Set back face to be culled */
    GL_CHECK(glEnable  (GL_CULL_FACE));
    GL_CHECK(glCullFace(GL_BACK));
    
    /* Enable depth test to do comparison of depth values. */
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    /* Disable writing of each frame buffer color component. */
    GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    /* Update the lookAt matrix that we use for view matrix (to look at scene from the light's point of view). */
    calculateLookAtMatrix();

    /*
     * Draw the scene. 
     * Value of parameter indicates that shadow map should now be calculated.
     * Only the plane and scene cubes should be drawn (without cube representing light source). 
     * Scene should be rendered from the light's point of view.
     * Enable a polygon offset to ensure eliminate z-fighting in the resulting shadow map.
     */
    glEnable(GL_POLYGON_OFFSET_FILL);
    draw(false);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

/**
 * \brief Draw the lit shadow-mapped scene from the camera's point of view. 
 */
void drawScene(void)
{
    /* Use the default framebuffer object. */
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    /* Set the view port to the size of the window. */
    GL_CHECK(glViewport(0, 0, window.width, window.height));

    /* Disable culling. */
    GL_CHECK(glDisable(GL_CULL_FACE));

    /* Enable writing of each frame buffer color component. */
    GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    /* 
     * Draw the scene. 
     * Value of parameter indicates that shadow map has been already calculated and the normal scene should now be drawn.
     * All elements should be drawn (scene cubes, plane, and the light cube).
     * Scene should be rendered from the camera's point of view.
     */
    draw(true);
}

/**
 * \brief Function called to render the new frame into the back buffer. 
 */
void renderFrame(void)
{
    /* Time used to set light direction and position. */
    const float time = timer.getTime();

    /* Set position of the light. The light is moving on a circular curve (radius of a circle is now equal to 5).*/
    const float radius = 5.0f;

    light.position.x =  radius * sinf(time / 2.0f);
    light.position.y =  2.0f;
    light.position.z =  radius * cosf(time / 2.0f);

    /* Direction of light. */
    light.direction.x = lookAtPoint.x - light.position.x;
    light.direction.y = lookAtPoint.y - light.position.y;
    light.direction.z = lookAtPoint.z - light.position.z;

    /* Normalize the light direction vector. */
    light.direction.normalize();

    /* Clear the contents of back buffer. */
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Fill the shadow map texture with the calculated depth values. */
    createShadowMap();

    /* Draw the scene consisting of all objects, light and shadow. Use created shadow map to display shadows. */
    drawScene();
}

int main(int argc, char **argv)
{
    /* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if (platform != NULL)
    {
        if (initializeStructureData())
        {
            /* Initialize the windowing system. */
            platform->createWindow(window.width, window.height);

            /* Initialize EGL. */
            EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
            EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

            /* Initialize data used for rendering. */
            if (initializeData())
            {
                /* Create program. */
                if (setupPrograms())
                {            
                    /* Set the Polygon offset, used when rendering the into the shadow map to eliminate z-fighting in the shadows. */
                    glPolygonOffset(1.0, 0.0);

                    /* Start counting time. */
                    timer.reset();

                    /* Rendering loop to draw the scene starts here. */
                    bool shouldContinueTheLoop = true;

                    while (shouldContinueTheLoop)
                    {
                        /* If something happened to the window, leave the loop. */
                        if (platform->checkWindow() != Platform::WINDOW_IDLE)
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

                /* Delete all created GL objects. */
                deleteObjects();
            }
            else
            {
                LOGE("Could not initialize data used for rendering.");
            }

            /* Deallocate memory. */
            deallocateMemory();

            /* Shut down EGL. */
            EGLRuntime::terminateEGL();

            /* Shut down windowing system. */
            platform->destroyWindow();

            /* Shut down the Platform object. */
            delete platform;
        }
        else
        {
            LOGE("Could not initialize needed data.");
        }
    }
    else
    {
        LOGE("Could not create platform.");
    }

    return 0;
}