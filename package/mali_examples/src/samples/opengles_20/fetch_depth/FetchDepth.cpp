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
 * \file FetchDepth.cpp
 * \brief A simple rotating cube.
 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <string.h>

#include "FetchDepth.h"
#include "Text.h"
#include "Shader.h"
#include "Texture.h"
#include "Matrix.h"
#include "Timer.h"
#include "Platform.h"
#include "EGLRuntime.h"

#define WINDOW_W 800
#define WINDOW_H 600

using std::string;
using namespace MaliSDK;

/* Asset directories and filenames. */
string resourceDirectory              = "assets/";
string vertexShaderCubeFilename       = "FetchDepth_cube.vert";
string fragmentShaderCubeFilename     = "FetchDepth_cube.frag";
string vertexShaderGlassFilename      = "FetchDepth_glass.vert";
string fragmentShaderGlassFilename    = "FetchDepth_glass.frag";
string fragmentShaderGlassRttFilename = "FetchDepth_glass_rtt.frag";

/* Shader variables. */
GLuint programCubeID;
GLint iLocCubePosition;
GLint iLocCubeColor;
GLint iLocCubeMVP;

GLuint programGlassID;
GLint iLocGlassPosition;
GLint iLocGlassColor;
GLint iLocGlassMVP;
GLint iLocNear;
GLint iLocFar;
GLint iLocGlassMaterial;

int windowWidth = -1;
int windowHeight = -1;

bool supportFramebufferFetch = false;
GLuint fbo = 0;
GLint iLocViewportSize;
GLint iLocCubeDepthTexture;

/* A text object to draw text on the screen. */
Text *text;

bool setupGraphics(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    /* Full paths to the shader and texture files */
    string vertexShaderCubePath    = resourceDirectory + vertexShaderCubeFilename;
    string fragmentShaderCubePath  = resourceDirectory + fragmentShaderCubeFilename;
    string vertexShaderGlassPath   = resourceDirectory + vertexShaderGlassFilename;
    string fragmentShaderGlassPath;

    if (supportFramebufferFetch)
    {
        fragmentShaderGlassPath = resourceDirectory + fragmentShaderGlassFilename;
    }
    else
    {
        /* Render to texture if we don't support framebuffer fetch. */

        fragmentShaderGlassPath = resourceDirectory + fragmentShaderGlassRttFilename;

        GL_CHECK(glGenFramebuffers(1, &fbo));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

        GLuint textureId;
        GL_CHECK(glGenTextures(1, &textureId));

        /* Texture unit 1 for depth buffer */
        GL_CHECK(glActiveTexture(GL_TEXTURE1));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureId));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureId, 0));


        if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
        {
            LOGI("glCheckFramebufferStatue failed\n");
        }
    }

    /* Initialize OpenGL ES. */
    GL_CHECK(glEnable(GL_BLEND));
    /* Should do: src * (src alpha) + dest * (1-src alpha). */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Initialize the Text object and add some text. */
    text = new Text(resourceDirectory.c_str(), windowWidth, windowHeight);
    text->addString(0, 0, "Simple FetchDepth Example", 255, 255, 0, 255);

    /* Process shaders. */
    GLuint fragmentShaderCubeID, vertexShaderCubeID;
    GLuint fragmentShaderGlassID, vertexShaderGlassID;
    Shader::processShader(&vertexShaderCubeID,    vertexShaderCubePath.c_str(),    GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderCubeID,  fragmentShaderCubePath.c_str(),  GL_FRAGMENT_SHADER);
    Shader::processShader(&vertexShaderGlassID,   vertexShaderGlassPath.c_str(),   GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderGlassID, fragmentShaderGlassPath.c_str(), GL_FRAGMENT_SHADER);

    /* Create programID (ready to attach shaders) */
    programCubeID = GL_CHECK(glCreateProgram());
    programGlassID = GL_CHECK(glCreateProgram());

    /* Attach shaders and link programID */
    GL_CHECK(glAttachShader(programCubeID, vertexShaderCubeID));
    GL_CHECK(glAttachShader(programCubeID, fragmentShaderCubeID));
    GL_CHECK(glLinkProgram(programCubeID));
    GL_CHECK(glAttachShader(programGlassID, vertexShaderGlassID));
    GL_CHECK(glAttachShader(programGlassID, fragmentShaderGlassID));
    GL_CHECK(glLinkProgram(programGlassID));


    /* Get attribute locations of non-fixed attributes like color and texture coordinates. */
    iLocCubePosition = GL_CHECK(glGetAttribLocation(programCubeID, "av4position"));
    iLocCubeColor = GL_CHECK(glGetAttribLocation(programCubeID, "av3color"));

    LOGD("iLocCubePosition = %i\n", iLocCubePosition);
    LOGD("iLocCubeColor    = %i\n", iLocCubeColor);

    /* Get uniform locations */
    iLocCubeMVP = GL_CHECK(glGetUniformLocation(programCubeID, "mvp"));

    LOGD("iLocCubeMVP      = %i\n", iLocCubeMVP);


    LOGD("programCubeID    = %i\n", programCubeID);
    LOGD("programGlassID   = %i\n", programGlassID);

    /* Get attribute locations of non-fixed attributes like color and texture coordinates. */
    iLocGlassPosition = GL_CHECK(glGetAttribLocation(programGlassID, "av4position"));
    iLocGlassColor = GL_CHECK(glGetAttribLocation(programGlassID, "av4color"));

    LOGD("iLocGlassPosition = %i\n", iLocGlassPosition);
    LOGD("iLocGlassColor    = %i\n", iLocGlassColor);

    /* Get uniform locations */
    iLocGlassMVP = GL_CHECK(glGetUniformLocation(programGlassID, "mvp"));
    iLocNear = GL_CHECK(glGetUniformLocation(programGlassID, "near"));
    iLocFar = GL_CHECK(glGetUniformLocation(programGlassID, "far"));
    iLocGlassMaterial = GL_CHECK(glGetUniformLocation(programGlassID, "glassMaterial"));

    LOGD("iLocGlassMVP      = %i\n", iLocGlassMVP);
    LOGD("iLocNear          = %i\n", iLocNear);
    LOGD("iLocFar           = %i\n", iLocFar);
    LOGD("iLocGlassMaterial = %i\n", iLocGlassMaterial);

    if (!supportFramebufferFetch)
    {
        iLocViewportSize     = GL_CHECK(glGetUniformLocation(programGlassID, "viewportSize"));
        iLocCubeDepthTexture = GL_CHECK(glGetUniformLocation(programGlassID, "cubeDepthTexture"));

        LOGD("iLocViewportSize     = %i\n", iLocViewportSize);
        LOGD("iLocCubeDepthTexture = %i\n", iLocCubeDepthTexture);
    }

    GL_CHECK(glUseProgram(programCubeID));

    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    /* Set clear screen color. */
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

    return true;
}

void renderFrame(void)
{
    /* Draw Cube. */
    GL_CHECK(glUseProgram(programCubeID));

    /* Enable attributes for position, color and texture coordinates etc. */
    GL_CHECK(glEnableVertexAttribArray(iLocCubePosition));
    GL_CHECK(glEnableVertexAttribArray(iLocCubeColor));

    /* Populate attributes for position, color and texture coordinates etc. */
    GL_CHECK(glVertexAttribPointer(iLocCubePosition, 3, GL_FLOAT, GL_FALSE, 0, verticesCube));
    GL_CHECK(glVertexAttribPointer(iLocCubeColor, 3, GL_FLOAT, GL_FALSE, 0, colorsCube));

    static float angleY = 0;
    /*
     * Do some rotation with Euler angles. It is not a fixed axis as
     * quaternions would be, but the effect is nice.
     */
    Matrix modelView = Matrix::createRotationY(angleY);


    /* Initialize vectors that will be passed to matrixCameralookAt function. */
    Vec3f eyeVector   = {0.3, 0.1, 2.5};
    Vec3f lookAtPoint = {0.0, -1.0, 0.0};
    Vec3f upVector    = {0.0, 1.0, 0.0};

    /* Calculate view matrix. */
    Matrix viewMatrix = Matrix::matrixCameraLookAt(eyeVector, lookAtPoint, upVector);
    modelView = viewMatrix * modelView;

    const GLfloat fNear         = 0.01;
    const GLfloat fFar          = 3.0;
    const GLfloat glassMaterial = 1.8;

    Matrix perspective = Matrix::matrixPerspective(45.0, windowWidth/(float)windowHeight, fNear, fFar);
    Matrix modelViewPerspective = perspective * modelView;

    GL_CHECK(glUniformMatrix4fv(iLocCubeMVP, 1, GL_FALSE, modelViewPerspective.getAsArray()));

    /* Update cube's rotation angles for animating. */
    angleY += 1;

    if(angleY >= 360) angleY -= 360;
    if(angleY < 0) angleY += 360;

    if (supportFramebufferFetch)
    {
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT |
                         GL_DEPTH_BUFFER_BIT |
                         GL_STENCIL_BUFFER_BIT));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));
    }
    else
    {
        /* Render to texture */
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT |
                         GL_DEPTH_BUFFER_BIT |
                         GL_STENCIL_BUFFER_BIT));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));

        /* Render to screen */
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT |
                         GL_DEPTH_BUFFER_BIT |
                         GL_STENCIL_BUFFER_BIT));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));
    }


    /* Draw Glass. */
    GL_CHECK(glUseProgram(programGlassID));

    /* Enable attributes for position, color and texture coordinates etc. */
    GL_CHECK(glEnableVertexAttribArray(iLocGlassPosition));
    GL_CHECK(glEnableVertexAttribArray(iLocGlassColor));

    /* Populate attributes for position, color and texture coordinates etc. */
    GL_CHECK(glVertexAttribPointer(iLocGlassPosition, 3, GL_FLOAT, GL_FALSE, 0, verticesGlass));
    GL_CHECK(glVertexAttribPointer(iLocGlassColor, 4, GL_FLOAT, GL_FALSE, 0, colorsGlass));

    modelView = Matrix::createTranslation(0.5, 0.1, 0.8);
    modelView = viewMatrix * modelView;
    modelViewPerspective = perspective * modelView;

    GL_CHECK(glUniformMatrix4fv(iLocGlassMVP, 1, GL_FALSE, modelViewPerspective.getAsArray()));
    GL_CHECK(glUniform1f(iLocNear, fNear));
    GL_CHECK(glUniform1f(iLocFar, fFar));
    GL_CHECK(glUniform1f(iLocGlassMaterial, glassMaterial));

    if (!supportFramebufferFetch)
    {
        glUniform2f(iLocViewportSize, (float)windowWidth, (float)windowHeight);
        glUniform1i(iLocCubeDepthTexture, 1);
    }

    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Draw any text. */
    text->draw();
}

int main(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();
    /* Initialize windowing system. */
    platform->createWindow(WINDOW_W, WINDOW_H);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES2);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

    /* Check if support ARM_shader_framebuffer_fetch_depth_stencil extensions */
    const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
    if (strstr(extensions, "GL_ARM_shader_framebuffer_fetch_depth_stencil"))
    {
        supportFramebufferFetch = true;
    }
    else if (!strstr(extensions, "GL_OES_depth_texture"))
    {
        LOGI("OpenGL ES 2.0 implementation neither support ARM_shader_framebuffer_fetch_depth_stencil nor support OES_depth_texture extension.");
        exit(1);
    }

    /* Initialize OpenGL ES graphics subsystem. */
    setupGraphics(WINDOW_W, WINDOW_H);

    /* Timer variable to calculate FPS. */
    Timer fpsTimer;
    fpsTimer.reset();

    bool end = false;
    /* The rendering loop to draw the scene. */
    while(!end)
    {
        /* If something has happened to the window, end the sample. */
        if(platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            end = true;
        }

        /* Calculate FPS. */
        float fFPS = fpsTimer.getFPS();
        if(fpsTimer.isTimePassed(1.0f))
        {
            LOGI("FPS:\t%.1f\n", fFPS);
        }

        /* Render a single frame */
        renderFrame();

        /*
         * Push the EGL surface color buffer to the native window.
         * Causes the rendered graphics to be displayed on screen.
         */
        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    /* Shut down OpenGL ES. */
    /* Shut down Text. */
    delete text;

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;

    return 0;
}
