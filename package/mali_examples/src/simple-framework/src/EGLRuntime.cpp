/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "EGLRuntime.h"
#include "Platform.h"

#include <cstdlib>

namespace MaliSDK
{
    Platform* platform = Platform::getInstance();

    EGLDisplay EGLRuntime::display;
    EGLContext EGLRuntime::context;
    EGLSurface EGLRuntime::surface;
    EGLConfig EGLRuntime::config;
    bool EGLRuntime::disableAntiAliasing = false;

    #if defined(__arm__)
    const int EGLRuntime::redSize = 5;
    const int EGLRuntime::greenSize = 6;
    const int EGLRuntime::blueSize = 5;
    const int EGLRuntime::bufferSize = 16;
    #else
    const int EGLRuntime::redSize = 8;
    const int EGLRuntime::greenSize = 8;
    const int EGLRuntime::blueSize = 8;
    const int EGLRuntime::bufferSize = 32;
    #endif
    
    EGLint EGLRuntime::configAttributes[] =
    {
        /* DO NOT MODIFY. */
        /* These attributes are in a known order and may be re-written at initialization according to application requests. */
        EGL_SAMPLES,             4,

        EGL_RED_SIZE,            redSize,
        EGL_GREEN_SIZE,          greenSize,
        EGL_BLUE_SIZE,           blueSize,
        EGL_ALPHA_SIZE,          0,

        EGL_BUFFER_SIZE,         bufferSize,
        EGL_STENCIL_SIZE,        0,
        EGL_RENDERABLE_TYPE,     0,    /* This field will be completed according to application request. */

        EGL_SURFACE_TYPE,        EGL_WINDOW_BIT | EGL_PIXMAP_BIT,

        EGL_DEPTH_SIZE,          16,

        /* MODIFY BELOW HERE. */
        /* If you need to add or override EGL attributes from above, do it below here. */

        EGL_NONE
    };

    EGLint EGLRuntime::contextAttributes[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 0, /* This field will be completed according to application request. */
        EGL_NONE
    };

    /**
     * Using the defaults (EGL_RENDER_BUFFER = EGL_BACK_BUFFER).
     */
    EGLint EGLRuntime::windowAttributes[] =
    {
        EGL_NONE
        /*
         * Uncomment and remove EGL_NONE above to specify EGL_RENDER_BUFFER value to eglCreateWindowSurface.
         * EGL_RENDER_BUFFER,     EGL_BACK_BUFFER,
         * EGL_NONE
         */
    };

    void EGLRuntime::initializeEGL(OpenGLESVersion requestedAPIVersion)
    {
        EGLConfig *configsArray = NULL;
        EGLint numberOfConfigs = 0;

        EGLBoolean success = EGL_FALSE;

    #if defined(_WIN32)
        /* Win32 */
        display = eglGetDisplay(platform->deviceContext);
    #elif defined(__arm__)
        /* Linux on ARM */
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    #else
        /* Desktop Linux */
        platform->display = XOpenDisplay(NULL);
        display = eglGetDisplay(platform->display);
    #endif

        if(display == EGL_NO_DISPLAY)
        {
            EGLint error = eglGetError();
            LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
            LOGE("No EGL Display available at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }

        /* Initialize EGL. */
        success = eglInitialize(display, NULL, NULL);
        if(success != EGL_TRUE)
        {
            EGLint error = eglGetError();
            LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
            LOGE("Failed to initialize EGL at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }

        /* Depending on app-requested EGL attributes, tweak the attributes we pass to EGL. */
        if(requestedAPIVersion == OPENGLES1)
        {
            configAttributes[15] = EGL_OPENGL_ES_BIT;
            contextAttributes[1] = 1;
        }
        else if(requestedAPIVersion == OPENGLES2)
        {
            configAttributes[15] = EGL_OPENGL_ES2_BIT;
            contextAttributes[1] = 2;
        }
        /* 
         * Despite the fact an OpenGL ES 3.0 config is required, we request configs using the OpenGL_ES2_BIT.
         * At the time of writing there is no OpenGL_ES3_BIT, and so platforms return 
         * OpenGL ES 3.0 configs with the OpenGL_ES2_BIT set.
         * We request a context with EGL_CONTEXT_CLIENT_VERSION of 3 (OpenGL ES 3.0) which will ensure that 
         * OpenGL ES 3.0 features are supported.
         */
        else if (requestedAPIVersion == OPENGLES3)
        {
            configAttributes[15] = EGL_OPENGL_ES2_BIT;
            contextAttributes[1] = 3;
        }
        
        /* Disable AntiAliasing, useful if it's not supported on a platform. */
        if (disableAntiAliasing)
        {
            configAttributes[1] = EGL_DONT_CARE;
        }

        /* Enumerate available EGL configurations which match or exceed our required attribute list. */
        success = eglChooseConfig(display, configAttributes, NULL, 0, &numberOfConfigs);
        if(success != EGL_TRUE)
        {
            EGLint error = eglGetError();
            LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
            LOGE("Failed to enumerate EGL configs at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }

        LOGD("Number of configs found is %d\n", numberOfConfigs);

        /* Allocate space for all EGL configs available and get them. */
        configsArray = (EGLConfig *)calloc(numberOfConfigs, sizeof(EGLConfig));
        if(configsArray == NULL)
        {
            LOGE("Out of memory at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }
        success = eglChooseConfig(display, configAttributes, configsArray, numberOfConfigs, &numberOfConfigs);
        if(success != EGL_TRUE)
        {
            EGLint error = eglGetError();
            LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
            LOGE("Failed to enumerate EGL configs at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }

        /*
         * Loop through the EGL configs to find a color depth match.
         * Note: This is necessary, since EGL considers a higher color depth than requested to be 'better'
         * even though this may force the driver to use a slow color conversion blitting routine. 
         */
        bool matchFound = false;
        int matchingConfig = -1;
        for(int configsIndex = 0; (configsIndex < numberOfConfigs) && !matchFound; configsIndex++)
        {
            EGLint attributeValue = 0;

            success = eglGetConfigAttrib(display, configsArray[configsIndex], EGL_RED_SIZE, &attributeValue);
            if(success != EGL_TRUE)
            {
                EGLint error = eglGetError();
                LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
                LOGE("Failed to get EGL attribute at %s:%i\n", __FILE__, __LINE__);
                exit(1);
            }

            if(attributeValue == redSize)
            {
                success = eglGetConfigAttrib(display, configsArray[configsIndex], EGL_GREEN_SIZE, &attributeValue);
                if(success != EGL_TRUE)
                {
                    EGLint error = eglGetError();
                    LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
                    LOGE("Failed to get EGL attribute at %s:%i\n", __FILE__, __LINE__);
                    exit(1);
                }

                if(attributeValue == greenSize)
                {
                    success = eglGetConfigAttrib(display, configsArray[configsIndex], EGL_BLUE_SIZE, &attributeValue);
                    if(success != EGL_TRUE)
                    {
                        EGLint error = eglGetError();
                        LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
                        LOGE("Failed to get EGL attribute at %s:%i\n", __FILE__, __LINE__);
                        exit(1);
                    }

                    if(attributeValue == blueSize) 
                    {
                        matchFound = true;
                        matchingConfig = configsIndex;
                    }
                }
            }
        }
        if(!matchFound)
        {
            LOGE("Failed to find matching EGL config at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }
        /* Copy the configsArray to our static variables. */
        config = configsArray[matchingConfig];

    #if defined(__linux__) && !defined(__arm__)
        ((DesktopLinuxPlatform*)(platform))->createX11Window();
    #endif

        /* Create a surface. */
        surface = eglCreateWindowSurface(display, configsArray[matchingConfig], (EGLNativeWindowType)(platform->window), windowAttributes);
        if(surface == EGL_NO_SURFACE)
        {
            EGLint error = eglGetError();
            LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
            LOGE("Failed to create EGL surface at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }

        /* Unconditionally bind to OpenGL ES API as we exit this function, since it's the default. */
        eglBindAPI(EGL_OPENGL_ES_API);
        
        context = eglCreateContext(display, configsArray[matchingConfig], EGL_NO_CONTEXT, contextAttributes);
        if(context == EGL_NO_CONTEXT)
        {
            EGLint error = eglGetError();
            LOGE("eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
            LOGE("Failed to create EGL context at %s:%i\n", __FILE__, __LINE__);
            exit(1);
        }

        free(configsArray);
        configsArray = NULL;
    }

    void EGLRuntime::terminateEGL(void)
    {
        /* Shut down EGL. */
        eglBindAPI(EGL_OPENGL_ES_API);
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
    }
}