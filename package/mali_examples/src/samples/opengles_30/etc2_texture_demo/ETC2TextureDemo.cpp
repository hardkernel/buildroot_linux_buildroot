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
 * \file ETC2TextureDemo.cpp
 * \brief Demonstration of ETC2 texture compression support in OpenGL ES 3.0.
 * 
 * Compresed textures are loaded and displayed on the screen. The internal format of each texture is displayed
 * at the bottom of the screen.
 * The application cycles through all of the textures formats supported by OpenGL ES 3.0.
 *
 * Formats:
 * - GL_COMPRESSED_R11_EAC: 11 bits for a single channel. Useful for single channel data where higher than 8 bit precision is needed. For example, heightmaps.
 * - GL_COMPRESSED_SIGNED_R11_EAC: Signed version of GL_COMPRESSED_SIGNED_R11_EAC, useful when signed data is needed.
 * - GL_COMPRESSED_RG11_EAC: 11 bits for two channels. Useful for two channel data where higher than 8 bit precision is needed. For example, normalised bump maps, the third component can be reconstructed from the other two components.
 * - GL_COMPRESSED_SIGNED_RG11_EAC: Signed version of GL_COMPRESSED_RG11_EAC, useful when signed data is needed.
 * - GL_COMPRESSED_RGB8_ETC2: 8 bits for three channels. Useful for normal textures without alpha values.
 * - GL_COMPRESSED_SRGB8_ETC2: sRGB version of GL_COMPRESSED_RGB8_ETC2.
 * - GL_COMPRESSED_RGBA8_ETC2_EAC: 8 bits for four channels. Useful for normal textures with varying alpha values.
 * - GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: sRGB version of GL_COMPRESSED_RGBA8_ETC2_EAC.
 * - GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: 8 bits for three channels and a 1 bit alpha channel. Useful for normal textures with binary alpha values.
 * - GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: sRGB version of GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2.
 */

#include "ETC2TextureDemo.h"

#include "EGLRuntime.h"
#include "ETCHeader.h"
#include "Platform.h"
#include "Shader.h"
#include "Text.h"
#include "Texture.h"
#include "Timer.h"

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>

using namespace MaliSDK;
using std::string;

/* Path to asset directories. */ 
string resourceDirectory = "assets/";
string compressedTextureDirectory = resourceDirectory + "textures/compressed/";

/* Structure to hold information about textures:
 * - internal format of image,
 * - path to image file,
 * - texture name,
 * - texture ID, used by OpenGL ES. 
 */
struct Image
{
    GLenum       internalformat;
    std::string  fileName;
    const char*  nameOfImageIneternalformat;
    GLuint       textureId; 
};

/* Initialization of assets. */
Image image0 = {GL_COMPRESSED_R11_EAC,                        compressedTextureDirectory + "HeightMap.pkm",       "GL_COMPRESSED_R11_EAC",                        0};
Image image1 = {GL_COMPRESSED_SIGNED_R11_EAC,                 compressedTextureDirectory + "HeightMapSigned.pkm", "GL_COMPRESSED_SIGNED_R11_EAC",                 0};
Image image2 = {GL_COMPRESSED_RG11_EAC,                       compressedTextureDirectory + "BumpMap.pkm",         "GL_COMPRESSED_RG11_EAC",                       0};
Image image3 = {GL_COMPRESSED_SIGNED_RG11_EAC,                compressedTextureDirectory + "BumpMapSigned.pkm",   "GL_COMPRESSED_SIGNED_RG11_EAC",                0};
Image image4 = {GL_COMPRESSED_RGB8_ETC2,                      compressedTextureDirectory + "Texture.pkm",         "GL_COMPRESSED_RGB8_ETC2",                      0};
Image image5 = {GL_COMPRESSED_SRGB8_ETC2,                     compressedTextureDirectory + "Texture.pkm",         "GL_COMPRESSED_SRGB8_ETC2",                     0};
Image image6 = {GL_COMPRESSED_RGBA8_ETC2_EAC,                 compressedTextureDirectory + "SemiAlpha.pkm",       "GL_COMPRESSED_RGBA8_ETC2_EAC",                 0};
Image image7 = {GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,          compressedTextureDirectory + "SemiAlpha.pkm",       "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC",          0};
Image image8 = {GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  compressedTextureDirectory + "BinaryAlpha.pkm",     "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2",  0};
Image image9 = {GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, compressedTextureDirectory + "BinaryAlpha.pkm",     "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2", 0};
 
/* Array of asset objects that will be used for displaying the images and text on screen. */
Image imageArray[] = {image0,
                      image1,
                      image2,
                      image3,
                      image4,
                      image5,
                      image6,
                      image7,
                      image8,
                      image9};

GLuint      bufferObjectIds[2]                        = {0};   /* Array of buffer objects names. Buffer objects hold quad and texture coordinates. */            
GLuint      currentAssetIndex                         = 0;     /* Index of imageArray to currently displayed image. */
const float displayInterval                           = 5.0f;  /* Number of seconds to display one image. */
GLuint      fragmentShaderId                          = 0;     /* Fragment shader name. */
string      fragmentShaderPath                        = resourceDirectory + "fragment_shader_source.frag";
Text*       internalformatTextDisplayer               = NULL;  /* Instance of a class that holds text with internalformat of displayed image. */
GLint       modelViewMatrixLocation                   = 0;     /* Default shader uniform model view location. */  
const int   numberOfTextures                          = sizeof(imageArray) / sizeof(imageArray[0]);
GLint       positionLocation                          = 0;     /* Default shader attribute position location. */
GLuint      programId                                 = 0;     /* Program name. */
float       scalingFactor                             = 0.75f; /* Scale factor for displaying texture image. */
GLint       textureCoordinateLocation                 = 0;     /* Default shader attribute texture coordinate location. */
GLint       textureLocation                           = 0;     /* Default shader uniform sampler2D location.*/ 
Timer       timer;                                             /* Instance of a timer that is used to change displayed image every a couple of seconds.*/
int         windowHeight                              = 800;   /* Height of window */
int         windowWidth                               = 800;   /* Width of window */
GLuint      vertexShaderId                            = 0;     /* Vertex shader name. */
string      vertexShaderPath                          = resourceDirectory + "vertex_shader_source.vert";
GLuint      vertexArrayId                             = 0;

/* Array of coordinates describing quad. */
float vertexData[] = {-1.0f, -1.0f, 0.0f,
                       1.0f, -1.0f, 0.0f,
                      -1.0f,  1.0f, 0.0f,
                      -1.0f,  1.0f, 0.0f,
                       1.0f, -1.0f, 0.0f,
                       1.0f,  1.0f, 0.0f};

/* Array of texture coordinates used for mapping texture to a quad. */
float textureCoordinatesData[] = {0.0f, 1.0f,
                                  1.0f, 1.0f,
                                  0.0f, 0.0f,
                                  0.0f, 0.0f,
                                  1.0f, 1.0f,
                                  1.0f, 0.0f};

/** Generate and fill texture objects with data.
 *
 *  \param textureIndex holds index of an imageArray (indicates which texture is to be created).
 *
 *  \return false if an error appeared, true otherwise.
 */
bool initializeTexture(int textureIndex)
{
    if (textureIndex >= 0 && textureIndex < numberOfTextures)
    {
        /* Loads the image data and information about the image. */
        std::string fileName = imageArray[textureIndex].fileName;
        ETCHeader etcHeader;
        unsigned char* imageData = NULL;
        Texture::loadPKMData(fileName.c_str(), &etcHeader, &imageData);

        if (imageData != NULL)
        {        
            /* Get width and height of mage. */
            int imageHeight = etcHeader.getHeight();
            int imageWidth  = etcHeader.getWidth();
            /* Get size of compressed image with padding included. */
            GLenum internalformat = imageArray[textureIndex].internalformat;
            GLsizei imageSize = etcHeader.getSize(internalformat);

            GLenum target = GL_TEXTURE_2D;
            /* Generate and bind texture. Generated texture name is written to imageArray at a given index. */
            GL_CHECK(glGenTextures(1,     &imageArray[textureIndex].textureId));
            GL_CHECK(glBindTexture(target, imageArray[textureIndex].textureId));

            /* Call CompressedTexImage2D() function which specifies texture with compressed image. */
            GL_CHECK(glCompressedTexImage2D(target, 0, internalformat, imageWidth, imageHeight, 0, imageSize, imageData));

            /* Set parameters for a texture. */
            GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
            GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
            GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        }
        else
        {
            LOGE("Could not load image data.");
            return false;
        }
    }
    else
    {
        LOGE("Incorrect value of index of imageArray.");
        return false;
    }

    return true;
}

/** Initializes data used for rendering.
 *
 *  \return false if an error appeared, true otherwise.
 */
bool setupTextures()
{
    /* Set OpenGL to use right aligmnent when reading texture images. */
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    /* Generate textures and fill them with data. */
    for (int allTextures = 0; allTextures < numberOfTextures; allTextures++)
    {
        if (!initializeTexture(allTextures))
        {
            return false;
        }
    }

    /* Generate and bind vertex array. */
    GL_CHECK(glGenVertexArrays(1, &vertexArrayId));
    GL_CHECK(glBindVertexArray(vertexArrayId));

    /* Generate buffers. */
    GL_CHECK(glGenBuffers(2, bufferObjectIds));

    /* Fill buffer object with vertex data. */
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, bufferObjectIds[0]));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW));

    /* Fill buffer object with texture coordinates data. */
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, bufferObjectIds[1]));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinatesData), textureCoordinatesData, GL_STATIC_DRAW));

    return true;
}

/** Draw image and text into back buffer.
 *
 *  \return false if an error appeared, true otherwise.
 */
bool draw()
{
    /* Draw text. */
    if (internalformatTextDisplayer != NULL)
    {
        internalformatTextDisplayer->clear();
        internalformatTextDisplayer->addString(0, 0, imageArray[currentAssetIndex].nameOfImageIneternalformat, 255, 255, 255, 255);
        internalformatTextDisplayer->draw();
    }

    /* Draw texture-mapped quad. */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, imageArray[currentAssetIndex].textureId));
    GL_CHECK(glUseProgram(programId));

    if (textureLocation != -1)
    {
        GL_CHECK(glUniform1i(textureLocation, 0));
    }
    
    /* Set up vertex atrribute pointers. */
    if (positionLocation != -1)
    {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, bufferObjectIds[0]));
        GL_CHECK(glEnableVertexAttribArray(positionLocation));
        GL_CHECK(glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

        if (textureCoordinateLocation != -1)
        {
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, bufferObjectIds[1]));
            GL_CHECK(glVertexAttribPointer(textureCoordinateLocation, 2, GL_FLOAT, GL_FALSE, 0, 0));
            GL_CHECK(glEnableVertexAttribArray(textureCoordinateLocation));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    /* Draw quad with texture image. */
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

    return true;
}

/** Create program that will be used to rasterize the geometry.
 *
 *  \return false if an error appeared, true otherwise.
 */
bool setupGraphics()
{
    if (!setupTextures())
    {
        return false;
    }

    internalformatTextDisplayer = new Text(resourceDirectory.c_str(), windowWidth, windowHeight);

    glClearColor(0.1f, 0.3f, 0.2f, 1.0f);

    /* Create scale matrix and orthographic matrix. */
    Matrix scaleMatrix       = Matrix::createScaling     (scalingFactor * windowWidth, scalingFactor * windowHeight, 1.0f);
    Matrix ortographicMatrix = Matrix::matrixOrthographic(float(-windowWidth), float(windowWidth), float(-windowHeight), 
                                                          float(windowHeight), -1.0f,              1.0f);
    
    /* Enable blending because it is needed for text drawing. */
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Do everything to create program. */
    Shader::processShader(&vertexShaderId,   vertexShaderPath.c_str(),   GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderId, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    programId = GL_CHECK(glCreateProgram());

    GL_CHECK(glAttachShader(programId, vertexShaderId));
    GL_CHECK(glAttachShader(programId, fragmentShaderId));

    GL_CHECK(glLinkProgram(programId));
    GL_CHECK(glUseProgram(programId));

    /* Get attributes and uniforms locations from shaders attached to the program. */
    modelViewMatrixLocation   = GL_CHECK(glGetUniformLocation(programId, "modelViewMatrix"));
    positionLocation          = GL_CHECK(glGetAttribLocation (programId, "attributePosition"));
    textureCoordinateLocation = GL_CHECK(glGetAttribLocation (programId, "attributeTextureCoordinate"));
    textureLocation           = GL_CHECK(glGetUniformLocation(programId, "uniformTexture"));

    /* Set up model-view matrix. */
    if (modelViewMatrixLocation != -1)
    {
        Matrix resultMatrix = ortographicMatrix * scaleMatrix;
        GL_CHECK(glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, resultMatrix.getAsArray()));
    }
    else
    {
        LOGE("Could not retrieve attribute location: modelViewMatrix.");
        return false;
    }

    return true;
}

/** Render new frame's contents into back buffer.
 *
 *  \return false if an error appeared, true otherwise.
 */
bool renderFrame(void)
{
    /* Clear contents of back buffer. */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Check if time for displaying one image has passed. */
    if(timer.getTime() > displayInterval)
    {
        /* If last picture available is displayed, move to the first one. */
        if (currentAssetIndex < numberOfTextures - 1)
        {
            currentAssetIndex ++; 
        }
        else
        {
            currentAssetIndex = 0;
        }

        /* Reset time counter. */
        timer.reset();
    }

    if (!draw())
    {
        return false;
    }
    
    return true;
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

        /* Prepare GL objects. */
        if (setupGraphics())
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
                if (!renderFrame())
                {
                    shouldContinueTheLoop = false;
                }

                /* 
                 * Push the EGL surface color buffer to the native window.
                 * Causes the rendered graphics to be displayed on screen.
                 */
                eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
            }

            /* Delete textures. */
            for (int i = 0; i < numberOfTextures; i++)
            {
                GL_CHECK(glDeleteTextures(1, &imageArray[i].textureId));
            }

            /* Delete buffers. */
            GL_CHECK(glDeleteBuffers(2, bufferObjectIds));

            /* Delete vertex array object. */
            GL_CHECK(glDeleteVertexArrays(1, &vertexArrayId));

            /* Release text object instance. */
            if (internalformatTextDisplayer != NULL)
            {
                delete internalformatTextDisplayer;
                internalformatTextDisplayer = NULL;
            }
        }
        else
        {
            LOGE("Could not prepare GL objects.");
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
        LOGE("Could not create platform.");
    }

    return 0;
}