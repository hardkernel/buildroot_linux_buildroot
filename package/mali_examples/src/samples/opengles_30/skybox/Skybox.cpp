/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _WIN32
    #include <unistd.h>
#endif /* _WIN32 */

#include "Platform.h"
#include "Image.h"
#include "Quaternions.h"
#include "Shader.h"
#include "Text.h"

#include <cmath>

/* Disable deprecation warning on Windows. */
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif /* _WIN32 */

/* Window resolution. */
const unsigned int window_width  = 800;
const unsigned int window_height = 600;

using namespace MaliSDK;

int main(int argc, char** argv)
{
    /* Result of linking a program. */
    GLint link_status = GL_FALSE;

    /* Location of a 'viewMat' uniform variable. */
    GLint location_viewMat = 0;

    /* ID of a program object. */
    GLuint program_id = 0;

    /* ID of a fragment shader. */
    GLuint frag_shader_id = 0;

    /* ID of a vertex shader. */
    GLuint vert_shader_id = 0;

    /* Quaternions representing rotations around X, Y and Z axes. */
    Quaternion Q_X = { 0.0f, 0.0f, 0.0f, 0.0f };
    Quaternion Q_Y = { 0.0f, 0.0f, 0.0f, 0.0f };
    Quaternion Q_Z = { 0.0f, 0.0f, 0.0f, 0.0f };

    /* Quaternions to store resultant products. */
    Quaternion Q_XY  = { 0.0f, 0.0f, 0.0f, 0.0f };
    Quaternion Q_XYZ = { 0.0f, 0.0f, 0.0f, 0.0f };

    /* Path to cubemap texture. */
    char file_name[] = { "assets/greenhouse_skybox-0.ppm" };

    /* Used to hold cube-map texture face data when initializing skybox cube-map texture. */
    ImageFile cubemap_image = { 0, 0, NULL };

    /* Texture cubemap targets. */
    GLenum cubemap_faces[] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    /* Texture cubemap name. */
    GLuint cubemap_texture = 0;

    /* Number of degrees to rotate counterclockwise around X, Y and Z axes respectively. */
    float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;

    /* 4x4 matrix that transforms the skybox's vertices from model space to world space. */
    float model_view_matrix[16] = {0.0f};

    /* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if(platform == NULL)
    {
        fprintf(stderr, "Could not create platform\n");
        exit(-1);
    }

    /* Initialize windowing system. */
    platform->createWindow(window_width, window_height);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

    /* Generate texture name and bind it to the texture cubemap target. */
    GL_CHECK(glGenTextures(1, &cubemap_texture));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture));

    /* Set up texture parameters. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    /* Load cubemap texture. */
    cubemap_image = load_ppm_file(file_name);

    /* Specify storage for all levels of a cubemap texture. */
    GL_CHECK(glTexStorage2D(GL_TEXTURE_CUBE_MAP,    /* Texture target */
                            1,                      /* Number of texture levels */
                            GL_RGB8,                /* Internal format for texture storage */
                            cubemap_image.width,    /* Width of the texture image */
                            cubemap_image.height)); /* Height of the texture image */

    for (int n_face = 0; n_face < sizeof(cubemap_faces) / sizeof(cubemap_faces[0]); n_face++)
    {
        if (n_face != 0)
        {
            sprintf(file_name, "assets/greenhouse_skybox-%d.ppm", n_face);

            cubemap_image = load_ppm_file(file_name);
        }

        GL_CHECK(glTexSubImage2D(cubemap_faces[n_face],                  /* Texture target. */
                                 0,                                      /* Level-of-detail number. */
                                 0,                                      /* Texel offset in the x direction. */
                                 0,                                      /* Texel offset in the y direction. */
                                 cubemap_image.width,                    /* Width of the texture image. */
                                 cubemap_image.height,                   /* Height of the texture image. */
                                 GL_RGB,                                 /* Format of the pixel data. */
                                 GL_UNSIGNED_BYTE,                       /* Type of the pixel data. */
                                 (const GLvoid*) cubemap_image.pixels)); /* Pointer to the image data. */

        FREE_CHECK(cubemap_image.pixels);
    }

    /* Shader initialization. */
    Shader::processShader(&vert_shader_id, "assets/shader.vert", GL_VERTEX_SHADER);
    Shader::processShader(&frag_shader_id, "assets/shader.frag", GL_FRAGMENT_SHADER);

    /* Create a program object that we will attach the fragment and vertex shader to. */
    program_id = GL_CHECK(glCreateProgram());

    /* Attach the shaders. */
    GL_CHECK(glAttachShader(program_id, vert_shader_id));
    GL_CHECK(glAttachShader(program_id, frag_shader_id));

    /* Link the program object. */
    GL_CHECK(glLinkProgram(program_id));

    /* Indicates the link operation on program was successful. */
    GL_CHECK(glGetProgramiv(program_id, GL_LINK_STATUS, &link_status));

    if (link_status != GL_TRUE)
    {
        fprintf(stderr, "Linking a program object has failed.\n");
        exit(-1);
    }

    /* The program object has been successfully linked. Let's use it. */
    GL_CHECK(glUseProgram(program_id));

    /* Retrieve uniform location for "viewMat" uniform defined in vertex shader. */
    location_viewMat = GL_CHECK(glGetUniformLocation(program_id, "viewMat"));

    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    Text* text = new Text("assets/", window_width, window_height);
    text->clear();
    text->addString(0, 0, "Skybox Sample", 255, 255, 0, 255);

    /* Rendering loop to draw the scene starts here. */
    bool shouldContinueTheLoop = true;

    while (shouldContinueTheLoop)
    {
        /* If something happened to the window, leave the loop. */
        if (platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            shouldContinueTheLoop = false;
        }

        /* Construct quaternions for X, Y and Z axes. */
        Q_X = construct_quaternion(1.0f, 0.0f, 0.0f, roll);
        Q_Y = construct_quaternion(0.0f, 1.0f, 0.0f, pitch);
        Q_Z = construct_quaternion(0.0f, 0.0f, 1.0f, yaw);

        /* Obtain the resultant quaternion. */
        Q_XY  = multiply_quaternions(Q_X, Q_Y);
        Q_XYZ = multiply_quaternions(Q_XY, Q_Z);

        /* Compute a modelview matrix. Model matrix is a unit matrix. */
        construct_modelview_matrix(Q_XYZ, model_view_matrix);

        /* In this demo, we do not need to provide the vertex shader with any mesh data, because a predefined set 
           of 4 vertices is embedded within the shader. These vertices, expressed in Normalized Device Coordinates,
           correspond to four corners of the visible screen space. By using this vertices to form a triangle strip,
           we end up with a full-screen quad that is later used for rasterization stage. */

        /* Restore the cubemap program object, because it has been changed by text rendering call. */
        GL_CHECK(glUseProgram(program_id));

        /* Upload the matrix to view matrix uniform so that it can be used for vertex shader stage. */
        GL_CHECK(glUniformMatrix4fv(location_viewMat, 1, GL_FALSE, (const GLfloat*) model_view_matrix));

        /* The angles can be decremented too to reverse the direction of rotation. */
        roll  += 0.2f;
        pitch += 0.4f;
        yaw   += 0.2f;

        /* Rotating the skybox by more than 360 or less than 0 degrees is not permitted, in order to avoid problems. */
        if (fabs(roll) >= 360.0f)
        {
            if (roll < 0.0f) 
            {
                roll += 360.0f;
            }
            else
            {
                roll -= 360.0f;
            }
        }

        if (fabs(pitch) >= 360.0f)
        {
            if (pitch < 0.0f)
            {
                pitch += 360.0f;
            }
            else
            {
                pitch -= 360.0f;
            }
        }

        if (fabs(yaw) >= 360.0f)
        {
            if (yaw < 0.0f) 
            {
                yaw += 360.0f;
            }
            else
            {
                yaw -= 360.0f;
            }
        }

        /* Render a full-screen quad, as described above.
           Note that the actual content of the quad is drawn within the fragment shader. */
        GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

        text->draw();

        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);

        /* Limit to 100 fps. */
        #ifdef _WIN32
                Sleep(10);
        #else
                usleep(10000);
        #endif
    }

    /* End of event loop.
       The window has been closed.
       The only thing we should do now is a clean up. */

    /* Delete the cube map texture. */
    GL_CHECK(glDeleteTextures(1, &cubemap_texture));

    /* Release shaders. */
    GL_CHECK(glUseProgram(0));
    GL_CHECK(glDeleteShader(vert_shader_id));
    GL_CHECK(glDeleteShader(frag_shader_id));
    GL_CHECK(glDeleteProgram(program_id));

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;

    return 0;
}

#ifdef _WIN32
    #pragma warning(pop)
#endif /* _WIN32 */