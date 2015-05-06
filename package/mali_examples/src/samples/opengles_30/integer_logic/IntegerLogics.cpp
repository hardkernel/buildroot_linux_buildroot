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
 * \file IntegerLogics.cpp
 * \brief The application simulates cellular automata phenomenon following Rule 30. It uses two programs
 *        which operate on two textures used in a ping-pong manner.
 *
 *        The first program takes the ping texture ("ping") as the input and renders the output to a second texture ("pong").
 *        Rendering in this case is performed by drawing one row at a time, with each row having height of 1 pixel and being of screen width.
 *        Excluding the first row, each row is drawn by reading one row above the currently processed one and applying the cellular automata  rule.
 *        The first row's contents are set by the application.
 *        Since we cannot draw and read from the same texture at a single time, the drawing is performed one row at a time.
 *        After a row is drawn to texture A, the application binds texture B for drawing and uses texture A
 *        for reading the previous line. In the end, texture A contains even rows and texture B stores odd rows. 
 *
 *        Having finished drawing lines to these two textures, we run another GL program that merges both textures into a single one by
 *        using texture A for even lines and texture B for odd ones.
 *
 *        In order to be able to render to a texture, we use a custom frame-buffer.
 *
 *        For the first run, the input line has only one pixel lit, so it generates
 *        the commonly known Rule 30 pattern. Then, every 5 seconds, textures are reset
 *        and the input is randomly generated.
 *
 */

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>

#include <string>

#include "EGLRuntime.h"
#include "IntegerLogics.h"
#include "Matrix.h"
#include "Platform.h"
#include "Shader.h"
#include "Texture.h"
#include "Timer.h"

using           std::string;
using namespace MaliSDK;

/* Asset directory. */
string resourceDirectory            = "assets/";
/* Name of the file in which "rule 30" vertex shader's body is located. */
string vertexRule30ShaderFilename   = "IntegerLogic_Rule30_shader.vert";
/* Name of the file in which "merge" vertex shader's body is located. */
string vertexMergeShaderFilename    = "IntegerLogic_Merge_shader.vert";
/* Name of the file in which "rule 30" fragment shader's body is located. */
string fragmentRule30ShaderFilename = "IntegerLogic_Rule30_shader.frag";
/* Name of the file in which "merge" fragment shader's body is located. */
string fragmentMergeShaderFilename  = "IntegerLogic_Merge_shader.frag";

/* ID assigned by GL ES for "rule 30" program. */
GLuint rule30ProgramID = 0;
/* ID assigned by GL ES for "merge" program. */
GLuint mergeProgramID  = 0;

/* Texture unit used for configuring 2D texture binding for ping textures. */
const GLuint pingTextureUnit = 0;
/* Texture unit used for configuring 2D texture binding for pong textures. */
const GLuint pongTextureUnit = 1;

/* Data for the initial line of the ping texture. */
GLvoid* pingTextureData = NULL;
/* ID of ping texture that holds the input data. */
GLuint  pingTextureID   = -1;
/* ID of pong texture whose entire input depend on the ping texture. */
GLuint  pongTextureID   = -1;

/* OpenGL ES ID for a frame-buffer we use for off-screen rendering. */
GLuint framebufferID = -1;
/* OpenGL ES ID for a buffer object used for storing line vertex position data. */
GLuint linePositionBOID = -1;
/* OpenGL ES ID for a buffer object used for storing line U/V texture coordinate data. */
GLuint lineUVBOID = -1;
/* OpenGL ES ID for a buffer object used for storing quad vertex position data. */
GLuint quadPositionBOID = 0;
/* OpenGL ES ID for a buffer object used for storing quad U/V texture coordinate data. */
GLuint quadUVBOID = 0;
/* OpengGL ES ID for a Vertex Array object that we use for storing line vertex attribute assignments. */
GLuint lineVAOID = -1;
/* OpenGL ES ID for a Vertex Array object that we use for storing quad vertex attribute assignments. */
GLuint quadVAOID = 0;

/* Cached projection matrix. */
Matrix modelViewProjectionMatrix;

/* Time interval between input textures are reset. */
const float resetTimeInterval = 5.0f;

/**
 * \brief Generates input for Rule 30 Cellular Automaton, setting a white dot in the top line of the texture
 *        on the given horizontal offset.
 * 
 * \param [in]  xoffset     Horizontal position of the stripe.
 * \param [in]  width       Width of the texture.
 * \param [in]  height      Height of the texture.
 * \param [in]  nComponents Number of components defining the colors in the texture.
 * \param [out] textureData Output texture.
 */
void generateRule30Input(unsigned int xoffset, unsigned int width, unsigned int height, unsigned int nComponents, GLvoid** textureData)
{
    if(textureData == NULL)
    {
        LOGE("Null data passed");
        return;
    }

    for(unsigned int channelIndex = 0; channelIndex < nComponents; ++channelIndex)
    {
        (*(unsigned char**)textureData)[(height - 1) * width * nComponents + xoffset * nComponents + channelIndex] = 255;
    }
}

/**
 * \brief Genertes random input for Rule 30 Cellular Automaton, setting random white dots in the top line of the texture.
 *
 * \param [in]  width       Width of the texture.
 * \param [in]  height      Height of the texture.
 * \param [in]  nComponents Number of components defining the colors in the texture.
 * \param [out] textureData Output texture.
 */
void generateRule30Input(unsigned int width, unsigned int height, unsigned int nComponents, GLvoid** textureData)
{
    if(textureData == NULL)
    {
        LOGE("Null data passed");
        return;
    }

    for (unsigned int texelIndex = 0; texelIndex < width; ++texelIndex)
    {
        bool setWhite = (rand() % 2 == 0) ? true : false;

        if (setWhite)
        {
            for (unsigned int channelIndex = 0; channelIndex < nComponents; ++channelIndex)
            {
                (*(unsigned char**)textureData)[(height - 1) * width * nComponents + texelIndex * nComponents + channelIndex] = 255;
            }
        }
    }
}

/* Initializes all the required components. */
bool setupGraphics()
{
    /* Create paths we will use for loading shader bodies. */
    string vertexRule30ShaderPath   = resourceDirectory + vertexRule30ShaderFilename;
    string vertexMergeShaderPath    = resourceDirectory + vertexMergeShaderFilename;
    string fragmentRule30ShaderPath = resourceDirectory + fragmentRule30ShaderFilename;
    string fragmentMergeShaderPath  = resourceDirectory + fragmentMergeShaderFilename;

    /* Initialize matrices. */
    Matrix scale        = Matrix::createScaling     ( (float) WINDOW_W, (float) WINDOW_H, 1.0f);
    /* Multiplication by 2 for vertical boundaries are caused by setting 0.5 as w coordinate in vertices array. */
    Matrix orthographic = Matrix::matrixOrthographic(-(float) WINDOW_W, (float) WINDOW_W, -(float) WINDOW_H * 2, (float) WINDOW_H * 2, -1.0f, 1.0f);

    modelViewProjectionMatrix = orthographic * scale;

    /* Create Input data for the ping texture. */
    Texture::createTexture(WINDOW_W,     WINDOW_H, 0,           &pingTextureData);
    generateRule30Input   (WINDOW_W / 2, WINDOW_W, WINDOW_H, 1, &pingTextureData);

    /* Generate textures. */
    GLuint textureIDs[] = {0, 0};

    GL_CHECK(glGenTextures(2, textureIDs));

    pingTextureID = textureIDs[0];
    pongTextureID = textureIDs[1];

    /* Load ping texture data. */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture  (GL_TEXTURE_2D, pingTextureID));
    GL_CHECK(glTexImage2D   (GL_TEXTURE_2D, 0,                     GL_R8UI, WINDOW_W, WINDOW_H, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, pingTextureData));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    /* Load pong texture data. */
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture  (GL_TEXTURE_2D, pongTextureID));
    GL_CHECK(glTexImage2D   (GL_TEXTURE_2D, 0,                     GL_R8UI, WINDOW_W, WINDOW_H, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    /* Code to setup shaders, geometry, and to initialise OpenGL ES states. */
    GLuint fragmentMergeShaderID  = 0;
    GLuint fragmentRule30ShaderID = 0;
    GLuint vertexRule30ShaderID   = 0;
    GLuint vertexMergeShaderID    = 0;

    /* Process shaders. */
    Shader::processShader(&vertexRule30ShaderID,   vertexRule30ShaderPath.c_str(),   GL_VERTEX_SHADER);
    Shader::processShader(&vertexMergeShaderID,    vertexMergeShaderPath.c_str(),    GL_VERTEX_SHADER);
    Shader::processShader(&fragmentRule30ShaderID, fragmentRule30ShaderPath.c_str(), GL_FRAGMENT_SHADER);
    Shader::processShader(&fragmentMergeShaderID,  fragmentMergeShaderPath.c_str(),  GL_FRAGMENT_SHADER);

    /* Create programs. */
    rule30ProgramID = GL_CHECK(glCreateProgram());
    mergeProgramID  = GL_CHECK(glCreateProgram());

    /* Attach shaders. */
    GL_CHECK(glAttachShader(rule30ProgramID, vertexRule30ShaderID));
    GL_CHECK(glAttachShader(rule30ProgramID, fragmentRule30ShaderID));
    GL_CHECK(glAttachShader(mergeProgramID,  vertexMergeShaderID));
    GL_CHECK(glAttachShader(mergeProgramID,  fragmentMergeShaderID));

    /* Link programs. */
    GL_CHECK(glLinkProgram(rule30ProgramID));
    GL_CHECK(glLinkProgram(mergeProgramID) );

    /* Set up buffer objects. */
    GLuint boIDs[] = {0, 0, 0, 0};

    GL_CHECK(glGenBuffers(4, boIDs));

    linePositionBOID = boIDs[0];
    lineUVBOID       = boIDs[1];
    quadPositionBOID = boIDs[2];
    quadUVBOID       = boIDs[3];

    /* Set up framebuffer object. */
    GL_CHECK(glGenFramebuffers(1, &framebufferID));

    /* Set up VAO for line data. */
    GL_CHECK(glGenVertexArrays(1, &lineVAOID));
    GL_CHECK(glBindVertexArray(lineVAOID));

    /* Set up vertex attributes locations in "rule30" program. */
    GLint positionLocation = GL_CHECK(glGetAttribLocation(rule30ProgramID, "position"));
    GLint texCoordLocation = GL_CHECK(glGetAttribLocation(rule30ProgramID, "vertexTexCoord"));

    if (positionLocation == -1)
    {
        LOGD("Could not find position attribute in program [%d]", rule30ProgramID);
    }
    else
    {
        /* Fill buffers with line vertices attribute data. */
        GL_CHECK(glBindBuffer         (GL_ARRAY_BUFFER,   linePositionBOID));
        GL_CHECK(glBufferData         (GL_ARRAY_BUFFER,   sizeof(lineVertices), lineVertices, GL_STATIC_DRAW));
        GL_CHECK(glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CHECK(glEnableVertexAttribArray(positionLocation));
    }

    if (texCoordLocation == -1)
    {
        LOGD("Could not find model_tex_coord attribute in program [%d]", rule30ProgramID);
    }
    else
    {
        /* Fill buffers with line U/V attribute data. */
        GL_CHECK(glBindBuffer         (GL_ARRAY_BUFFER, lineUVBOID));
        GL_CHECK(glBufferData         (GL_ARRAY_BUFFER, sizeof(lineTextureCoordinates), lineTextureCoordinates, GL_STATIC_DRAW));
        GL_CHECK(glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CHECK(glEnableVertexAttribArray(texCoordLocation));
    }

    /* Set up VAO for quad data. */
    GL_CHECK(glGenVertexArrays(1, &quadVAOID));
    GL_CHECK(glBindVertexArray(quadVAOID));

    /* Set up vertex attributes locations in "merge" program. */
    positionLocation  = GL_CHECK(glGetAttribLocation(mergeProgramID, "position"));
    texCoordLocation  = GL_CHECK(glGetAttribLocation(mergeProgramID, "vertexTexCoord"));

    if (positionLocation == -1)
    {
        LOGD("Could not find position attribute in program [%d]", mergeProgramID);
    }
    else
    {
        /* Fill buffers with quad vertices attribute data. */
        GL_CHECK(glBindBuffer         (GL_ARRAY_BUFFER,   quadPositionBOID));
        GL_CHECK(glBufferData         (GL_ARRAY_BUFFER,   sizeof(quadVertices), quadVertices, GL_STATIC_DRAW));
        GL_CHECK(glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CHECK(glEnableVertexAttribArray(positionLocation));
    }

    if (texCoordLocation == -1)
    {
        LOGD("Could not find model_tex_coord attribute in program [%d]", mergeProgramID);
    }
    else
    {
        /* Fill buffers with quad U/V attribute data. */
        GL_CHECK(glBindBuffer         (GL_ARRAY_BUFFER, quadUVBOID));
        GL_CHECK(glBufferData         (GL_ARRAY_BUFFER, sizeof(quadTextureCoordinates), quadTextureCoordinates, GL_STATIC_DRAW));
        GL_CHECK(glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CHECK(glEnableVertexAttribArray(texCoordLocation));
    }

    /* Set line width to 1.5, to avoid rounding errors. */
    GL_CHECK(glLineWidth(1.5));

    return true;
}

/* Renders to the texture attached to a custom framebuffer, following the Rule 30. */
void performOffscreenRendering()
{
    static bool isRenderedForTheFirstTime = true;

    /* Array specifying the draw buffers to which render. */
    const GLuint offscreenFBODrawBuffers[] = {GL_COLOR_ATTACHMENT0};

    /* Set up uniform locations. */
    GLint inputTextureLocation        = GL_CHECK(glGetUniformLocation(rule30ProgramID, "inputTexture")        );
    GLint verticalOffsetLocation      = GL_CHECK(glGetUniformLocation(rule30ProgramID, "verticalOffset")      );
    GLint mvpMatrixLocation           = GL_CHECK(glGetUniformLocation(rule30ProgramID, "mvpMatrix")           );
    GLint inputVerticalOffsetLocation = GL_CHECK(glGetUniformLocation(rule30ProgramID, "inputVerticalOffset") );
    GLint inputNeighbourLocation      = GL_CHECK(glGetUniformLocation(rule30ProgramID, "inputNeighbour")      );

    /* Offset of the input line passed to the appropriate uniform. */
    float inputVerticalOffset = 0.0f; 

    /* Activate the first program. */
    GL_CHECK(glUseProgram(rule30ProgramID));
    GL_CHECK(glBindVertexArray(lineVAOID));

    /* Set up the framebuffer. */
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferID));

    if(isRenderedForTheFirstTime)
    {
        GL_CHECK(glDrawBuffers    (1, offscreenFBODrawBuffers));

        /* Pass data to uniforms. */
        GL_CHECK(glUniformMatrix4fv(mvpMatrixLocation,      1, GL_FALSE, modelViewProjectionMatrix.getAsArray()));
        GL_CHECK(glUniform1f       (inputNeighbourLocation, 1.0f / WINDOW_W)                                    );

        isRenderedForTheFirstTime = false;
    }

    /* Render each line, beginning from the 2nd one, using the input from the previous line.*/
    for (unsigned int y = 1; y <= WINDOW_H; ++y)
    {
        /* Even lines should be taken from the ping texture, odd from the pong. */
        bool  isEvenLineBeingRendered = (y % 2 == 0) ? (true) : (false);
        /* Vertical offset of the currently rendered line. */
        float verticalOffset          = (float) y / (float) WINDOW_H;

        /* Pass data to uniforms. */
        GL_CHECK(glUniform1f(verticalOffsetLocation,      verticalOffset));
        GL_CHECK(glUniform1f(inputVerticalOffsetLocation, inputVerticalOffset));

        if (isEvenLineBeingRendered)
        {
            GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,    GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingTextureID, 0));
            GL_CHECK(glUniform1i           (inputTextureLocation, pongTextureUnit)                                        );
        }
        else
        {
            GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,    GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pongTextureID, 0));
            GL_CHECK(glUniform1i           (inputTextureLocation, pingTextureUnit)                                        );
        }

        /* Drawing a horizontal line defined by 2 vertices. */
        GL_CHECK(glDrawArrays(GL_LINES, 0, 2));

        /* Update the input vertical offset after the draw call, so it points to the previous line. */
        inputVerticalOffset = verticalOffset;
    }

    /* Unbind the framebuffer. */
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

/* Renders to the back buffer. */
void renderToBackBuffer()
{
    static bool isRenderedForTheFirstTime = true;

    /* Set up uniform locations. */
    GLint mvpMatrixLocation      = GL_CHECK(glGetUniformLocation(mergeProgramID, "mvpMatrix")     );
    GLint pingTextureLocation    = GL_CHECK(glGetUniformLocation(mergeProgramID, "pingTexture")   );
    GLint pongTextureLocation    = GL_CHECK(glGetUniformLocation(mergeProgramID, "pongTexture")   );

    /* Activate the second program. */
    GL_CHECK(glUseProgram(mergeProgramID));
    GL_CHECK(glBindVertexArray(quadVAOID));

    if(isRenderedForTheFirstTime)
    {
        /* Pass data to uniforms. */
        GL_CHECK(glUniformMatrix4fv(mvpMatrixLocation,      1, GL_FALSE, modelViewProjectionMatrix.getAsArray()));
        GL_CHECK(glUniform1i       (pingTextureLocation,    0)                                                  );
        GL_CHECK(glUniform1i       (pongTextureLocation,    1)                                                  );

        isRenderedForTheFirstTime = false;
    }

    /* Draw a quad as a triangle strip definied by 4 vertices. */
    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

/* Complete rendering of one frame. */
void renderFrame()
{
    performOffscreenRendering();
    renderToBackBuffer();
}

/* Reset the textures, so a new pattern can be generated. */
void resetTextures()
{
    /* Delete existing texture data. */
    Texture::deleteTextureData(&pingTextureData);

    /* Create new texture data. */
    Texture::createTexture(WINDOW_W, WINDOW_H, 0, &pingTextureData);
    generateRule30Input   (WINDOW_W, WINDOW_H, 1, &pingTextureData);

    /*
     * Since texture objects have already been created, we can substitute ping image using glTexSubImage2D call.
     * Pong texture does not require reset, because its content depends entirely on the first line of the ping texture.
     */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WINDOW_W, WINDOW_H, GL_RED_INTEGER, GL_UNSIGNED_BYTE, pingTextureData));
}

int main(int argc, char **argv)
{
    /* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if (platform != 0)
    {
        /* Initialize windowing system. */
        platform->createWindow(WINDOW_W, WINDOW_H);

        /* Initialize EGL. */
        EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3);

        EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

        /* Initialize OpenGL-ES graphics subsystem. */
        if (setupGraphics())
        {
            /* Render frame for the first time. */
            renderFrame();

            /* Push the EGL surface color buffer to the native window. */
            eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
            
            /* Generic framework timer used to count the time interval for texture switch. */
            Timer timer;
            timer.reset();

            /* The rendering loop that draws the scene. */
            bool end = false;

            while(!end)
            {
                /* If something has happen to the window, end the sample. */
                #if !defined(__arm__)
                    MaliSDK::Platform::WindowStatus window_status = platform->checkWindow();
                    
                    if(window_status != Platform::WINDOW_IDLE)
                    {
                        end = true;
                    }
                #endif

                /* Reset input data every predefined amount of time. */
                if(timer.isTimePassed(resetTimeInterval))
                {
                    resetTextures();
                    renderFrame();

                    eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
                } 
            }

            /* Delete texture data. */
            Texture::deleteTextureData(&pingTextureData);

            /* Shut down EGL. */
            EGLRuntime::terminateEGL();

            /* Shut down windowing system. */
            platform->destroyWindow();

            /* Shut down the Platform object. */
            delete platform;
        }
        else
        {
            LOGE("Graphics setup failed");

            exit(1);
        }
    }
    else
    {
        LOGE("Could not create platform");

        exit(1);
    }
    return 0;
}
