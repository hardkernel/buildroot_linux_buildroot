/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef PROJECTED_LIGHTS_H
#define PROJECTED_LIGHTS_H

#include "VectorTypes.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace MaliSDK
{
    /** Field of view used for projection matrix calculations [in degrees] from camera point of view. */
    #define CAMERA_PERSPECTIVE_FOV_IN_DEGREES (60.0f)
    /** Name of a bmp file where colour texture image is stored. */
    #define COLOR_TEXTURE_NAME ("assets/mali.bmp")
    /** Define a translation in x and Z space of a colour texture. */
    #define COLOR_TEXTURE_TRANSLATION (15.0f)
    /** Scaling factor used to set-up a cube geometry (indicates the size of the cube). */
    #define CUBE_SCALING_FACTOR (2.0f)
    /** Field of view used for projection matrix calculations [in degrees] from light point of view. */
    #define LIGHT_PERSPECTIVE_FOV_IN_DEGREES (90.0f)
    /** Value of the far plane used to set-up a projection view. */
    #define FAR_PLANE (50.0f)
    /* Scaling factor used to set-up a cube geometry (indicates the size of the cube representing the spot light source). */
    #define LIGHT_SOURCE_SCALING_FACTOR (0.3f)
    /** Position of a model (plane and cube) on Y axis. */
    #define MODEL_Y_POSITION (-3.0f)
    /** Angle (in degrees) of a model rotation around Y axis. */
    #define MODEL_Y_ROTATION_ANGLE_IN_DEGREES (60.0f)
    /** Value of the near plane used to set-up a projection view. */
    #define NEAR_PLANE (1.0f)
    /** Scaling factor used to set-up a plane geometry (indicates the size of the plane). */
    #define PLANE_SCALING_FACTOR (10.0f)
    /* [Declare shadow map texture resolution] */
    /** Hight of a shadow map texture. */
    #define SHADOW_MAP_TEXTURE_HEIGHT (2400)
    /** Width of a shadow map texture. */
    #define SHADOW_MAP_TEXTURE_WIDTH (2400)
    /* [Declare shadow map texture resolution] */
    /** Value of the projected light angle (in degrees) */
    #define SPOT_LIGHT_ANGLE_IN_DEGREES (25.0f)
    /* Radius that will be used to calculate new direction of a spot light. */
    #define SPOT_LIGHT_TRANSLATION_RADIUS (3.0f)
    /** Texture unit that will be used for colour texture binding purposes.*/
    #define TEXTURE_UNIT_FOR_COLOR_TEXTURE (0)
    /** Texture unit that will be used for shadow map texture binding purposes.*/
    #define TEXTURE_UNIT_FOR_SHADOW_MAP_TEXTURE (1)
    /* [Declare window resolution] */
    /** Window resolution: height. */
    #define WINDOW_HEIGHT (600)
    /** Window resolution: width. */
    #define WINDOW_WIDTH (600)
    /* [Declare window resolution] */

    /** Structure holding properties of a directional light. */
    struct DirectionalLightProperties
    {
        GLfloat ambient;
        Vec3f   color;
        Vec3f   position;
    };

    /** Structure holding all data needed to set the geometry properties. */
    struct GeometryProperties
    {
        Vec4f  color;                              /* Colour of the geometry (assumption: geometry will have a solid colour). */
        float* coordinates;                        /* Array holding the geometry's coordinates. */
        float* normals;                            /* Array holding the geometry's normal vectors. */
        int    numberOfElementsInCoordinatesArray; /* Number of elements written to coordinates array. */
        int    numberOfElementsInNormalsArray;     /* Number of elements written to an array holding geometry's normal vectors. */
        Vec3f  position;                           /* Array holding position of the geometry in 3D space. */
    
        GeometryProperties()
        {
            coordinates                        = NULL;
            normals                            = NULL;
            numberOfElementsInCoordinatesArray = 0;
            numberOfElementsInNormalsArray     = 0;
        }
    };

    /** Structure holding IDs of generated program and shader objects.
      * It is assumed that shader objects will be attached to the program object. */
    struct ProgramAndShaderObjectIds
    {
        GLuint fragmentShaderObjectId;
        GLuint programObjectId;
        GLuint vertexShaderObjectId;

        ProgramAndShaderObjectIds()
        {
            fragmentShaderObjectId = 0;
            programObjectId        = 0;
            vertexShaderObjectId   = 0;
        }
    };

    /** Structure holding object IDs which are needed to render a geometry. */
    struct RenderGeometryObjects
    {
        GLuint coordinatesBufferObjectId;
        GLuint normalsBufferObjectId;
        GLuint vertexArrayObjectId;

        RenderGeometryObjects()
        {
            coordinatesBufferObjectId = 0;
            normalsBufferObjectId     = 0;
            vertexArrayObjectId       = 0;
        }
    };

    /** Structure holding object IDs which are needed to render the scene. */
    struct RenderSceneObjects
    {
        GLuint                colorTextureObjectId;
        GLuint                depthTextureObjectId;
        GLuint                framebufferObjectId;
        RenderGeometryObjects renderCube;
        RenderGeometryObjects renderPlane;

        RenderSceneObjects()
        {
            colorTextureObjectId = 0;
            depthTextureObjectId = 0;
            framebufferObjectId  = 0;
        }
    };

    /** Structure holding locations of attributes and uniforms for program object
      * responsible for rendering the scene. */
    struct RenderSceneProgramLocations
    {
        GLint attributeVertexCoordinates;
        GLint attributeVertexNormals;
        GLint uniformColorTexture;
        GLint uniformDirectionalLightAmbient;
        GLint uniformDirectionalLightColor;
        GLint uniformDirectionalLightPosition;
        GLint uniformGeometryColor;
        GLint uniformModelViewMatrix;
        GLint uniformModelViewProjectionMatrix;
        GLint uniformNormalMatrix;
        GLint uniformShadowMap;
        GLint uniformSpotLightColor;
        GLint uniformSpotLightCosAngle;
        GLint uniformSpotLightLookAtPointInEyeSpace;
        GLint uniformSpotLightPositionInEyeSpace;
        GLint uniformViewToColorTextureMatrix;
        GLint uniformViewToDepthTextureMatrix;

        RenderSceneProgramLocations()
        {
            attributeVertexCoordinates            = -1;
            attributeVertexNormals                = -1;
            uniformColorTexture                   = -1;
            uniformDirectionalLightAmbient        = -1;
            uniformDirectionalLightColor          = -1;
            uniformDirectionalLightPosition       = -1;
            uniformGeometryColor                  = -1;
            uniformModelViewMatrix                = -1;
            uniformModelViewProjectionMatrix      = -1;
            uniformNormalMatrix                   = -1;
            uniformShadowMap                      = -1;
            uniformSpotLightColor                 = -1;
            uniformSpotLightCosAngle              = -1;
            uniformSpotLightLookAtPointInEyeSpace = -1;
            uniformSpotLightPositionInEyeSpace    = -1;
            uniformViewToColorTextureMatrix       = -1;
            uniformViewToDepthTextureMatrix       = -1;
        }
    };

    /** Structure holding properties of a light source. */
    struct SpotLightProperties
    {
        Vec4f color;
        Vec3f position;
    };

    struct ModelViewProperties
    {
        Matrix modelMatrix;
        Matrix modelViewMatrix;
        Matrix modelViewProjectionMatrix;
        Matrix normalMatrix;
    };

    /** Structure holding information needed to set up the (perspective) view
    * from camera point of view. */
    struct CameraViewProperties
    {
        ModelViewProperties cubeViewProperties;
        Vec3f               lookAtPoint;
        ModelViewProperties planeViewProperties;
        Vec3f               position;
        Matrix              projectionMatrix;
        Vec4f               spotLightLookAtPointInEyeSpace;
        Vec4f               spotLightPositionInEyeSpace;
        Vec3f               upVector;
        Matrix              viewMatrix;
        Matrix              viewToColorTextureMatrix;
        Matrix              viewToDepthTextureMatrix;
    };

    /** Structure holding information needed to set up the (perspective) view
    * from spot light point of view. */
    struct SpotLightViewProperites
    {
        ModelViewProperties cubeViewProperties;
        Vec3f               lookAtPoint;
        ModelViewProperties planeViewProperties;
        Vec3f               position;
        Matrix              projectionMatrix;
        Vec3f               upVector;
        Matrix              viewMatrix;
    };
}
#endif /* PROJECTED_LIGHTS_H */