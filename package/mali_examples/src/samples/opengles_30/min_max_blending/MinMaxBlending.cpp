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
 * \file MinMaxBlending.cpp
 * The application demonstrates behaviour of blending in GL_MIN and GL_MAX mode. It renders a 3D texture which consists of a
 * series of greyscaled images obtained from magnetic resonance of a human head. The images are placed one after another in
 * Z axis, so when blending is enabled they imitate a 3D model of the head.
 *
 * Texture coordinates are then rotated, so viewers can see the model from different perspectives and after each 5 seconds,
 * blending equation is changed. Since U/V/W coordinates are taken from interval <0.0, 1.0> and they are clamped to edge,
 * there might occure some distortions for specific angles of rotation. That is why, the application adds a few blank layers
 * behind and in the front of the original images. Now, if rotated coordinates exceed the interval, only the additional edge
 * layers are repeated creating a noiseless background.
 *
 * Because images contain a lot of black color, regular min blending would result in having black square on the screen. Hence,
 * there is a threshold applied in fragment shader which prevents rendering fragments that are not bright enough. Additionally,
 * for both types of blending, contrast of output luminance had to be modified to see more details.
 *
 * To use your own input images, it is check their format and adjust values of min blending threshold,
 * luminance of additional edge layers and contrast modifier.
 */

#include "MinMaxBlending.h"
#include "Matrix.h"
#include "Platform.h"
#include "Shader.h"
#include "Texture.h"
#include "Timer.h"
#include "EGLRuntime.h"

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <iostream>
#include <sstream>
#include <string>

using std::string;
using namespace MaliSDK;

/* Asset directories and filenames */
string resourceDirectory = "assets/";
string imagesFilename    = "MRbrain";

/* Number of images in resourceDirectory. */
const int imagesCount = 109;

/* Dimensions of window. */
const int windowWidth  = 600;
const int windowHeight = 600;

/* 
 * 3D texture dimensions. Although there are 109 images in resourceDirectory, texture depth is extended to 128 for 2 reasons:
 *     1) We require some layers in the front and behind the original ones, to avoid errors while rotating texture coordinates.
 *     2) Setting depth as a half of the other dimensions slightly improves the effect of blending.
 */
const GLint textureWidth  = 256;
const GLint textureHeight = 256;
const GLint textureDepth  = 128;

/* Emprically determined value of threshold used for min blending. */
const GLfloat minBlendingThreshold = 0.37f;
/* Color value of a 3D texture layer. */
const short fillerLuminance = 4;

/* ID of a 3D texture rendered on the screen. Filled by OpenGL ES. */
GLuint textureID = 0;

/* ID of a program assigned by OpenGL ES. */
GLuint programID = 0;

/* ID of a buffer object storing vertices of a square. */
GLuint verticesBufferID = 0;
/* ID of a buffer object storing U/V/W texture coordinates. */
GLuint uvwBufferID = 0;

/* ID of a vertex array object. */
GLuint vaoID = 0;

/* Locations of changable uniforms. */
GLint isMinBlendingLocation  = -1;
GLint rotationVectorLocation = -1;

/* 
 * Since there are additional layers in the front and in the back of original texture images, there
 * are two different functions used to load them. That is why we need this variable to indicate which
 * layer of 3D texture should be filled at the moment.
 */
GLint textureZOffset = 0;

/* Flag passed to shaders indicating current blending equation. */
GLboolean isMinBlending = GL_FALSE;

/* Amount of time in seconds used by a timer to switch blending equations. */
const float resetTimeInterval = 5.0f;

/* 
 * Array storing vertices of a square built from 2 triangles moved in the negative Z direction.
 *
 *   2-3----------------5
 *   | \\               |
 *   |   \\             |
 *   |     \\           |
 *   |       \\         |
 *   |         \\       |
 *   |           \\     |
 *   |             \\   |
 *   |               \\ |
 *   0----------------1-4
 */
const float squareVertices[] = 
{
    -1.0f,  1.0f, -1.0f, 1.0f,
     1.0f,  1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f,
     1.0f,  1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, -1.0f, 1.0f,
};

/* Array storing 3D texture coordinates corresponding to vertices of a square. */
const float uvwCoordinates[] =
{
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
};

/**
 * \brief Fills next empty 3D texture layer with textureData.
 *
 * It is called by the functions which prepare texture data either
 * by creating it inside the application or loading it from from a file.
 *
 * \param textureData Data the 3D texture is filled with.
 */
void setNextTextureImage(GLvoid* textureData)
{
    /* Set 2D image at the current textureZOffset. */
    GL_CHECK(glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, textureZOffset, textureWidth, textureHeight, 1, GL_RED_INTEGER, GL_SHORT, textureData));

    /* Increment textureZOffset. */
    textureZOffset++;
}

/**
 * \brief Creates and loads count unicolor layers to a 3D texture.
 * \param count Number of layers to be filled.
 */
void loadUniformTextures(int count)
{
    GLvoid* textureData = 0;

    /* Create texture with short data type. */
    Texture::createTexture(textureWidth, textureHeight, fillerLuminance, (short**) &textureData);

    /* Load created texture count times. */
    for (int i = 0; i < count; ++i)
    {
        setNextTextureImage(textureData);
    }

    /* Free allocated memory space. */
    delete [] (unsigned char *)textureData;
}

/**
 * \brief Loads imagesCount images located in resourceDirectory. 
 */
void loadImages(void)
{
    /* Indices of images start with 1. */
    for (int currentImageIndex = 1; currentImageIndex <= imagesCount; ++currentImageIndex)
    {
        GLvoid* textureData = 0;
        /* Maximum number of digits representing extensions. */
        const int digitsCount = 3;

        /* Convert index of the current image, to a string. */
        std::stringstream stringStream;
        stringStream << currentImageIndex;
        string numericExtension = stringStream.str();
        
        /* Path to the image. */
        const string filePath = resourceDirectory + imagesFilename + "." + numericExtension;

        /* Load data from a file. */
        Texture::loadData(filePath.c_str(), (unsigned char**) &textureData);

        /* Push loaded data to the next layer of a 3D texture that has not been filled yet. */
        setNextTextureImage(textureData);

        /* Free allocated memory space. */
        free(textureData);
    }
}

/**
 * \brief Fills 3D texture with images.
 * \return false if an error was reported, true otherwise.
 */
bool initializeTextureData(void)
{
    /* Number of layers added at the front of a 3D texture. */
    const int frontLayersCount = (textureDepth - imagesCount) / 2;
    /* Number of layers added at the back of a 3D texture. */
    const int backLayersCount = textureDepth - frontLayersCount - imagesCount;

    /* Check if both numbers of additional layers are not negative. */
    if (frontLayersCount < 0 || backLayersCount < 0)
    {
        LOGE("Too low textureDepth value or too many images have been tried to be loaded.");
        return false;
    }

    /* Load front layers. */
    loadUniformTextures(frontLayersCount);
    /* Load imagesCount images. */
    loadImages();
    /* Load back layers. */
    loadUniformTextures(backLayersCount);

    /* Make sure the 3D texture is fully loaded. */
    if(textureZOffset != textureDepth)
    {
        LOGE("3D texture not completely loaded.");
        return false;
    }

    return true;
}

/** 
 * \brief Initializes OpenGL ES texture components. 
 * \return false if an error was reported, true otherwise.
 */
bool initialize3DTexture(void)
{
    /* Generate and bind 3D texture. */
    GL_CHECK(glGenTextures(1, &textureID));
    GL_CHECK(glBindTexture(GL_TEXTURE_3D, textureID));

    /* Initialize storage space for texture data. */
    GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, GL_R16I, textureWidth, textureHeight, textureDepth, 0, GL_RED_INTEGER, GL_SHORT, NULL));

    /* Set texture parameters. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    /* Try loading image data. */
    if (!initializeTextureData())
    {
        LOGE("There occured errors while initializing texture data.");
        return false;
    }

    return true;
}

/** 
 * \brief Creates program and attaches shaders to it.
 */
void initializeProgram(void)
{
    /* Path to vertex shader source. */
    const string vertexShaderPath = resourceDirectory + "Min_Max_Blending_shader.vert";
    /* Path to fragment shader source. */
    const string fragmentShaderPath = resourceDirectory + "Min_Max_Blending_shader.frag";

    /* IDs of shaders. */
    GLuint vertexShaderID   = 0;
    GLuint fragmentShaderID = 0;

    /* Compile shaders and handle possible compilation errors. */
    Shader::processShader(&vertexShaderID,   vertexShaderPath.c_str(), GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    /* Generate ID for a program. */
    programID = GL_CHECK(glCreateProgram());

    /* Attach shaders to the program. */
    GL_CHECK(glAttachShader(programID, vertexShaderID));
    GL_CHECK(glAttachShader(programID, fragmentShaderID));

    /* Link the program. */
    GL_CHECK(glLinkProgram(programID));

    /* Since there is only one program, it is enough to make it current at this stage. */
    GL_CHECK(glUseProgram(programID));
}

/**
 * \brief Initializes input vertex data for shaders. 
 * \return false if an error was reported, true otherwise.
 */
bool initializeAttribArrays(void)
{
    /* Location of input variables in vertex shader. */
    GLint positionLocation            = GL_CHECK(glGetAttribLocation(programID, "inputPosition"));
    GLint inputUVWCoordinatesLocation = GL_CHECK(glGetAttribLocation(programID, "inputUVWCoordinates"));

    /* Generate and bind a vertex array object. */
    GL_CHECK(glGenVertexArrays(1, &vaoID));
    GL_CHECK(glBindVertexArray(vaoID));

    if (positionLocation != -1)
    {
        /* Generate and bind a buffer object storing vertices of a single quad. */
        GL_CHECK(glGenBuffers(1, &verticesBufferID));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, verticesBufferID));

        /* Put data into the buffer. */
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW));

        /* Set a vertex attribute pointer at the beginnig of the buffer. */
        GL_CHECK(glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CHECK(glEnableVertexAttribArray(positionLocation));
    }
    else
    {
        LOGE("Could not locate \"inputPosition\" attribute in program [%d].", programID);
        return false;
    }

    if (inputUVWCoordinatesLocation != -1)
    {
        /* Generate and bind a buffer object storing UV texture coordinates. */
        GL_CHECK(glGenBuffers(1, &uvwBufferID));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, uvwBufferID));

        /* Put data into the buffer. */
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(uvwCoordinates), uvwCoordinates, GL_STATIC_DRAW));

        /* Set vertex attribute pointer at the beginning of the buffer. */
        GL_CHECK(glVertexAttribPointer(inputUVWCoordinatesLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CHECK(glEnableVertexAttribArray(inputUVWCoordinatesLocation));
    }
    else
    {
        LOGE("Could not locate \"inputUVWCoordinates\" attribute in program [%d].", programID);
        return false;
    }

    return true;
}

/**
 * \brief Initializes uniform variables in program. 
 */
void initializeUniformData(void)
{
    /* Locations in shaders of uniform variables whose values are set only once. */
    GLint cameraMatrixLocation         = GL_CHECK(glGetUniformLocation(programID, "cameraMatrix"));
    GLint projectionMatrixLocation     = GL_CHECK(glGetUniformLocation(programID, "projectionMatrix"));
    GLint textureSamplerLocation       = GL_CHECK(glGetUniformLocation(programID, "textureSampler"));
    GLint instancesCountLocation       = GL_CHECK(glGetUniformLocation(programID, "instancesCount"));
    GLint minBlendingThresholdLocation = GL_CHECK(glGetUniformLocation(programID, "minBlendingThreshold"));

    /* Locations in shaders of uniform variables whose values are going to be modified. */
    isMinBlendingLocation  = GL_CHECK(glGetUniformLocation(programID, "isMinBlending"));
    rotationVectorLocation = GL_CHECK(glGetUniformLocation(programID, "rotationVector"));

    if (cameraMatrixLocation != -1)
    {
        /* Value of translation of camera in Z axis. */
        const float cameraTranslation = -2.0f;

        /* Matrix representing translation of camera. */
        Matrix cameraMatrix = Matrix::createTranslation(0.0f, 0.0f, cameraTranslation);

        /* Pass matrix to the program. */
        GL_CHECK(glUniformMatrix4fv(cameraMatrixLocation, 1, GL_FALSE, cameraMatrix.getAsArray()));
    }
    else
    {
        LOGD("Could not locate \"cameraMatrix\" uniform in program [%d].", programID);
    }

    if (projectionMatrixLocation != -1)
    {
        /* Perspective matrix used as projection matrix. */
        Matrix projectionMatrix = Matrix::matrixPerspective(45.0f, (float) windowWidth / (float) windowHeight, 0.01f, 10.0f);

        /* Pass matrix to the program. */
        GL_CHECK(glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix.getAsArray()));
    }
    else
    {
        LOGD("Could not locate \"projectionMatrix\" uniform in program [%d].", programID);
    }

    if (textureSamplerLocation != -1)
    {
        /* Pass default texture unit ID to the program. */
        GL_CHECK(glUniform1i(textureSamplerLocation, 0));
    }
    else
    {
        LOGD("Could not locate \"textureSampler\" uniform in program [%d].", programID);
    }

    if (instancesCountLocation != -1)
    {
        /* Pass the number of instances to be drawn, which is equal to the depth of texture. */
        GL_CHECK(glUniform1i(instancesCountLocation, textureDepth));
    }
    else
    {
        LOGD("Could not locate \"instancesCount\" uniform in program [%d].", programID);
    }

    if (minBlendingThresholdLocation != -1)
    {
        /* Pass the value of threshold used for min blending. */
        GL_CHECK(glUniform1f(minBlendingThresholdLocation, minBlendingThreshold));
    }
    else
    {
        LOGD("Could not locate \"minBlendingThreshold\" uniform in program [%d].", programID);
    }

    if (isMinBlendingLocation == -1)
    {
        LOGD("Could not locate \"isMinBlending\" uniform in program [%d]. Blending equation will not change.", programID);
    }

    if (rotationVectorLocation == -1)
    {
        LOGD("Could not locat \"rotationVector\" uniform in program [%d]. Texture will not rotate.", programID);
    }
}

/**
 * \brief Initializes OpenGL ES context. 
 * \return false if an error was reported, true otherwise.
 */
bool setupGraphics(void)
{
    initializeProgram();

    /* Try initializing 3D texture. Log error if it fails. */
    if (initialize3DTexture())
    {
        /* Try initializing atrribute arrays. Log error if it fails. */
        if (initializeAttribArrays())
        {
            initializeUniformData();

            /* Enable blending. */
            GL_CHECK(glEnable(GL_BLEND));
        }
        else
        {
            LOGE("Graphics setup failed at initialization of attribute arrays.");
            return false;
        }
    }
    else
    {
        LOGE("Graphics setup failed at texture initialization.");
        return false;
    }

    return true;
}

/**
 * \brief Sets current blending equation. 
 */
void setBlendEquation(GLboolean isMinBlending)
{
    if (isMinBlending)
    {
        /* Set new blend equation. */
        GL_CHECK(glBlendEquation(GL_MIN));
        /* Set white color for min blending. */
        GL_CHECK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
    }
    else
    {
        /* Set new blend equation. */
        GL_CHECK(glBlendEquation(GL_MAX));
        /* Set black color for max blending. */
        GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    }

    /* Pass boolean value informing shader about current blending mode. */
    GL_CHECK(glUniform1i(isMinBlendingLocation, isMinBlending));    
}

/**
 * \brief Renders single frame. 
 */
void renderFrame(void)
{
    /* Rotation angles. */
    static float angleX = 0.0f;
    static float angleY = 0.0f;
    static float angleZ = 0.0f;

    /* Arbitrary angle incrementat values. */
    const float angleXIncrement = 0.75f;
    const float angleYIncrement = 1.0f;
    const float angleZIncrement = 0.5f;

    /* Vector storing rotation angles that is going to be passed to shader. */
    float rotationVector[] = {angleX, angleY, angleZ};

    /* Clear the screen. */
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Pass the rotation vector to shader. */
    GL_CHECK(glUniform3fv(rotationVectorLocation, 1, rotationVector));

    /* Draw a single square layer consisting of 6 vertices for textureDepth times. */
    GL_CHECK(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, textureDepth));

    /* Increment rotation angles.*/
    angleX += angleXIncrement;
    angleY += angleYIncrement;
    angleZ += angleZIncrement;

    /* Make sure rotation angles do not go over 360 degrees. */
    if(angleX >= 360.0f) angleX = 0.0f;
    if(angleY >= 360.0f) angleY = 0.0f;
    if(angleZ >= 360.0f) angleZ = 0.0f;
}

/**
 * \brief Releases OpenGL ES objects. 
 *
 * It should be called before leaving the application.
 */
void releaseOpenGLObjects(void)
{
    GL_CHECK(glDeleteTextures    (1, &textureID       ));
    GL_CHECK(glDeleteBuffers     (1, &verticesBufferID));
    GL_CHECK(glDeleteBuffers     (1, &uvwBufferID     ));
    GL_CHECK(glDeleteVertexArrays(1, &vaoID           ));
    GL_CHECK(glDeleteProgram     (programID           ));
}

int main(int argc, char **argv)
{
    /* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if (platform != NULL)
    {
        /* Initialize windowing system. */
        platform->createWindow(windowWidth, windowHeight);

        /* Initialize EGL. */
        EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);
        EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

        /* Initialize OpenGL-ES graphics subsystem */
        if (setupGraphics())
        {
            /* Set initial blending equation. */
            setBlendEquation(isMinBlending);

            /* Generic framework timer used to count the time interval for switch of blending equations. */
            Timer timer;
            timer.reset();

            bool end = false;

            /* The rendering loop to draw the scene. */
            while(!end)
            {
                /* Finish the loop when state of window has been changed. */                
                if(platform->checkWindow() != Platform::WINDOW_IDLE)
                {
                    end = true;
                }

                /* Switch blending each resetTimeInterval seconds passed. */
                if (timer.isTimePassed(resetTimeInterval))
                {
                    isMinBlending = !isMinBlending;
                    setBlendEquation(isMinBlending);
                }

                /* Render a single frame */
                renderFrame();

                /*
                 * Push the EGL surface color buffer to the native window.
                 * Causes the rendered graphics to be displayed on screen.
                 */
                eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
            }

            releaseOpenGLObjects();
        }
        else
        {
            LOGE("Graphics setup failed. Application will exit.");
            exit(1);
        }

        /* Shut down EGL. */
        EGLRuntime::terminateEGL();

        /* Shut down windowing system. */
        platform->destroyWindow();

        /* Shut down the Platform object*/
        delete platform;
    }
    else
    {
        LOGE("Could not initialize platform. Application will exit.");
        exit(1);
    }

    return 0;
}
