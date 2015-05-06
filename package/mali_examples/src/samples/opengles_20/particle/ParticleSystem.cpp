/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * \file ParticleSystem.cpp
 * \brief A sample which shows how to create and draw a particle system.
 *
 * Uses a shader to calculate particle trajectories, particle
 * size and transparency.
 */

#include "ParticleSystem.h"
#include "DiscEmitter.h"

#include "Text.h"
#include "Texture.h"
#include "Shader.h"
#include "Timer.h"
#include "Matrix.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "Platform.h"
#include "EGLRuntime.h"

#include <EGL/egl.h>

#define WINDOW_W 800
#define WINDOW_H 600

#define PARTICLE_DATA_SIZE   9

/* Particle settings to play with */
/* Number of particles */
#define NUM_PARTICLES        2800
/* Radius of emitter disc */
#define EMITTER_RADIUS        0.1f
/* Maximum emission angle */
#define EMITTER_ANGLE        30.0f
/* Factor that multiplies the initial velocity of particles */
#define VELOCITY_FACTOR        0.5f
/* Factor that multiplies the lifetime of particles. */
#define LIFETIME_FACTOR        6.5f
/* Red component of particle base color. */
#define BASE_COLOUR_RED        0.5f
/* Green component of particle base color. */
#define BASE_COLOUR_GREEN    0.5f
/* Blue component of particle base color. */
#define BASE_COLOUR_BLUE    0.7f
/* Z component of gravity vector. */
#define GRAVITY_Z            -0.06f

using std::string;
using namespace MaliSDK;

/* Asset directories and filenames. */
string resourceDirectory = "assets/";
string vertexShaderFilename = "Particle_system.vert";
string fragmentShaderFilename = "Particle_system.frag";
string textureFilename =  "smoke";
string imageExtension = ".raw";

/* Texture. */
#define TEXTURE_WIDTH 128
#define TEXTURE_HEIGHT 128

/* A text object to draw text on the screen. */
Text *text;

/* Structure to hold all shader data. */
typedef struct
{
    /* Handle to a program object. */
    GLuint programID;

    /* Handle to texture object. */
    GLuint textureID;

    /* Attribute locations. */
    GLint iLocPosition;
    GLint iLocVelocity;
    GLint iLocParticleTimes;

    /* Uniform locations. */
    GLint iLocMVP;
    GLint iLocGravity;
    GLint iLocColour;

    /*Sampler location. */
    GLint iLocSampler;

    /* Elapsed time per frame. */
    float frameTime;

    /* Transformation matrix. */
    Matrix modelViewPerspective;

    /* Gravity. */
    Vec3f gravity;

    /* Base colour. */
    Vec3f baseColour;

    /* Particle data array. */
    float particleData[NUM_PARTICLES * PARTICLE_DATA_SIZE];

} UserData;

UserData *userData = NULL;

/* Initialization of all particle data. */
void initializeParticleDataArray()
{
    /* Use the above particle setting #defines to change particle parameters. */

    DiscEmitter emitter(EMITTER_RADIUS, EMITTER_ANGLE);
    Particle particle;
    int index;

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        emitter.getParticle(particle);

        index = PARTICLE_DATA_SIZE*i;

        userData->particleData[index]        = particle.initPos.x;
        userData->particleData[index + 1]    = particle.initPos.y;
        userData->particleData[index + 2]    = particle.initPos.z;

        userData->particleData[index + 3]    = VELOCITY_FACTOR*particle.initVel.x;
        userData->particleData[index + 4]    = VELOCITY_FACTOR*particle.initVel.y;
        userData->particleData[index + 5]    = VELOCITY_FACTOR*particle.initVel.z;

        userData->particleData[index + 6]    = 4.0f*particle.delay;
        userData->particleData[index + 7]    = LIFETIME_FACTOR*particle.lifetime;
        /* Initial particle age. */
        userData->particleData[index + 8]    = 0.0f;
    }
}

/* Update of particle age. */
void updateParticleAge()
{
    int temp;

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        temp = PARTICLE_DATA_SIZE * i;

        /* Increment particle age. */
        userData->particleData[temp + 8] += userData->frameTime;
        if( userData->particleData[temp + 8] > userData->particleData[temp + 7])
        {
            /* Reset particle age. */
            userData->particleData[temp + 8] = 0.0f;
        }
    }
}

void initialiseTextureFromRawGreyscaleAlphaFile(const string texturePath )
{
    GL_CHECK(glGenTextures(1, &userData->textureID));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, userData->textureID));

    string mainTexturePath = texturePath + imageExtension;

    unsigned char *textureData = NULL;

    FILE * file = NULL;

    /* Open texture file. */
    file = fopen( mainTexturePath.c_str(), "rb" );

    if(file)
    {
        /* Gray scale image. */
        int numBytes = TEXTURE_WIDTH * TEXTURE_HEIGHT;

        /* Allocate data buffer. */
        textureData = (unsigned char *) malloc( numBytes * sizeof(unsigned char));

        if(!textureData)
        {
            fclose(file);
            LOGE("Could not allocate memory for texture data.");
            exit(1);
        }

        /* Read texture file. */
        fread(textureData, numBytes, 1, file);
        fclose(file);
    }
    else
    {
        LOGE("Could not open texture file %s.", mainTexturePath.c_str());
        exit(1);
    }

    GL_CHECK(glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData));
    free(textureData);

    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

bool setupGraphics(int width, int height)
{
    /* [Create userData] */
    /* Create structure to hold all data. */
    userData = new UserData;
    /* [Create userData] */

    /* Ask OpenGL ES for the maximum number of vertex attributes supported. */
    GLint maxVertexAttribs = 0;
    GL_CHECK(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs));
    LOGD("GL_MAX_VERTEX_ATTRIBS = %d", maxVertexAttribs);

    /* [Initialize shaders] */
    LOGD("setupGraphics(%d, %d)", width, height);

    /* Full paths to the shader and texture files. */
    string vertexShaderPath = resourceDirectory + vertexShaderFilename;
    string fragmentShaderPath = resourceDirectory + fragmentShaderFilename;
    string texturePath = resourceDirectory + textureFilename;

    GLuint vertexShaderID = 0;
    GLuint fragmentShaderID = 0;
    /* [Initialize shaders] */

    /* [Intialize OpenGL ES] */
    /* Initialize OpenGL ES */
    GL_CHECK(glEnable(GL_BLEND));
    /* Alpha blending. */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Initialize the Text object and add some text. */
    text = new Text(resourceDirectory.c_str(), width, height);
    char buffer [50];
    sprintf (buffer, "Simple Particle System: %d particles", NUM_PARTICLES);
    text->addString(10, 10, buffer, 255, 255, 0, 255);

    initialiseTextureFromRawGreyscaleAlphaFile(texturePath);

    /* [Intialize OpenGL ES] */
    /* [Process shaders] */
    /* Process shaders. */
    Shader::processShader(&vertexShaderID, vertexShaderPath.c_str(), GL_VERTEX_SHADER);
    LOGD("vertexShaderID = %d", vertexShaderID);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);
    LOGD("fragmentShaderID = %d", fragmentShaderID);

    userData->programID = GL_CHECK(glCreateProgram());

    GL_CHECK(glAttachShader(userData->programID, vertexShaderID));
    GL_CHECK(glAttachShader(userData->programID, fragmentShaderID));
    GL_CHECK(glLinkProgram(userData->programID));
    GL_CHECK(glUseProgram(userData->programID));
    /* [Process shaders] */

    /* [Initialize particles] */
    initializeParticleDataArray();
    /* [Initialize particles] */

    /* [Attribute location] */
    /* Get attribute locations. */
    GL_CHECK(userData->iLocPosition = glGetAttribLocation(userData->programID, "a_v3Position"));
    LOGD("glGetAttribLocation(\"a_v3Position\") = %d\n", userData->iLocPosition);

    GL_CHECK(userData->iLocVelocity = glGetAttribLocation(userData->programID, "a_v3Velocity"));
    LOGD("glGetAttribLocation(\"a_v3Velocity\") = %d\n", userData->iLocVelocity);

    GL_CHECK(userData->iLocParticleTimes = glGetAttribLocation(userData->programID, "a_v3ParticleTimes"));
    LOGD("glGetAttribLocation(\"a_v3ParticleTimes\") = %d\n", userData->iLocParticleTimes);

    /* Get uniform locations. */
    userData->iLocMVP = GL_CHECK(glGetUniformLocation(userData->programID, "mvp"));
    LOGD("glGetUniformLocation(\"mvp\") = %d\n", userData->iLocMVP);

    userData->iLocGravity = GL_CHECK(glGetUniformLocation(userData->programID, "u_v3gravity"));
    LOGD("glGetUniformLocation(\"u_v3gravity\") = %d\n", userData->iLocGravity);

    userData->iLocSampler = GL_CHECK(glGetUniformLocation(userData->programID, "s_texture"));
    if (userData->iLocSampler == -1)
    {
        LOGD("Warning: glGetUniformLocation(\"s_texture\") = %d\n", userData->iLocSampler);
    }
    else
    {
        GL_CHECK(glUniform1i(userData->iLocSampler, 0));
    }


    userData->iLocColour = GL_CHECK(glGetUniformLocation(userData->programID, "u_v3colour"));
    LOGD("glGetUniformLocation(\"u_v3colour\") = %d\n", userData->iLocColour);
    /* [Attribute location] */

    /* [Viewport initialization] */
    GL_CHECK(glViewport(0, 0, width, height));

    /* Set clear screen color. */
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    /* [Viewport initialization] */


    /* [Data initialization] */
    Matrix modelView = Matrix::createRotationX(-90);
    /* Pull the camera back from emitter. */
    modelView[14] -= 2.0;
    /* Pull the camera down from emitter. */
    modelView[13] -= 0.5;

    Matrix perspective = Matrix::matrixPerspective(45.0f, (float)width/(float)height, 0.01f, 100.0f);
    userData->modelViewPerspective = perspective * modelView;

    /* Define constant acceleration vector of the field where the particles are moving. */
    userData->gravity.x = userData->gravity.y = 0.0f;
    userData->gravity.z = GRAVITY_Z;

    /* Define particle base color. */
    userData->baseColour.x = BASE_COLOUR_RED;
    userData->baseColour.y = BASE_COLOUR_GREEN;
    userData->baseColour.z = BASE_COLOUR_BLUE;
    /* [Data initialization] */

    return true;
}

/* [Render frame] */
void renderFrame(void)
{
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    updateParticleAge();

    GL_CHECK(glUseProgram(userData->programID));

    /* Pass particle data to the shader. */
    GL_CHECK(glVertexAttribPointer(userData->iLocPosition, 3, GL_FLOAT, GL_FALSE, PARTICLE_DATA_SIZE * sizeof(GLfloat), userData->particleData));
    GL_CHECK(glVertexAttribPointer(userData->iLocVelocity, 3, GL_FLOAT, GL_FALSE, PARTICLE_DATA_SIZE * sizeof(GLfloat), &userData->particleData[3]));
    GL_CHECK(glVertexAttribPointer(userData->iLocParticleTimes, 3, GL_FLOAT, GL_FALSE, PARTICLE_DATA_SIZE * sizeof(GLfloat), &userData->particleData[6]));

    GL_CHECK(glEnableVertexAttribArray(userData->iLocPosition));
    GL_CHECK(glEnableVertexAttribArray(userData->iLocVelocity));
    GL_CHECK(glEnableVertexAttribArray(userData->iLocParticleTimes));

    GL_CHECK(glUniformMatrix4fv(userData->iLocMVP, 1, GL_FALSE, userData->modelViewPerspective.getAsArray()));
    GL_CHECK(glUniform3fv(userData->iLocGravity, 1, (GLfloat *)&userData->gravity));
    GL_CHECK(glUniform3fv(userData->iLocColour, 1, (GLfloat *)&userData->baseColour));

    /* Bind the texture. */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, userData->textureID));

    /* Set the sampler to point at the 0th texture unit. */
    GL_CHECK(glUniform1i(userData->iLocSampler, 0));

    /* Draw particles. */
    GL_CHECK(glDrawArrays(GL_POINTS, 0, NUM_PARTICLES));

    /* Draw fonts. */
    text->draw();
}
/* [Render frame] */

int main(void)
{
    /* [Intialize Platform] */
    /* Initialize the Platform object for platform specific functions. */
    Platform *platform = Platform::getInstance();
    /* [Intialize Platform] */

    /* [Intialize Window] */
    /* Initialize windowing system. */
    platform->createWindow(WINDOW_W, WINDOW_H);
    /* [Intialize Window] */

    /* [Intialize EGL] */
    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES2);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));
    /* [Intialize EGL] */

    /* Initialize OpenGL ES graphics subsystem. */
    setupGraphics(WINDOW_W, WINDOW_H);

    /* Timer variable to calculate FPS. */
    Timer fpsTimer;
    fpsTimer.reset();

    bool end = false;
    /* The rendering loop to draw the scene. */
    while(!end)
    {
        userData->frameTime = fpsTimer.getInterval();

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

        /* Render a single frame. */
        renderFrame();

        /*
         * Push the EGL surface color buffer to the native window.
         * Causes the rendered graphics to be displayed on screen.
         */
        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    /* [Release resources] */
    /* Shut down OpenGL ES. */

    /* Shut down Text. */
    delete text;

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;
    /* [Release resources] */

    return 0;
}
