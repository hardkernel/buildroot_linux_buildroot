/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "Platform.h"
#include "EGLRuntime.h"
#include <GLES3/gl3.h>

#include "Text.h"
#include "AstcTextures.h"
#include "Shader.h"
#include "Timer.h"
#include "SolidSphere.h"

using namespace MaliSDK;
using std::string;

/* Instance of Timer used to measure texture switch time. */
Timer timer;

/* Instance of Timer used to determine current time. */
Timer fps_timer;

/* Instance of text renderer used to display name of the compressed texture internalformat, 
   in which textures used for rendering are stored. */
Text* text_displayer = NULL;

/* Instance of SolidSphere which provides mesh data for the globe. */
SolidSphere* solid_sphere = NULL;

/* Window resolution. */
const unsigned int window_width  = 1024;
const unsigned int window_height = 768;

/* Field of view in y-direction set up to 60 deg., expressed in radians. */
const float field_of_view = M_PI * 60.0f / 180.0f;

/* Aspect ratio of perspective frustrum. */
const float x_to_y_ratio = (float) window_width / (float) window_height;

/* Distances between camera and near/far plane of clipping frustrum. */
const float z_near = 0.01f;
const float z_far  = 100.0f;

/* Sampler locations. */
GLint cloud_texture_location     = 0;
GLint daytime_texture_location   = 0;
GLint nighttime_texture_location = 0;

/* Uniform locations. */
GLint mv_location  = 0;
GLint mvp_location = 0;

/* Attribute locations. */
GLint normal_location         = 0;
GLint position_location       = 0;
GLint texture_coords_location = 0;

/* Buffer object ID. */
GLuint bo_id = 0;

/* Vertex and fragment shader IDs. */
GLuint vert_shader_id = 0;
GLuint frag_shader_id = 0;

/* Program object ID. */
GLuint program_id = 0;

/* Vertex array object ID. */
GLuint vao_id = 0;

/* Initial angles around x, y and z axes. */
float angle_x = START_ANGLE_X_ROTATION;
float angle_y = 0;
float angle_z = 0;

/* Time value for rotation and translation calculations. */
float current_time = 0;

/* Model-view transform matrix. */
Matrix model_view_matrix;

/* Model-view-perspective transform matrix. */
Matrix mvp_matrix;

/* Perspective projection matrix. */
Matrix perspective_matrix = Matrix::matrixPerspective(field_of_view, x_to_y_ratio, z_near, z_far);

/* Rotation matrix. */
Matrix rotate_matrix;

/* Indicates which texture set is to be bound to texture units. */
unsigned int current_texture_set_id = 0;

/* Array storing texture bindings. */
texture_set texture_ids[NUM_OF_TEXTURE_IDS] = { 0 };

/* Number of texture sets. */
const int n_texture_ids = sizeof(texture_ids) / sizeof(texture_ids[0]);

int sphere_indices_size = 0;
unsigned short* sphere_indices = NULL;

/* Array containing information about all texture sets. */
texture_set_info texture_sets_info[] =
{
    /* Compression type. */                    /* Path to cloud texture file. */        /* Path to day texture file. */        /* Path to night texture file. */      /* Name of compression algorithm. */
    GL_COMPRESSED_RGBA_ASTC_4x4_KHR,           "assets/CloudAndGloss4x4.astc",          "assets/Earth-Color4x4.astc",          "assets/Earth-Night4x4.astc",          "4x4 ASTC",
    GL_COMPRESSED_RGBA_ASTC_5x4_KHR,           "assets/CloudAndGloss5x4.astc",          "assets/Earth-Color5x4.astc",          "assets/Earth-Night5x4.astc",          "5x4 ASTC",
    GL_COMPRESSED_RGBA_ASTC_5x5_KHR,           "assets/CloudAndGloss5x5.astc",          "assets/Earth-Color5x5.astc",          "assets/Earth-Night5x5.astc",          "5x5 ASTC",
    GL_COMPRESSED_RGBA_ASTC_6x5_KHR,           "assets/CloudAndGloss6x5.astc",          "assets/Earth-Color6x5.astc",          "assets/Earth-Night6x5.astc",          "6x5 ASTC",
    GL_COMPRESSED_RGBA_ASTC_6x6_KHR,           "assets/CloudAndGloss6x6.astc",          "assets/Earth-Color6x6.astc",          "assets/Earth-Night6x6.astc",          "6x6 ASTC",
    GL_COMPRESSED_RGBA_ASTC_8x5_KHR,           "assets/CloudAndGloss8x5.astc",          "assets/Earth-Color8x5.astc",          "assets/Earth-Night8x5.astc",          "8x5 ASTC",
    GL_COMPRESSED_RGBA_ASTC_8x6_KHR,           "assets/CloudAndGloss8x6.astc",          "assets/Earth-Color8x6.astc",          "assets/Earth-Night8x6.astc",          "8x6 ASTC",
    GL_COMPRESSED_RGBA_ASTC_8x8_KHR,           "assets/CloudAndGloss8x8.astc",          "assets/Earth-Color8x8.astc",          "assets/Earth-Night8x8.astc",          "8x8 ASTC",
    GL_COMPRESSED_RGBA_ASTC_10x5_KHR,          "assets/CloudAndGloss10x5.astc",         "assets/Earth-Color10x5.astc",         "assets/Earth-Night10x5.astc",         "10x5 ASTC",
    GL_COMPRESSED_RGBA_ASTC_10x6_KHR,          "assets/CloudAndGloss10x6.astc",         "assets/Earth-Color10x6.astc",         "assets/Earth-Night10x6.astc",         "10x6 ASTC",
    GL_COMPRESSED_RGBA_ASTC_10x8_KHR,          "assets/CloudAndGloss10x8.astc",         "assets/Earth-Color10x8.astc",         "assets/Earth-Night10x8.astc",         "10x8 ASTC",
    GL_COMPRESSED_RGBA_ASTC_10x10_KHR,         "assets/CloudAndGloss10x10.astc",        "assets/Earth-Color10x10.astc",        "assets/Earth-Night10x10.astc",        "10x10 ASTC",
    GL_COMPRESSED_RGBA_ASTC_12x10_KHR,         "assets/CloudAndGloss12x10.astc",        "assets/Earth-Color12x10.astc",        "assets/Earth-Night12x10.astc",        "12x10 ASTC",
    GL_COMPRESSED_RGBA_ASTC_12x12_KHR,         "assets/CloudAndGloss12x12.astc",        "assets/Earth-Color12x12.astc",        "assets/Earth-Night12x12.astc",        "12x12 ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,   "assets/CloudAndGloss4x4.astc",          "assets/Earth-Color4x4.astc",          "assets/Earth-Night4x4.astc",          "4x4 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,   "assets/CloudAndGloss5x4.astc",          "assets/Earth-Color5x4.astc",          "assets/Earth-Night5x4.astc",          "5x4 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,   "assets/CloudAndGloss5x5.astc",          "assets/Earth-Color5x5.astc",          "assets/Earth-Night5x5.astc",          "5x5 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,   "assets/CloudAndGloss6x5.astc",          "assets/Earth-Color6x5.astc",          "assets/Earth-Night6x5.astc",          "6x5 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,   "assets/CloudAndGloss6x6.astc",          "assets/Earth-Color6x6.astc",          "assets/Earth-Night6x6.astc",          "6x6 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,   "assets/CloudAndGloss8x5.astc",          "assets/Earth-Color8x5.astc",          "assets/Earth-Night8x5.astc",          "8x5 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,   "assets/CloudAndGloss8x6.astc",          "assets/Earth-Color8x6.astc",          "assets/Earth-Night8x6.astc",          "8x6 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,   "assets/CloudAndGloss8x8.astc",          "assets/Earth-Color8x8.astc",          "assets/Earth-Night8x8.astc",          "8x8 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,  "assets/CloudAndGloss10x5.astc",         "assets/Earth-Color10x5.astc",         "assets/Earth-Night10x5.astc",         "10x5 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,  "assets/CloudAndGloss10x6.astc",         "assets/Earth-Color10x6.astc",         "assets/Earth-Night10x6.astc",         "10x6 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,  "assets/CloudAndGloss10x8.astc",         "assets/Earth-Color10x8.astc",         "assets/Earth-Night10x8.astc",         "10x8 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, "assets/CloudAndGloss10x10.astc",        "assets/Earth-Color10x10.astc",        "assets/Earth-Night10x10.astc",        "10x10 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, "assets/CloudAndGloss12x10.astc",        "assets/Earth-Color12x10.astc",        "assets/Earth-Night12x10.astc",        "12x10 SRGB ASTC",
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, "assets/CloudAndGloss12x12.astc",        "assets/Earth-Color12x12.astc",        "assets/Earth-Night12x12.astc",        "12x12 SRGB ASTC"
};

/**
 * \brief Update texture bindings and text presented by text renderer.
 *
 * \param[in] force_switch_texture If true texture is immediately switched,
 *                                 if false texture is switched after interval passes.
 */
void update_texture_bindings(bool force_switch_texture)
{
    if (timer.getTime() >= ASTC_TEXTURE_SWITCH_INTERVAL || force_switch_texture)
    {
        /* If the current texture set is to be changed, reset timer to start counting time again. */
        timer.reset();

        /* Clear current text. */
        text_displayer->clear();

        if (!force_switch_texture)
        {
            if (current_texture_set_id < n_texture_ids - 1)
            {
                current_texture_set_id++;
            }
            else
            {
                current_texture_set_id = 0;
            }
        }

        /* Changed displayed text. */
        text_displayer->addString((window_width - (Text::textureCharacterWidth * texture_ids[current_texture_set_id].name.size())) >> 1,
                                   window_height - Text::textureCharacterHeight,
                                   texture_ids[current_texture_set_id].name.c_str(),
                                   255,  /* Red channel. */
                                   255,  /* Green channel. */
                                   0,    /* Blue channel. */
                                   255); /* Alpha channel. */
    }

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_ids[current_texture_set_id].cloud_and_gloss_texture_id));
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_ids[current_texture_set_id].earth_color_texture_id));
    GL_CHECK(glActiveTexture(GL_TEXTURE2));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_ids[current_texture_set_id].earth_night_texture_id));
}

/**
 * \brief Define and retrieve compressed texture image.
 *
 * \param[in] file_name           Texture file name.
 * \param[in] texture_compression Compression type (ETC1/ETC2/ASTC).
 */
GLuint load_texture(string file_name, int texture_compression)
{
    unsigned char* input_data = NULL;

    GLenum       compressed_data_internal_format = 0;
    long         file_size = 0;
    unsigned int n_bytes_to_read = 0;
    size_t       result = 0;
    GLuint       to_id = 0;

    /* Number of blocks in the x, y or z direction. */
    int xblocks = 0;
    int yblocks = 0;
    int zblocks = 0;

    /* Number of bytes for each dimension. */
    int xsize   = 0;
    int ysize   = 0;
    int zsize   = 0;

    FILE* compressed_data_file = fopen(file_name.c_str(), "rb");

    if (compressed_data_file == NULL)
    {
        LOGE("Could not open a file.\n");
        exit(1);
    }

    LOGI("Loading texture [%s]\n", file_name.c_str());

    /* Obtain file size. */
    fseek(compressed_data_file, 0, SEEK_END);
    file_size = ftell(compressed_data_file);
    rewind(compressed_data_file);

    /* Allocate memory to contain the whole file. */
    input_data = (unsigned char*) malloc(sizeof(unsigned char) * file_size);

    if (input_data == NULL)
    {
        LOGE("Memory allocation error FILE: %s LINE: %i\n", __FILE__, __LINE__);
        exit(2);
    }

    /* Copy the file into the buffer. */
    result = fread(input_data, 1, file_size, compressed_data_file);

    if (result != file_size)
    {
        LOGE("Reading error [%s] ... FILE: %s LINE: %i\n", file_name.c_str(), __FILE__, __LINE__);
        exit(3);
    }

    /* Traverse the file structure. */
    astc_header* astc_data_ptr = (astc_header*) input_data;

    /* Merge x,y,z-sizes from 3 chars into one integer value. */
    xsize = astc_data_ptr->xsize[0] + (astc_data_ptr->xsize[1] << 8) + (astc_data_ptr->xsize[2] << 16);
    ysize = astc_data_ptr->ysize[0] + (astc_data_ptr->ysize[1] << 8) + (astc_data_ptr->ysize[2] << 16);
    zsize = astc_data_ptr->zsize[0] + (astc_data_ptr->zsize[1] << 8) + (astc_data_ptr->zsize[2] << 16);

    /* Compute number of blocks in each direction. */
    xblocks = (xsize + astc_data_ptr->blockdim_x - 1) / astc_data_ptr->blockdim_x;
    yblocks = (ysize + astc_data_ptr->blockdim_y - 1) / astc_data_ptr->blockdim_y;
    zblocks = (zsize + astc_data_ptr->blockdim_z - 1) / astc_data_ptr->blockdim_z;

    /* Each block is encoded on 16 bytes, so calculate total compressed image data size. */
    n_bytes_to_read                 = xblocks * yblocks * zblocks << 4;
    compressed_data_internal_format = texture_compression;

    /* We now have file contents in memory so let's fill a texture object with the data. */
    GL_CHECK(glGenTextures(1, &to_id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, to_id));

    /* Upload texture data to ES. */
    GL_CHECK(glCompressedTexImage2D(GL_TEXTURE_2D,
                                    0,
                                    compressed_data_internal_format,
                                    xsize,
                                    ysize,
                                    0,
                                    n_bytes_to_read,
                                    (const GLvoid*)&astc_data_ptr[1]));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT));

    /* Unbind texture from target. */
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    /* Terminate file operations. */
    fclose(compressed_data_file);
    free(input_data);

    input_data = NULL;

    return to_id;
}

/**
 * \brief Define 32 texture sets that the demo will switch between every 5 seconds.
 */
void load_textures(void)
{
    for (int i = 0; i < n_texture_ids; i++)
    {
        texture_ids[i].cloud_and_gloss_texture_id = load_texture(texture_sets_info[i].cloud_and_gloss_texture_file_path, texture_sets_info[i].compression_type_name);
        texture_ids[i].earth_color_texture_id     = load_texture(texture_sets_info[i].earth_color_texture_file_path,     texture_sets_info[i].compression_type_name);
        texture_ids[i].earth_night_texture_id     = load_texture(texture_sets_info[i].earth_night_texture_file_path,     texture_sets_info[i].compression_type_name);
        texture_ids[i].name                       = texture_sets_info[i].compressed_texture_format_name;
    }

    /* Configure texture set. */
    update_texture_bindings(true);
}

/**
 * \brief Invoke glGetAttribLocation(), if it has returned a positive value.
 *        Otherwise, print a message and exit. Function used for clarity reasons.
 *
 * \param[in] program     OpenGL ES specific.
 * \param[in] attrib_name OpenGL ES specific.
 */
GLint get_and_check_attrib_location(GLuint program, const GLchar* attrib_name)
{
    GLint attrib_location = GL_CHECK(glGetAttribLocation(program, attrib_name));

    if (attrib_location == -1)
    {
        LOGE("Cannot retrieve location of %s attribute.\n", attrib_name);
        exit(0);
    }

    return attrib_location;
}

/** 
 * \brief Invoke glGetUniformLocation, if it has returned a positive value.
 *        Otherwise, print a message and exit. Function used for clarity reasons.
 *
 * \param[in] program      OpenGL ES specific.
 * \param[in] uniform_name OpenGL ES specific.
 */
GLint get_and_check_uniform_location(GLuint program, const GLchar* uniform_name)
{
    GLint uniform_location = GL_CHECK(glGetUniformLocation(program, uniform_name));

    if (uniform_location == -1)
    {
        LOGE("Cannot retrieve location of %s uniform.\n", uniform_name);
        exit(0);
    }

    return uniform_location;
}

/**
 * \brief This function sets up a program object that will be used for rendering,
 *        as well as retrieves attribute & uniform locations.
 */
void setup_program(void)
{
    /* Create program_id (ready to attach shaders). */
    program_id = GL_CHECK(glCreateProgram());

    /* Shaders initialization. */
    Shader::processShader(&vert_shader_id, "assets/shader.vert", GL_VERTEX_SHADER);
    Shader::processShader(&frag_shader_id, "assets/shader.frag", GL_FRAGMENT_SHADER);

    /* Attach shaders and link program object. */
    GL_CHECK(glAttachShader(program_id, vert_shader_id));
    GL_CHECK(glAttachShader(program_id, frag_shader_id));
    GL_CHECK(glLinkProgram(program_id));

    /* Get attribute locations of attributes vertex position, normal and texture coordinates. */
    position_location       = get_and_check_attrib_location(program_id, "av4position");
    normal_location         = get_and_check_attrib_location(program_id, "vv3normal");
    texture_coords_location = get_and_check_attrib_location(program_id, "vv3tex2dcoord");

    /* Get uniform locations. */
    mv_location                = get_and_check_uniform_location(program_id, "mv");
    mvp_location               = get_and_check_uniform_location(program_id, "mvp");
    cloud_texture_location     = get_and_check_uniform_location(program_id, "cloud_texture");
    daytime_texture_location   = get_and_check_uniform_location(program_id, "daytime_texture");
    nighttime_texture_location = get_and_check_uniform_location(program_id, "nighttime_texture");

    /* Activate program object. */
    GL_CHECK(glUseProgram(program_id));
}

/**
 * \brief Sets up a buffer object that will hold mesh data (vertex positions, normal vectors, textures UV coordinates).
 */
void load_buffer_data(void)
{
    const float sphere_radius = 1.0f;

    /* Number of pararells and meridians the sphere should consists of. */
    const unsigned int n_sectors = 64;
    const unsigned int n_rings   = 64;

    /* New instance of sphere. */
    solid_sphere = new SolidSphere(sphere_radius, n_rings, n_sectors);

    /* Obtain sizes of the several subbuffers. */
    GLsizei sphere_vertices_size;
    GLsizei sphere_normals_size;
    GLsizei sphere_texcoords_size;

    /* Load generated mesh data from SolidSphere object. */
    float* sphere_vertices  = solid_sphere->getSphereVertexData(&sphere_vertices_size);
    float* sphere_normals   = solid_sphere->getSphereNormalData(&sphere_normals_size);
    float* sphere_texcoords = solid_sphere->getSphereTexcoords(&sphere_texcoords_size);

    /* Size of the entire buffer. */
    GLsizei buffer_total_size = sphere_vertices_size + sphere_normals_size + sphere_texcoords_size;

    /* Create buffer object to hold all mesh data. */
    GL_CHECK(glGenBuffers(1, &bo_id));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, bo_id));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, buffer_total_size, NULL, GL_STATIC_DRAW));

    /* Upload subsets of mesh data to buffer object. */
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0,                                          sphere_vertices_size,  sphere_vertices));
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, sphere_vertices_size,                       sphere_normals_size,   sphere_normals));
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, sphere_vertices_size + sphere_normals_size, sphere_texcoords_size, sphere_texcoords));

    free(sphere_vertices);
    free(sphere_normals);
    free(sphere_texcoords);

    sphere_vertices  = NULL;
    sphere_normals   = NULL;
    sphere_texcoords = NULL;

    /* Configure vertex attribute arrays, so that position/normals/texture coordinate data is available to the vertex shader. */
    GL_CHECK(glGenVertexArrays(1, &vao_id));
    GL_CHECK(glBindVertexArray(vao_id));
    GL_CHECK(glEnableVertexAttribArray(position_location));
    GL_CHECK(glEnableVertexAttribArray(normal_location));
    GL_CHECK(glEnableVertexAttribArray(texture_coords_location));

    GLsizei buffer_offset = 0;

    /* Populate attribute for position. */
    GL_CHECK(glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) buffer_offset));

    buffer_offset += sphere_vertices_size;

    /* Populate attribute for normals. */
    GL_CHECK(glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) buffer_offset));

    buffer_offset += sphere_normals_size;

    /* Populate attribute for texture coordinates. */
    GL_CHECK(glVertexAttribPointer(texture_coords_location, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) buffer_offset));

    /* Bind texture units to texture samplers. */
    GL_CHECK(glUniform1i(cloud_texture_location,     0));
    GL_CHECK(glUniform1i(daytime_texture_location,   1));
    GL_CHECK(glUniform1i(nighttime_texture_location, 2));

    /* Load generated indices from SolidSphere object. */
    sphere_indices = solid_sphere->getSphereIndices(&sphere_indices_size);
}

/**
 * \brief Renders a single frame.
 */
void render_frame(void)
{
    /* Prepare rotation matrices and use them to set up model+view matrix. */
    model_view_matrix = Matrix::createRotationX(angle_x);
    rotate_matrix     = Matrix::createRotationY(angle_y);
    model_view_matrix = rotate_matrix * model_view_matrix;
    rotate_matrix     = Matrix::createRotationZ(-angle_z);
    model_view_matrix = rotate_matrix * model_view_matrix;

    /* Pull the camera back from the cube - move back and forth as time goes by.
       To achieve it scale with time translational part of model matrix for z-direction. */
    model_view_matrix[14] -= 2.5f + sinf(current_time / 5.0f) * 0.5f;

    /* Actually used program object has been changed by text rendering functions. Restore it. */
    GL_CHECK(glUseProgram(program_id));
    GL_CHECK(glBindVertexArray(vao_id));

    /* Upload view matrix. */
    GL_CHECK(glUniformMatrix4fv(mv_location, 1, GL_FALSE, &model_view_matrix[0]));

    /* Bring model from camera space into Normalized Device Coordinates (NDC). */
    mvp_matrix = perspective_matrix * model_view_matrix;

    /* Upload complete Model Space->World Space->Camera(Eye) Space->NDC transformation by updating relevant MVP uniform.
       Vertex shader will then use this matrix to transform all input vertices from model space to screen space. */
    GL_CHECK(glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp_matrix[0]));

    /* Get the current time. */
    current_time = fps_timer.getTime();

    /* Obtain angular rates around X, Y, Z axes. */
    angle_x = (float) (current_time * X_ROTATION_SPEED);
    angle_y = (float) (current_time * Y_ROTATION_SPEED);
    angle_z = (float) (current_time * Z_ROTATION_SPEED);

    /* Clear buffers. */
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Draw the sphere from array data. */
    GL_CHECK(glDrawElements(GL_TRIANGLES, sphere_indices_size, GL_UNSIGNED_SHORT, sphere_indices));

    /* Also don't forget to show name of the compression algorithm in action. */
    text_displayer->draw();

    /* In the end swap back and front buffers. */
    if (!eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface))
    {
        LOGE("Failed to swap buffers.\n");
    }

    /* Switch the texture set if more than 5 seconds has passed since the last switch event. */
    update_texture_bindings(false);
}

int main(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if (platform == NULL)
    {
        LOGE("Could not create platform\n");
        exit(-1);
    }

    /* Initialize windowing system. */
    platform->createWindow(window_width, window_height);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

    /* Make sure the required ASTC extension is present. */
    const GLubyte* extensions = GL_CHECK(glGetString(GL_EXTENSIONS));

    string extensionsString;
    string astcExtensionName("GL_KHR_texture_compression_astc_ldr");

    extensionsString.append((const char*) extensions);

    if (extensionsString.find(astcExtensionName) == string::npos)
    {
        LOGI("OpenGL ES 3.0 implementation does not support GL_KHR_texture_compression_astc_ldr extension.\n");
        exit(0);
    }

    /* Enable culling and depth testing. */
    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    /* Enable blending and specify pixel arihmetic.
       Transparency is implemented using blend function with primitives sorted from the farthest to the nearest. */
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    text_displayer = new Text("assets/", window_width, window_height);

    /* Create texture objects and fill them with texture data. */
    load_textures();

    /* Set up shader objects.
       Rerieve uniform and attribute locations. */
    setup_program();

    /* Prepare buffer objects that will hold mesh data. */
    load_buffer_data();

    /* Start counting time. */
    timer.reset();
    fps_timer.reset();

    /* Rendering loop used to draw the scene starts here. */
    while (true)
    {
        /* If something happened to the window, leave the loop. */
        if (platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            break;
        }

        /* Render a frame. */
        render_frame();
    }

    /* Delete all used textures. */
    for (int i = 0; i < n_texture_ids; i++)
    {
        GL_CHECK(glDeleteTextures(1, &texture_ids[i].cloud_and_gloss_texture_id));
        GL_CHECK(glDeleteTextures(1, &texture_ids[i].earth_color_texture_id));
        GL_CHECK(glDeleteTextures(1, &texture_ids[i].earth_night_texture_id));
    }

    /* Cleanup shaders. */
    GL_CHECK(glUseProgram(0));
    GL_CHECK(glDeleteShader(vert_shader_id));
    GL_CHECK(glDeleteShader(frag_shader_id));
    GL_CHECK(glDeleteProgram(program_id));

    /* Delete vertex array object. */
    GL_CHECK(glDisableVertexAttribArray(position_location));
    GL_CHECK(glDisableVertexAttribArray(normal_location));
    GL_CHECK(glDisableVertexAttribArray(texture_coords_location));
    GL_CHECK(glDeleteVertexArrays(1, &vao_id));

    /* Free buffer object memory. */
    GL_CHECK(glDeleteBuffers(1, &bo_id));

     /* Free indices buffer. */
    free(sphere_indices);
    sphere_indices = NULL;

    /* Release memory for Text and SolidSphere instances. */
    delete text_displayer;
    delete solid_sphere;

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;

    return 0;
}
