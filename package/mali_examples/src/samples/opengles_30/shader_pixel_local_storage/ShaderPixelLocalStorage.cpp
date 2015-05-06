/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * \file ShaderPixelLocalStorage.cpp
 * \brief This tutorial demonstrates how a tile buffer can be used to implement the deferred shading technique.
 */

#define GLES_VERSION 3

#include "VectorTypes.h"
#include "Platform.h"
#include "Shader.h"
#include "Timer.h"
#include "Matrix.h"

#include "GLES3/gl3.h"
#include "EGL/egl.h"

#include <vector>
#include <string>
#include "SphereModel.h"
#include "PlaneModel.h"
#include "CubeModel.h"

/* Define new constants from the GL_EXT_shader_pixel_local_storage extension. */
#ifndef GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT 0x8F63
#endif

#ifndef GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT      0x8F67
#endif

#ifndef GL_SHADER_PIXEL_LOCAL_STORAGE_EXT
#define GL_SHADER_PIXEL_LOCAL_STORAGE_EXT               0x8F64
#endif

using namespace MaliSDK;

/* General tutorial properties. */
unsigned int window_width                = 800;    /**< Window width resolution (pixels).                        */
unsigned int window_height               = 600;    /**< Window height resolution (pixels).                       */

/* Camera movement properties. */
const float  camera_distance             = 3.5f;    /**< Distance from camera to scene center.                   */
const float  camera_equator_angle_speed  = 1;       /**< Camera angle speed along equator (degrees per second).  */
const float  camera_latitude_angle_speed = 2;       /**< Camera angle speed along longitude (degrees per second).*/
const float  camera_latitude_min_angle   = 30;      /**< Camera minimum latitude (degrees).                      */
const float  camera_latitude_max_angle   = 60;      /**< Camera maximum latitude (degrees).                      */

/* Projection matrix properties. */
const float  frustum_fovy                = 45.0f;   /**< 45 degrees field of view in the y direction.             */
const float  frustum_z_near              = 1.0f;    /**< How close the viewer is to the near clipping plane.      */
const float  frustum_z_far               = 10.0f;   /**< How far the viewer is from the far clipping plane.       */

/* Sphere properties. */
const int    sphere_tessellation_level   = 64;      /**< Amount of latitudes and longitudes sphere sampled by.    */

/* Plane properties. */
const float  plane_color_r               = 0.1f;    /**< Plane red color component.                               */
const float  plane_color_g               = 0.7f;    /**< Plane green color component.                             */
const float  plane_color_b               = 0.1f;    /**< Plane blue color component.                              */

/** Light properties. */
typedef struct
{
    GLfloat r, g, b;                                /**< Light color components.                                  */
    GLfloat light_radius;                           /**< Radius of the sphere illuminated by this light.          */
    GLfloat orbit_height;                           /**< Orbit height of the light over the square surface.       */
    GLfloat orbit_radius;                           /**< Orbit radius of the light.                               */
    GLfloat angle_speed;                            /**< Angular velocity of the light.                           */
} light_properties_type;

/** Array of lights. */
light_properties_type lights_array[] =
{
    { 1.0f, 1.0f, 1.0f,  5.0f,  0.9f,  0.0f,   0.0f },
    { 1.0f, 0.0f, 0.0f,  0.5f,  0.2f,  0.1f,   7.0f },
    { 0.0f, 1.0f, 0.0f,  0.5f,  0.2f,  0.2f,  11.0f },
    { 0.0f, 0.0f, 1.0f,  0.5f,  0.2f,  0.3f,  13.0f },
    { 0.0f, 1.0f, 1.0f,  0.5f,  0.2f,  0.4f,  17.0f },
    { 1.0f, 0.0f, 1.0f,  0.5f,  0.2f,  0.5f,  19.0f },
    { 1.0f, 1.0f, 0.0f,  0.5f,  0.2f,  0.6f,  23.0f },
    { 1.0f, 0.7f, 0.5f,  0.5f,  0.2f,  0.7f,  29.0f },
    { 0.7f, 0.5f, 1.0f,  0.5f,  0.2f,  0.8f,  31.0f },
    { 0.5f, 1.0f, 0.7f,  0.5f,  0.2f,  0.9f,  37.0f },
    { 1.0f, 0.3f, 0.3f,  0.5f,  0.2f,  1.0f,  41.0f },
    { 0.3f, 0.3f, 1.0f,  0.5f,  0.2f,  1.1f,  43.0f },
    { 0.3f, 1.0f, 0.3f,  0.5f,  0.2f,  1.2f,  47.0f }
};
/** Amount of lights lighting the scene. */
const int lights_array_size = sizeof(lights_array) / sizeof(lights_array[0]);

/** Sphere properties. */
typedef struct
{
    GLfloat x, y, z;                                                 /**< Sphere coordinates.                                    */
    GLfloat radius;                                                  /**< Sphere radius.                                         */
    GLfloat r, g, b;                                                 /**< Sphere color components.                               */
} sphere_properties_type;

sphere_properties_type spheres_array[] =                             /**< Spheres properties array.                              */
{
    { 0.346f,  0.088f, -0.156f,   0.088f,   1.000f,  1.000f,  1.000f },
    {-0.091f,  0.051f, -0.413f,   0.051f,   0.504f,  0.129f,  0.214f },
    {-0.548f,  0.068f, -0.026f,   0.068f,   0.996f,  0.578f,  0.429f },
    {-0.725f,  0.085f,  0.006f,   0.085f,   0.848f,  0.749f,  0.142f },
    {-0.637f,  0.050f, -0.658f,   0.050f,   0.990f,  0.346f,  0.467f },
    {-0.385f,  0.062f, -0.643f,   0.062f,   0.725f,  0.023f,  0.377f },
    {-0.847f,  0.061f,  0.617f,   0.061f,   0.558f,  0.553f,  0.958f },
    {-0.908f,  0.081f, -0.618f,   0.081f,   0.258f,  0.002f,  0.741f },
    { 0.717f,  0.077f,  0.636f,   0.077f,   0.190f,  0.736f,  0.594f },
    { 0.443f,  0.063f,  0.485f,   0.063f,   0.486f,  0.727f,  0.105f },
    {-0.844f,  0.088f, -0.183f,   0.088f,   0.110f,  0.762f,  0.423f },
    { 0.178f,  0.063f, -0.891f,   0.063f,   0.273f,  0.367f,  0.030f },
    {-0.355f,  0.057f,  0.795f,   0.057f,   0.023f,  0.502f,  0.658f },
    { 0.619f,  0.075f, -0.196f,   0.075f,   0.376f,  0.320f,  0.121f },
    {-0.214f,  0.085f,  0.768f,   0.085f,   0.303f,  0.870f,  0.957f },
    { 0.723f,  0.083f,  0.081f,   0.083f,   0.863f,  0.072f,  0.856f },
    { 0.395f,  0.068f, -0.573f,   0.068f,   0.836f,  0.087f,  0.512f },
    { 0.124f,  0.088f,  0.521f,   0.088f,   0.115f,  0.449f,  0.168f },
    { 0.478f,  0.051f, -0.876f,   0.051f,   0.992f,  0.527f,  0.223f },
    {-0.322f,  0.072f,  0.336f,   0.072f,   0.463f,  0.157f,  0.575f },
    {-0.913f,  0.050f,  0.280f,   0.050f,   0.863f,  0.804f,  0.743f },
    {-0.570f,  0.085f, -0.347f,   0.085f,   0.918f,  0.180f,  0.838f },
    {-0.407f,  0.081f, -0.204f,   0.081f,   0.242f,  0.692f,  0.471f },
    { 0.217f,  0.081f,  0.018f,   0.081f,   0.893f,  0.330f,  0.663f },
    {-0.541f,  0.068f,  0.655f,   0.068f,   0.762f,  0.128f,  0.031f },
    {-0.428f,  0.074f,  0.473f,   0.074f,   0.764f,  0.091f,  0.943f },
    {-0.023f,  0.053f, -0.688f,   0.053f,   0.424f,  0.533f,  0.194f },
    {-0.333f,  0.073f, -0.051f,   0.073f,   0.957f,  0.562f,  0.164f },
    {-0.289f,  0.067f,  0.527f,   0.067f,   0.937f,  0.238f,  0.112f },
    { 0.352f,  0.074f, -0.330f,   0.074f,   0.452f,  0.452f,  0.450f },
    {-0.835f,  0.069f,  0.407f,   0.069f,   0.239f,  0.815f,  0.809f },
    { 0.754f,  0.057f, -0.382f,   0.057f,   0.318f,  0.324f,  0.830f },
    { 0.729f,  0.070f, -0.650f,   0.070f,   0.984f,  0.849f,  0.712f },
    {-0.422f,  0.056f, -0.836f,   0.056f,   0.931f,  0.213f,  0.296f },
    { 0.595f,  0.052f,  0.921f,   0.052f,   0.365f,  0.266f,  0.437f },
    { 0.457f,  0.083f, -0.020f,   0.083f,   0.533f,  0.987f,  0.305f },
    {-0.025f,  0.076f,  0.053f,   0.076f,   0.470f,  0.998f,  0.420f },
    { 0.894f,  0.075f, -0.427f,   0.075f,   0.332f,  0.132f,  0.545f },
    {-0.052f,  0.077f,  0.624f,   0.077f,   0.109f,  0.287f,  0.587f },
    {-0.616f,  0.086f,  0.444f,   0.086f,   0.107f,  0.802f,  0.230f },
    {-0.421f,  0.075f, -0.503f,   0.075f,   0.055f,  0.892f,  0.211f },
    { 0.887f,  0.058f,  0.913f,   0.058f,   0.040f,  0.079f,  0.272f },
    { 0.327f,  0.075f, -0.755f,   0.075f,   0.201f,  0.579f,  0.255f },
    {-0.338f,  0.057f, -0.322f,   0.057f,   0.402f,  0.376f,  0.221f },
    {-0.181f,  0.085f, -0.310f,   0.085f,   0.860f,  0.441f,  0.410f },
    {-0.817f,  0.055f, -0.780f,   0.055f,   0.223f,  0.269f,  0.666f },
    { 0.174f,  0.088f,  0.769f,   0.088f,   0.282f,  0.455f,  0.363f },
    { 0.568f,  0.081f,  0.365f,   0.081f,   0.445f,  0.287f,  0.912f },
    { 0.590f,  0.082f, -0.370f,   0.082f,   0.470f,  0.685f,  0.088f },
    {-0.372f,  0.061f,  0.654f,   0.061f,   0.590f,  0.893f,  0.568f }
};
/** Amount of spheres on the scene. */
const int spheres_array_size = sizeof(spheres_array) / sizeof(spheres_array[0]);

/* 1. GBuffer generation pass variable data. */
GLuint  gbuffer_generation_pass_program_id                   = 0;    /**< Program object id for gbuffer generation pass.         */
GLuint  gbuffer_generation_pass_vert_shader_id               = 0;    /**< Vertex shader id for gbuffer generation pass.          */
GLuint  gbuffer_generation_pass_frag_shader_id               = 0;    /**< Fragment shader id for gbuffer generation pass.        */

GLint   gbuffer_generation_pass_mvp_matrix_location          = 0;    /**< Location of combined model-view-projection matrix.     */
GLint   gbuffer_generation_pass_vertex_coordinates_location  = 0;    /**< Location of vertex coordinates attribute.              */
GLint   gbuffer_generation_pass_vertex_normal_location       = 0;    /**< Location of vertex normal attribute.                   */
GLint   gbuffer_generation_pass_object_color_location        = 0;    /**< Location of color uniform.                             */

/* 2. Shading pass variable data. */
GLuint  shading_pass_program_id                              = 0;    /**< Program object id for shading pass.                    */
GLuint  shading_pass_vert_shader_id                          = 0;    /**< Vertex shader id for shading pass.                     */
GLuint  shading_pass_frag_shader_id                          = 0;    /**< Fragment shader id for shading pass.                   */

GLint   shading_pass_mvp_matrix_location                     = 0;    /**< Location of combined model-view-projection matrix.     */
GLint   shading_pass_light_coordinates_location              = 0;    /**< Location of light coordinates uniform.                 */
GLint   shading_pass_light_radius_location                   = 0;    /**< Location of light radius uniform.                      */
GLint   shading_pass_light_color_location                    = 0;    /**< Location of light color uniform.                       */
GLint   shading_pass_inverted_viewprojection_matrix_location = 0;    /**< Location of inverted viewprojection matrix uniform.    */
GLint   shading_pass_inverted_viewport_vector_location       = 0;    /**< Location of inverted viewport vector uniform.          */
GLint   shading_pass_lightbox_vertex_coordinates_location    = 0;    /**< Location of vertex coordinates attribute.              */

/* 3. Resolve pass variable data. */
GLuint  combination_pass_program_id                          = 0;    /**< Program object id for combination pass.                */
GLuint  combination_pass_vert_shader_id                      = 0;    /**< Vertex shader id for combination pass.                 */
GLuint  combination_pass_frag_shader_id                      = 0;    /**< Fragment shader id for combination pass.               */

/* General purpose constants and variables. */
const GLsizei full_quad_vertex_count                         = 4;    /**< Amount of vertices to draw fullscreen quad.            */
Timer         timer;                                                 /**< Instance of a timer to measure time moments.           */
Matrix        matrix_mvp;                                            /**< Combined model-view-projection matrix.                 */
Matrix        matrix_view_projection;                                /**< Combined View-Projection matrix.                       */
Matrix        matrix_view_projection_inverted;                       /**< Inverted View-Projection matrix.                       */

/* Vertex meshes and normal vectors. */
MaliSDK::SphereModel::coordinates_array sphere_mesh_vertices;        /**< Sphere mesh. Nine sequential numbers represent one triangle.               */
MaliSDK::SphereModel::coordinates_array sphere_mesh_normals;         /**< Sphere mesh normal vectors. Three sequential numbers represent one vector. */
MaliSDK::PlaneModel::coordinates_array  plane_mesh_vertices;         /**< Plane mesh. Nine sequential numbers represent one triangle.                */
MaliSDK::PlaneModel::coordinates_array  plane_mesh_normals;          /**< Plane mesh normal vectors. Three sequential numbers represent one vector.  */
MaliSDK::CubeModel::coordinates_array   cube_mesh_vertices;          /**< Cube mesh. Nine sequential numbers represent one triangle.                 */

/* For MRT fallback */
GLint   shading_pass_normal_x_y_location                    = 0;
GLint   shading_pass_color_rgb_normal_sign_location         = 0;
GLint   shading_pass_depth_32_location                      = 0;

/** Converts specified degrees value to radians.
 *
 *  @param degrees angle in degree units
 *  @return        angle in radian units
 */
inline float degreesToRadians(float degrees)
{
    return float(degrees * atanf(1) / 45.0f);
}

/** Calculates combined view-projection matrix and its invertsion.
 *  The camera moves above globe's north hemisphere surface
 *  along the equator, oscillating between longitudes.
 *
 *  @param model_time  time moment for which to calculate the matrix
 *  @param vp          combined view-projection matrix
 *  @param inverted_vp inverted view-projection matrix
 */
void calc_view_projection_matrices(float model_time, Matrix& vp, Matrix& inverted_vp)
{
    /* Define projection properties. */
    const float camera_latitude_angle_range = camera_latitude_max_angle - camera_latitude_min_angle;

    /* The camera moves along some latitude deviating in a loop from equator to pole. */
    Vec3f center = { 0.0f, 0.0f, 0.0f };
    Vec3f camera = { 0.0f, 0.0f, 0.0f };
    Vec3f up     = { 0.0f, 1.0f, 0.0f };

    /* Calculate current camera position using the polar coordinate system. */
    float longitude              = float(degreesToRadians(camera_latitude_angle_speed * model_time));
    float min_max_latitude_ratio = float((1.0 + sin(degreesToRadians(camera_latitude_angle_range * model_time))) / 2.0);
    float latitude               = float(degreesToRadians(camera_latitude_min_angle + camera_latitude_angle_range * min_max_latitude_ratio));

    /* Convert polar coordinates into Cartesian coordinates. */
    camera.x = camera_distance * cos(latitude) * sin(longitude);
    camera.z = camera_distance * cos(latitude) * cos(longitude);
    camera.y = camera_distance * sin(latitude);

    /* Calculate the view matrix, which translates coordinates from world space to eye space. */
    Matrix mat4_view = Matrix::matrixCameraLookAt(camera, center, up);

    /* Create the projection matrix from frustum parameters. */
    const float frustum_aspect = (float)window_width / (float)window_height;
    Matrix mat4_projection = Matrix::matrixPerspective(degreesToRadians(frustum_fovy), frustum_aspect, frustum_z_near, frustum_z_far);

    /* Calculate combined view-projection matrix. */
    vp =  mat4_projection * mat4_view;

    /* Calculate inverted view-projection matrix. */
    inverted_vp = Matrix::matrixInvert(vp);
}

/** Calculates and returns a model matrix, which includes two operations:
 *  object scaling and object position translation.
 *
 *  @param scaling_factor factor by which to scale object's size
 *  @param translation    translation vector for object coordinates
 */
Matrix calc_model_matrix(float scaling_factor, Vec3f translation)
{
    Matrix matrix_scaling     = Matrix::createScaling(scaling_factor, scaling_factor, scaling_factor);
    Matrix matrix_translation = Matrix::createTranslation(translation.x, translation.y, translation.z);
    Matrix matrix_model       = matrix_translation * matrix_scaling;

    return matrix_model;
}

/** Calculates light position at the specified time.
 *  Lights move by planetary trajectory over the plane surface
 *  with their center over the center of the surface.
 *
 *  @param model_time   time for which to calculate the matrix
 *  @param orbit_height height of the light over the surface
 *  @param orbit_radius orbital distance from the center of the light
 *  @param angle_speed  angular velocity (in degrees per second) of the light
 *  @return             world coordinates of the light
 */
Vec3f calculate_light_position(float model_time, float orbit_height, float orbit_radius, float angle_speed)
{
    Vec3f light_position;

    light_position.x = orbit_radius * sin(degreesToRadians(angle_speed * model_time));
    light_position.z = orbit_radius * cos(degreesToRadians(angle_speed * model_time));
    light_position.y = orbit_height;

    return light_position;
}

/** Initializes the OpenGL ES and model environments.
 *
 *  @param width  window width reported by operating system
 *  @param height window width reported by operating system
 */
void setup_graphics(int width, int height)
{
    /* Store window width and height. */
    window_width  = width;
    window_height = height;

    /* Create sphere model. Generate the triangles' vertex mesh and per-vertex list of normal vectors in each vertex. */
    MaliSDK::SphereModel::getTriangleRepresentation(1.0f, sphere_tessellation_level, sphere_mesh_vertices);
    sphere_mesh_normals = sphere_mesh_vertices;

    /* Create plane model. Generate the triangles' vertex mesh and per-vertex list of normal vectors in each vertex. */
    MaliSDK::PlaneModel::getTriangleRepresentation(plane_mesh_vertices);
    MaliSDK::PlaneModel::getNormals(plane_mesh_normals);

    /* Generate a cube mesh. */
    MaliSDK::CubeModel::getTriangleRepresentation(1.0f, cube_mesh_vertices);


    /* 1. GBuffer generation pass. */
    /* Create gbuffer generation pass program object. */
    gbuffer_generation_pass_program_id = GL_CHECK(glCreateProgram());

    /* Compile the vertex and the fragment shaders. */
    Shader::processShader(&gbuffer_generation_pass_vert_shader_id, "assets/gbuffer_generation_pass_shader.vert", GL_VERTEX_SHADER  );
    Shader::processShader(&gbuffer_generation_pass_frag_shader_id, "assets/gbuffer_generation_pass_shader.frag", GL_FRAGMENT_SHADER);

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(gbuffer_generation_pass_program_id, gbuffer_generation_pass_vert_shader_id));
    GL_CHECK(glAttachShader(gbuffer_generation_pass_program_id, gbuffer_generation_pass_frag_shader_id));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(gbuffer_generation_pass_program_id));

    /* Get uniform locations */
    gbuffer_generation_pass_mvp_matrix_location         = GL_CHECK(glGetUniformLocation(gbuffer_generation_pass_program_id, "uMVP"));
    gbuffer_generation_pass_object_color_location       = GL_CHECK(glGetUniformLocation(gbuffer_generation_pass_program_id, "uObjectColor"));

    /* Get vertex attribute locations. */
    gbuffer_generation_pass_vertex_coordinates_location = GL_CHECK(glGetAttribLocation(gbuffer_generation_pass_program_id, "vObjectVertexCoordinates"));
    gbuffer_generation_pass_vertex_normal_location      = GL_CHECK(glGetAttribLocation(gbuffer_generation_pass_program_id, "vObjectVertexNormal"));

    /* Enable vertex attribute arrays in which we'll pass data through. */
    GL_CHECK(glEnableVertexAttribArray(gbuffer_generation_pass_vertex_coordinates_location));
    GL_CHECK(glEnableVertexAttribArray(gbuffer_generation_pass_vertex_normal_location));


    /* 2. Shading pass. */
    /* Create shading pass program object. */
    shading_pass_program_id = GL_CHECK(glCreateProgram());

    /* Compile the vertex and the fragment shaders. */
    Shader::processShader(&shading_pass_vert_shader_id, "assets/shading_pass_shader.vert", GL_VERTEX_SHADER  );
    Shader::processShader(&shading_pass_frag_shader_id, "assets/shading_pass_shader.frag", GL_FRAGMENT_SHADER);

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(shading_pass_program_id, shading_pass_vert_shader_id));
    GL_CHECK(glAttachShader(shading_pass_program_id, shading_pass_frag_shader_id));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(shading_pass_program_id));

    /* Get uniform locations. */
    shading_pass_light_coordinates_location              = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uLightPos"));
    shading_pass_light_radius_location                   = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uLightRadius"));
    shading_pass_light_color_location                    = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uLightColor"));
    shading_pass_inverted_viewprojection_matrix_location = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uInvViewProj"));
    shading_pass_inverted_viewport_vector_location       = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uInvViewport"));
    shading_pass_mvp_matrix_location                     = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uMVP"));

    /* Activate the program object, for which we'll be setting uniform values constant across program. */
    GL_CHECK(glUseProgram(shading_pass_program_id));

    /* Calculate and set the inverted viewport vector, which is constant through the rest of program. */
    GL_CHECK(glUniform2f(shading_pass_inverted_viewport_vector_location, 1.0f / window_width, 1.0f / window_height));

    /* Get locations of vertex attributes. */
    shading_pass_lightbox_vertex_coordinates_location    = GL_CHECK(glGetAttribLocation(shading_pass_program_id, "vLightboxVertexCoordinates"));

    /* All vertex attribute arrays are disabled by default which means they are not used for rendering. We enable the arrays we passed the data to. */
    GL_CHECK(glEnableVertexAttribArray(shading_pass_lightbox_vertex_coordinates_location));


    /* 3. Combination pass. */
    /* Create combination pass program object. */
    combination_pass_program_id = GL_CHECK(glCreateProgram());

    /* Compile the vertex and the fragment shaders. */
    Shader::processShader(&combination_pass_vert_shader_id, "assets/combination_pass_shader.vert", GL_VERTEX_SHADER  );
    Shader::processShader(&combination_pass_frag_shader_id, "assets/combination_pass_shader.frag", GL_FRAGMENT_SHADER);

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(combination_pass_program_id, combination_pass_vert_shader_id));
    GL_CHECK(glAttachShader(combination_pass_program_id, combination_pass_frag_shader_id));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(combination_pass_program_id));


    /* Enable face culling and depth testing, and disable blending. */
    GL_CHECK(glEnable(GL_CULL_FACE));
    /* [Disable blending and enable depth testing] */
    GL_CHECK(glDisable(GL_BLEND));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    /* [Disable blending and enable depth testing] */

    /* Start counting time. */
    timer.reset();
}

/** Renders one frame. */
void render_frame(void)
{
    /* All calculation will be executed for this time. */
    float model_time = timer.getTime();

    /* [Calculate view-projection matrix and its inversion] */
    /* We use it during gbuffer generation and shading passes. */
    calc_view_projection_matrices(model_time, matrix_view_projection, matrix_view_projection_inverted);
    /* [Calculate view-projection matrix and its inversion] */

    /* [Enable the pixel local storage extension] */
    GL_CHECK(glEnable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT));
    /* [Enable the pixel local storage extension] */


    /** Pass 1. GBuffer generation pass
     *
     *  At this pass we render primitives, but do not output them into the framebuffer/screen
     *  and only keep the generated fragment parameters (color and normal) in local pixel storage.
     */
    /* [Enable depth testing] */
    /* Only the fragment closest to the camera will be stored in the tile buffer. */
    GL_CHECK(glDepthMask(GL_TRUE));
    /* [Enable depth testing] */

    /* Clear the buffers. */
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Activate this pass' program. */
    GL_CHECK(glUseProgram(gbuffer_generation_pass_program_id));

    /* [Render the plane] */
    /* Attach mesh vertices and normals to appropriate shader attributes. */
    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_coordinates_location, 3, GL_FLOAT, GL_FALSE, 0, &plane_mesh_vertices[0]));
    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_normal_location,      3, GL_FLOAT, GL_FALSE, 0, &plane_mesh_normals[0] ));

    /* Specify a model-view-projection matrix. The model matrix is an identity matrix for the plane, so we can save one multiplication for the pass. */
    GL_CHECK(glUniformMatrix4fv(gbuffer_generation_pass_mvp_matrix_location,            1, GL_FALSE, matrix_view_projection.getAsArray()));
    GL_CHECK(glUniform3f(gbuffer_generation_pass_object_color_location, plane_color_r, plane_color_g, plane_color_b));

    /* Execute shader to render the plane into pixel storage. */
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)plane_mesh_vertices.size()/3));
    /* [Render the plane] */

    /* [Render the spheres] */
    /* Attach mesh vertices and normals to appropriate shader attributes. */
    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_coordinates_location, 3, GL_FLOAT, GL_FALSE, 0, &sphere_mesh_vertices[0]));
    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_normal_location,      3, GL_FLOAT, GL_FALSE, 0, &sphere_mesh_normals[0] ));

    /* Render each sphere on the scene in its position, size and color. */
    for (int i = 0; i < spheres_array_size; i++)
    {
        /* Calculate and set the MVP matrix for the sphere. Apply scaling and translation to sphere. */
        Vec3f sphere = { spheres_array[i].x, spheres_array[i].y, spheres_array[i].z };
        matrix_mvp = matrix_view_projection * calc_model_matrix(spheres_array[i].radius, sphere);

        /* Set MVP matrix and sphere color for the shader. */
        GL_CHECK(glUniformMatrix4fv(gbuffer_generation_pass_mvp_matrix_location, 1, GL_FALSE, matrix_mvp.getAsArray()));
        GL_CHECK(glUniform3f(gbuffer_generation_pass_object_color_location, spheres_array[i].r, spheres_array[i].g, spheres_array[i].b));

        /* Execute shader to render sphere into pixel local storage. */
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sphere_mesh_vertices.size()/3));
    }
    /* [Render the spheres] */


    /** Pass 2. Shading pass
     *
     *  At this pass we determine which fragments are illuminated by the light.
     *  We perform the same transformations over the lightbox vertices as in the gbuffer generation pass,
     *  but in the fragment shader we use the fragment world coordinates, the normal vector
     *  and the color to calculate and accumulate the lighting produced by each of the lights.
     */
    /* [Disable depth testing] */
    /* This pass should not update depths, only use them. */
    GL_CHECK(glDepthMask(GL_FALSE));
    /* [Disable depth testing] */

    /* Activate this pass' program. */
    GL_CHECK(glUseProgram(shading_pass_program_id));

    /* Set inverted MVP matrix for the shader. */
    GL_CHECK(glUniformMatrix4fv(shading_pass_inverted_viewprojection_matrix_location, 1, GL_FALSE, matrix_view_projection_inverted.getAsArray()));

    /* [Render the light boxes] */
    /* Attach the mesh vertices to the appropriate shader attribute. */
    GL_CHECK(glVertexAttribPointer(shading_pass_lightbox_vertex_coordinates_location, 3, GL_FLOAT, GL_FALSE, 0, &cube_mesh_vertices[0]));

    /* Process each light's bounding box on the scene in its position, size and color. */
    for (int i = 0; i < lights_array_size; i++)
    {
        /* Calculate the light position for the current time. */
        Vec3f light_position = calculate_light_position(model_time, lights_array[i].orbit_height, lights_array[i].orbit_radius, lights_array[i].angle_speed);

        /* Determine light box size. To avoid interference with the frustum it cannot be large than the scene. */
        float light_box_size = lights_array[i].light_radius > 1.0f ? 1.0f : lights_array[i].light_radius;

        /* Calculate and set MVP matrix for the light box. Apply scaling and translation. */
        matrix_mvp = matrix_view_projection * calc_model_matrix(light_box_size, light_position);
        GL_CHECK(glUniformMatrix4fv(shading_pass_mvp_matrix_location, 1, GL_FALSE, matrix_mvp.getAsArray()));

        /* Set the light radius, the light position and its color for the shading pass program. */
        GL_CHECK(glUniform1f(shading_pass_light_radius_location,      lights_array[i].light_radius));
        GL_CHECK(glUniform3f(shading_pass_light_color_location,       lights_array[i].r, lights_array[i].g, lights_array[i].b));
        GL_CHECK(glUniform3f(shading_pass_light_coordinates_location, light_position.x,  light_position.y,  light_position.z ));

        /* Execute the shader to light up fragments in the light box of the pixel storage. */
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)cube_mesh_vertices.size()/3));
    }
    /* [Render the light boxes] */


    /** Pass 3. Combination pass
     *
     *  At this pass we render the data gathered in pixel local storage onto screen.
     */
    /* [Combination pass] */
    /* Activate this pass' program. */
    GL_CHECK(glUseProgram(combination_pass_program_id));

    /* Render a fullscreen quad, so that each fragment has a chance to be updated with data from local pixel storage. */
    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, full_quad_vertex_count));
    /* [Combination pass] */


    /* [Disable the pixel local storage extension] */
    GL_CHECK(glDisable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT));
    /* [Disable the pixel local storage extension] */
}

/** Deinitializes the OpenGL ES environment. */
void cleanup()
{
    GL_CHECK(glDeleteShader (combination_pass_vert_shader_id       ));
    GL_CHECK(glDeleteShader (combination_pass_frag_shader_id       ));
    GL_CHECK(glDeleteProgram(combination_pass_program_id           ));
    GL_CHECK(glDeleteShader (shading_pass_vert_shader_id           ));
    GL_CHECK(glDeleteShader (shading_pass_frag_shader_id           ));
    GL_CHECK(glDeleteProgram(shading_pass_program_id               ));
    GL_CHECK(glDeleteShader (gbuffer_generation_pass_vert_shader_id));
    GL_CHECK(glDeleteShader (gbuffer_generation_pass_frag_shader_id));
    GL_CHECK(glDeleteProgram(gbuffer_generation_pass_program_id    ));

    sphere_mesh_vertices.clear();
    sphere_mesh_normals.clear();
    plane_mesh_vertices.clear();
    plane_mesh_normals.clear();
    cube_mesh_vertices.clear();
}

/** MRT (multiple render targets) fallback functions incase the device does not support PLS. **/
void setup_graphics_mrt(int width, int height);
void render_frame_mrt();
void cleanup_mrt();

/** Program entry point. */
int main(int argc, char** argv)
{
    /* Initialize the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if(platform != NULL)
    {
        /* Initialize windowing system. */
        platform->createWindow(window_width, window_height);

        /* Pixel-local storage cannot be used with multi-sampling. */
        EGLRuntime::setEGLSamples(0);
        /* Initialize EGL. */
        EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
        EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

        /* Check if PLS extension is available */
        bool pls_available = true;
        std::string ext = std::string((const char*)glGetString(GL_EXTENSIONS));
        if (ext.find("GL_EXT_shader_pixel_local_storage") == std::string::npos)
        {
            pls_available = false;
            LOGD("This device does not support shader pixel local storage");
            LOGD("Using multiple render targets instead.");
        }

        /* Initialze OpenGL ES and model environment for the tutorial. */
        if (pls_available)
            setup_graphics(window_width, window_height);
        else
            setup_graphics_mrt(window_width, window_height);

        bool end = false;
        /* The rendering loop to draw the scene. */
        while (!end)
        {
            /* If something happened to the window, leave the loop. */
            if(platform->checkWindow() != Platform::WINDOW_IDLE)
            {
                end = true;
            }

            /* Render a frame */
            if (pls_available)
                render_frame();
            else
                render_frame_mrt();

            /* Display the calculated scene on the window surface. */
            if (!eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface))
            {
                LOGE("Failed to swap buffers.");
            }
        }

        /* End of event loop.
         *  The window has been closed.
         *  The only thing we should do now is a clean up.
         */

        /* Deinitialze OpenGL ES environment. */
        if (pls_available)
            cleanup();
        else
            cleanup_mrt();

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

struct GBuffer
{
    GLuint fbo;
    GLuint depthbuffer;
    GLuint normal_x_y;
    GLuint color_rgb_normal_sign;
    GLuint depth_32;
};

GBuffer gbuffer;

GLuint gen_texture(GLenum internal_format, int width, int height)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void setup_graphics_mrt(int width, int height)
{
    window_width = width;
    window_height = height;

    /* Initialize a depth buffer to use for depth-testing and such */
    glGenRenderbuffers(1, &gbuffer.depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, gbuffer.depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, window_width, window_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    /* Create the output textures */
    gbuffer.normal_x_y = gen_texture(GL_RGBA8, window_width, window_height);
    gbuffer.color_rgb_normal_sign = gen_texture(GL_RGBA8, window_width, window_height);
    gbuffer.depth_32 = gen_texture(GL_RGBA8, window_width, window_height);

    /* Create a framebuffer object to hold all these outputs */
    glGenFramebuffers(1, &gbuffer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbuffer.normal_x_y, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gbuffer.color_rgb_normal_sign, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gbuffer.depth_32, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gbuffer.depthbuffer);

    /* This specifies that we wish to use more than one output in the fragment shader */
    GLenum draw_buffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, draw_buffers);

    /* This assert will fail if color formats were unsupported or something else went wrong */
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    MaliSDK::SphereModel::getTriangleRepresentation(1.0f, sphere_tessellation_level, sphere_mesh_vertices);
    sphere_mesh_normals = sphere_mesh_vertices;
    MaliSDK::PlaneModel::getTriangleRepresentation(plane_mesh_vertices);
    MaliSDK::PlaneModel::getNormals(plane_mesh_normals);
    MaliSDK::CubeModel::getTriangleRepresentation(1.0f, cube_mesh_vertices);

    /* 1. GBuffer generation pass. */
    gbuffer_generation_pass_program_id = GL_CHECK(glCreateProgram());
    Shader::processShader(&gbuffer_generation_pass_vert_shader_id, "assets/gbuffer_generation_pass_shader_mrt.vert", GL_VERTEX_SHADER);
    Shader::processShader(&gbuffer_generation_pass_frag_shader_id, "assets/gbuffer_generation_pass_shader_mrt.frag", GL_FRAGMENT_SHADER);
    GL_CHECK(glAttachShader(gbuffer_generation_pass_program_id, gbuffer_generation_pass_vert_shader_id));
    GL_CHECK(glAttachShader(gbuffer_generation_pass_program_id, gbuffer_generation_pass_frag_shader_id));
    GL_CHECK(glLinkProgram(gbuffer_generation_pass_program_id));
    gbuffer_generation_pass_mvp_matrix_location             = GL_CHECK(glGetUniformLocation(gbuffer_generation_pass_program_id, "uMVP"));
    gbuffer_generation_pass_object_color_location           = GL_CHECK(glGetUniformLocation(gbuffer_generation_pass_program_id, "uObjectColor"));
    gbuffer_generation_pass_vertex_coordinates_location     = GL_CHECK(glGetAttribLocation(gbuffer_generation_pass_program_id, "vObjectVertexCoordinates"));
    gbuffer_generation_pass_vertex_normal_location          = GL_CHECK(glGetAttribLocation(gbuffer_generation_pass_program_id, "vObjectVertexNormal"));
    GL_CHECK(glEnableVertexAttribArray(gbuffer_generation_pass_vertex_coordinates_location));
    GL_CHECK(glEnableVertexAttribArray(gbuffer_generation_pass_vertex_normal_location));

    /* 2. Shading pass. */
    /* Create shading pass program object. */
    shading_pass_program_id = GL_CHECK(glCreateProgram());
    Shader::processShader(&shading_pass_vert_shader_id, "assets/shading_pass_shader_mrt.vert", GL_VERTEX_SHADER);
    Shader::processShader(&shading_pass_frag_shader_id, "assets/shading_pass_shader_mrt.frag", GL_FRAGMENT_SHADER);
    GL_CHECK(glAttachShader(shading_pass_program_id, shading_pass_vert_shader_id));
    GL_CHECK(glAttachShader(shading_pass_program_id, shading_pass_frag_shader_id));
    GL_CHECK(glLinkProgram(shading_pass_program_id));
    shading_pass_light_coordinates_location                 = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uLightPos"));
    shading_pass_light_radius_location                      = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uLightRadius"));
    shading_pass_light_color_location                       = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uLightColor"));
    shading_pass_inverted_viewprojection_matrix_location    = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uInvViewProj"));
    shading_pass_inverted_viewport_vector_location          = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uInvViewport"));
    shading_pass_normal_x_y_location                        = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uNormalXY"));
    shading_pass_color_rgb_normal_sign_location             = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uColorRGBNormalSign"));
    shading_pass_depth_32_location                          = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uDepth32"));
    shading_pass_mvp_matrix_location                        = GL_CHECK(glGetUniformLocation(shading_pass_program_id, "uMVP"));
    GL_CHECK(glUseProgram(shading_pass_program_id));
    shading_pass_lightbox_vertex_coordinates_location       = GL_CHECK(glGetAttribLocation(shading_pass_program_id, "vLightboxVertexCoordinates"));
    GL_CHECK(glEnableVertexAttribArray(shading_pass_lightbox_vertex_coordinates_location));
    timer.reset();
}

void render_frame_mrt()
{
    float model_time = timer.getTime();
    calc_view_projection_matrices(model_time, matrix_view_projection, matrix_view_projection_inverted);
    glViewport(0, 0, window_width, window_height);

    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthMask(GL_TRUE));
    GL_CHECK(glDepthFunc(GL_LEQUAL));
    GL_CHECK(glDepthRangef(0.0f, 1.0f));
    GL_CHECK(glDisable(GL_BLEND));
    GL_CHECK(glEnable(GL_CULL_FACE));

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo));
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    GL_CHECK(glClearDepthf(1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Pass 1. Gbuffer generation pass */
    GL_CHECK(glUseProgram(gbuffer_generation_pass_program_id));
    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_coordinates_location, 3, GL_FLOAT, GL_FALSE, 0, &plane_mesh_vertices[0]));
    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, &plane_mesh_normals[0]));
    GL_CHECK(glUniformMatrix4fv(gbuffer_generation_pass_mvp_matrix_location, 1, GL_FALSE, matrix_view_projection.getAsArray()));
    GL_CHECK(glUniform3f(gbuffer_generation_pass_object_color_location, plane_color_r, plane_color_g, plane_color_b));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)plane_mesh_vertices.size() / 3));

    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_coordinates_location, 3, GL_FLOAT, GL_FALSE, 0, &sphere_mesh_vertices[0]));
    GL_CHECK(glVertexAttribPointer(gbuffer_generation_pass_vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, &sphere_mesh_normals[0]));

    for (int i = 0; i < spheres_array_size; i++)
    {
        /* Calculate and set the MVP matrix for the sphere. Apply scaling and translation to sphere. */
        Vec3f sphere = { spheres_array[i].x, spheres_array[i].y, spheres_array[i].z };
        matrix_mvp = matrix_view_projection * calc_model_matrix(spheres_array[i].radius, sphere);

        /* Set MVP matrix and sphere color for the shader. */
        GL_CHECK(glUniformMatrix4fv(gbuffer_generation_pass_mvp_matrix_location, 1, GL_FALSE, matrix_mvp.getAsArray()));
        GL_CHECK(glUniform3f(gbuffer_generation_pass_object_color_location, spheres_array[i].r, spheres_array[i].g, spheres_array[i].b));

        /* Execute shader to render sphere into pixel local storage. */
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sphere_mesh_vertices.size() / 3));
    }

    /* Pass 2. Shading pass */
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_CHECK(glUseProgram(shading_pass_program_id));
    GL_CHECK(glDepthMask(GL_FALSE));
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
    GL_CHECK(glBlendEquation(GL_FUNC_ADD));

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, gbuffer.normal_x_y));
    GL_CHECK(glUniform1i(shading_pass_normal_x_y_location, 0));

    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, gbuffer.color_rgb_normal_sign));
    GL_CHECK(glUniform1i(shading_pass_color_rgb_normal_sign_location, 1));

    GL_CHECK(glActiveTexture(GL_TEXTURE2));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, gbuffer.depth_32));
    GL_CHECK(glUniform1i(shading_pass_depth_32_location, 2));

    GL_CHECK(glUniform2f(shading_pass_inverted_viewport_vector_location, 1.0f / window_width, 1.0f / window_height));
    GL_CHECK(glUniformMatrix4fv(shading_pass_inverted_viewprojection_matrix_location, 1, GL_FALSE, matrix_view_projection_inverted.getAsArray()));
    GL_CHECK(glVertexAttribPointer(shading_pass_lightbox_vertex_coordinates_location, 3, GL_FLOAT, GL_FALSE, 0, &cube_mesh_vertices[0]));
    for (int i = 0; i < lights_array_size; i++)
    {
         Vec3f light_position = calculate_light_position(model_time, lights_array[i].orbit_height, lights_array[i].orbit_radius, lights_array[i].angle_speed);

        /* Determine light box size. To avoid interference with the frustum it cannot be large than the scene. */
        float light_box_size = lights_array[i].light_radius > 1.0f ? 1.0f : lights_array[i].light_radius;

        /* Calculate and set MVP matrix for the light box. Apply scaling and translation. */
        matrix_mvp = matrix_view_projection * calc_model_matrix(light_box_size, light_position);
        GL_CHECK(glUniformMatrix4fv(shading_pass_mvp_matrix_location, 1, GL_FALSE, matrix_mvp.getAsArray()));

        /* Set the light radius, the light position and its color for the shading pass program. */
        GL_CHECK(glUniform1f(shading_pass_light_radius_location, lights_array[i].light_radius));
        GL_CHECK(glUniform3f(shading_pass_light_color_location, lights_array[i].r, lights_array[i].g, lights_array[i].b));
        GL_CHECK(glUniform3f(shading_pass_light_coordinates_location, light_position.x, light_position.y, light_position.z));

        /* Execute the shader to light up fragments in the light box of the pixel storage. */
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)cube_mesh_vertices.size() / 3));
    }
}

void cleanup_mrt()
{
    GL_CHECK(glDeleteShader(shading_pass_vert_shader_id));
    GL_CHECK(glDeleteShader(shading_pass_frag_shader_id));
    GL_CHECK(glDeleteProgram(shading_pass_program_id));
    GL_CHECK(glDeleteShader(gbuffer_generation_pass_vert_shader_id));
    GL_CHECK(glDeleteShader(gbuffer_generation_pass_frag_shader_id));
    GL_CHECK(glDeleteProgram(gbuffer_generation_pass_program_id));

    GL_CHECK(glDeleteFramebuffers(1, &gbuffer.fbo));
	GL_CHECK(glDeleteRenderbuffers(1, &gbuffer.depthbuffer));
	GL_CHECK(glDeleteTextures(1, &gbuffer.color_rgb_normal_sign));
	GL_CHECK(glDeleteTextures(1, &gbuffer.normal_x_y));
	GL_CHECK(glDeleteTextures(1, &gbuffer.depth_32));

    sphere_mesh_vertices.clear();
    sphere_mesh_normals.clear();
    plane_mesh_vertices.clear();
    plane_mesh_normals.clear();
    cube_mesh_vertices.clear();
}
