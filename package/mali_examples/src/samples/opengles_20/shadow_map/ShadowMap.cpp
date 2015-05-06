/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#include "mCommon.h"

#include "MAnimation.h"
#include "MImageTGA.h"
#include "MPathsManager.h"
#include "MGeometryComplex.h"
#include "MGeometryCone.h"
#include "MGeometryRectangle.h"
#include "MGeometrySphere.h"
#include "MGeometryTorus.h"
#include "MRendererProgram.h"
#include "MDRRendererTexture.h"
#include "MDRRenderTarget.h"

#include <iostream>

#include "3d.h"

#define WINDOW_W 800
#define WINDOW_H 600

#define VERTEX_SHADER_FILE          "ShadowMap_shadowmap.vert"
#define FRAGMENT_SHADER_FILE        "ShadowMap_shadowmap.frag"
#define VERTEX_SHADER_SHADOW_FILE   "ShadowMap_shadow.vert"
#define FRAGMENT_SHADER_SHADOW_FILE "ShadowMap_shadow.frag"
#define VERTEX_SHADER_SOLID_FILE    "ShadowMap_solid.vert"
#define FRAGMENT_SHADER_SOLID_FILE  "ShadowMap_solid.frag"
#define VERTEX_SHADER_FLARE_FILE    "ShadowMap_flare.vert"
#define FRAGMENT_SHADER_FLARE_FILE  "ShadowMap_flare.frag"
#define FLARE_IMAGE_FILE            "ShadowMap_Flare.tga"

using namespace std;
using namespace MaliSDK;

/* Declare variables. */
MRendererProgram* theProgramSolid = NULL;
MRendererProgram* theProgramShadow = NULL;
MRendererProgram* theProgramShadowMap = NULL;
MRendererProgram* theProgramFlare = NULL;
MAnimation1f theCamRotYAnim;
MAnimation1f theLightRotYAnim;
MTime theTime;

struct timeval sNow;

/* Animation variables. */
struct timeval sLastAnim1;
struct timeval sLastAnim2;

/* Scene transformation matrices. */
Matrix sMatLightM;
Matrix sMatLightV;
Matrix sMatLightProj;

Matrix sMatSceneV;
Matrix sMatSceneProj;
Matrix sMatCameraV;

Matrix sMatIdentity;

MGeometrySphere* theGeometrySphere = NULL;
MGeometryTorus* theGeometryTorus = NULL;
MGeometryCone* theGeometryCone = NULL;
MGeometryComplex* theGeometryComplex = NULL;
MGeometryRectangle* theGeometryRectangle = NULL;

MVector4f theGlobalAmbient;

MDRRendererTexture* theTextureFlare = NULL;
MDRRenderTarget* theRT = NULL;


/* FPS variables. */
struct timeval sLastFPS;
unsigned int uiFrame = 0;
long int lElapsedFPS = 0L;

int g_iWindowW = -1;
int g_iWindowH = -1;
int g_iShadowW = -1;
int g_iShadowH = -1;

bool setupGraphics(int width, int height)
{
    g_iWindowW = width;
    g_iWindowH = height;

    g_iShadowW = width;
    g_iShadowH = height;

    /* Allocate global variables */
    theProgramSolid = new MRendererProgram;
    theProgramShadow = new MRendererProgram;
    theProgramShadowMap = new MRendererProgram;
    theProgramFlare = new MRendererProgram;
    //
    theGeometrySphere = new MGeometrySphere;
    theGeometryTorus = new MGeometryTorus;
    theGeometryCone = new MGeometryCone;
    theGeometryComplex = new MGeometryComplex;
    theGeometryRectangle = new MGeometryRectangle;
    theTextureFlare = new MDRRendererTexture;
    /*
     * If from some reason you didn't want to use FBO, then swap the MODE_FBO
     * with the MODE_COPY_TEXTURE enum value, which is commented out.
     */
    theRT = new MDRRenderTarget(MDRRenderTarget::MODE_FBO /* MDRRenderTarget::MODE_COPY_TEXTURE */);


    /*
     * Set default global ambient. The vector is going to be changed in every frame rendering
     * depending on light position.
     */
    theGlobalAmbient.set(0.1f, 0.1f, 0.1f, 1.0f);

    /* Define animations. */
    theCamRotYAnim.setMode(MAnimation1f::PLAY_MODE_REPEAT);
    theCamRotYAnim.setKey(0.0f, MTime(0.0f));
    theCamRotYAnim.setKey(-360, MTime(20.0f));

    theLightRotYAnim.setMode(MAnimation1f::PLAY_MODE_REPEAT);
    theLightRotYAnim.setKey(0.0f, MTime(0.0f));
    theLightRotYAnim.setKey(360, MTime(8.0f));

    /* Set sphere resolution (horizontal and vertical density of triangles). */
    theGeometrySphere->set(0.5f, 20, 20);
    theGeometryTorus->set(0.32f, 0.18f, 20, 10);
    theGeometryCone->set(0.15f, 0.3f, 10, 2);

    theGeometryTorus->getPrimitive().transform(MVector3f(0.0f, 0.0f, 0.0f));
    theGeometryComplex->set(theGeometryTorus->getPrimitive());

    theGeometrySphere->getPrimitive().transform(MVector3f(1.3f, 0.0f, 0.0f));
    theGeometryComplex->append(theGeometrySphere->getPrimitive());
    theGeometrySphere->getPrimitive().transform(MVector3f(-2.6f, 0.0f, 0.0f));
    theGeometryComplex->append(theGeometrySphere->getPrimitive());
    theGeometrySphere->getPrimitive().transform(MVector3f(0.7f, 0.0f, -1.3f));
    theGeometryComplex->append(theGeometrySphere->getPrimitive());
    theGeometrySphere->getPrimitive().transform(MVector3f(0.6f, 0.0f, 1.3f));

    theGeometryRectangle->set(5.0f, 5.0f, 1, 1);

    /* Initialize matrices. */
    /* M_PI / 4.0f is 45 degrees Field Of View. */
    sMatSceneProj = Matrix::matrixPerspective(tan(M_PI / 4.0f), (g_iWindowW / (float)g_iWindowH), 1.0f, 100.0f);
    sMatLightProj = Matrix::matrixPerspective(tan(M_PI / 4.0f), (g_iShadowW / (float)g_iShadowH), 1.0f, 100.0f);
    /* Create scene modelview matrix, move geometry further away from camera. */
    {
        Matrix sMatSceneRot =  Matrix::createRotationX(45);
        sMatSceneV = Matrix::createTranslation(0.0f, 0.0f, -4.0f);
        sMatSceneV = sMatSceneV * sMatSceneRot;
    }
    sMatIdentity = Matrix::identityMatrix;

    /* Initialize OpenGL-ES. */
    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glCullFace(GL_BACK));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthMask(true));
    GL_CHECK(glDepthFunc(GL_LEQUAL));
    GL_CHECK(glEnable(GL_BLEND));
    /* Should do src * (src alpha) + dest * (1-src alpha). */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    theRT->initialize(g_iShadowW, g_iShadowH);

    /* Load shaders. */

    theProgramFlare->initialize(MPathsManager::getFullPathStatic(VERTEX_SHADER_FLARE_FILE),
                                MPathsManager::getFullPathStatic(FRAGMENT_SHADER_FLARE_FILE));
    /* Bind attribute parameters. */
    theProgramFlare->bindAttrib(MRendererProgram::A_LOC_VERTEX,      "a_v4Position");
    theProgramFlare->bindAttrib(MRendererProgram::A_LOC_TEXCOORD_0,  "a_v3TexCoord");
    /* Bind uniform parameters. */
    theProgramFlare->bindUniform(MRendererProgram::U_LOC_MAT_MVP,    "u_m4MVP");
    theProgramFlare->bindUniform(MRendererProgram::U_LOC_GENERIC_1,  "u_v4FillColor");
    theProgramFlare->bindUniform(MRendererProgram::U_LOC_SAMPLER_0,  "u_s2dShadowMap");
    /* Set default parameters. */
    theProgramFlare->setUniform(MRendererProgram::U_LOC_GENERIC_1, MVector4f(1.0f, 1.0f, 1.0f, 1.0f));
    theProgramFlare->setUniform(MRendererProgram::U_LOC_SAMPLER_0, 0);

    /* Load Shader for drawing objects into shadow map. */
    theProgramSolid->initialize(MPathsManager::getFullPathStatic(VERTEX_SHADER_SOLID_FILE),
                                MPathsManager::getFullPathStatic(FRAGMENT_SHADER_SOLID_FILE));
    /* Bind attribute parameters. */
    theProgramSolid->bindAttrib(MRendererProgram::A_LOC_VERTEX,     "a_v4Position");
    /* Bind uniform parameters. */
    theProgramSolid->bindUniform(MRendererProgram::U_LOC_MAT_MVP,   "u_m4MVP");
    theProgramSolid->bindUniform(MRendererProgram::U_LOC_GENERIC_1, "u_v4FillColor");
    /* Set default parameters. */
    theProgramSolid->setUniform(MRendererProgram::U_LOC_GENERIC_1, MVector4f(1.0f, 1.0f, 0.0f, 1.0f));

    /* Load Shader for drawing objects into shadow map. */
    theProgramShadow->initialize(MPathsManager::getFullPathStatic(VERTEX_SHADER_SHADOW_FILE),
                                MPathsManager::getFullPathStatic(FRAGMENT_SHADER_SHADOW_FILE));
    /* Bind attribute parameters. */
    theProgramShadow->bindAttrib(MRendererProgram::A_LOC_VERTEX, "a_v4Position");
    /* Bind uniform parameters. */
    theProgramShadow->bindUniform(MRendererProgram::U_LOC_MAT_MVP, "u_m4LightMVP");

    /* Load ShadowMap shader which draws object with shadows on them. */
    theProgramShadowMap->initialize(MPathsManager::getFullPathStatic(VERTEX_SHADER_FILE),
                                   MPathsManager::getFullPathStatic(FRAGMENT_SHADER_FILE));
    /* Bind attribute parameters. */
    theProgramShadowMap->bindAttrib(MRendererProgram::A_LOC_VERTEX,            "a_v4Position");
    theProgramShadowMap->bindAttrib(MRendererProgram::A_LOC_COLOR,             "a_v4FillColor");
    theProgramShadowMap->bindAttrib(MRendererProgram::A_LOC_NORMAL,            "a_v4Normal");
    /* Bind uniforms parameters. */
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_MAT_MVP,          "u_m4MVP");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_MAT_M,            "u_m4M");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_MAT_MV,           "u_m4MV");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_MAT_MV_INV_TRANS, "u_m4InvTransMV");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_MAT_0,            "u_m4LightV");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_MAT_1,            "u_m4LightP");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_SAMPLER_0,        "u_s2dShadowMap");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_GENERIC_1,        "u_v3Specular");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_GENERIC_2,        "u_v4LightPos");
    theProgramShadowMap->bindUniform(MRendererProgram::U_LOC_GENERIC_3,        "u_v3GlobalAmbient");
    /* Bind u_s2dShadowMap onto first texture unit. */
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_SAMPLER_0, 0);

    /* Load textures */
    MImageTGA image;
    cout << MPathsManager::getFullPathStatic(FLARE_IMAGE_FILE).getData() << endl;
    image.load(MPathsManager::getFullPathStatic(FLARE_IMAGE_FILE));
    theTextureFlare->create(image.getWidth(), image.getHeight(), image.getData());

    /* We pass this for each scene, below. */

    return true;
}

void terminateGraphics()
{
    deleteAndNULL(theRT);
    deleteAndNULL(theTextureFlare);
    deleteAndNULL(theGeometryRectangle);
    deleteAndNULL(theGeometryComplex);
    deleteAndNULL(theGeometryCone);
    deleteAndNULL(theGeometryTorus);
    deleteAndNULL(theGeometrySphere);
    //
    deleteAndNULL(theProgramFlare);
    deleteAndNULL(theProgramShadowMap);
    deleteAndNULL(theProgramShadow);
    deleteAndNULL(theProgramSolid);
}

void renderGeometry(
    MGeometryBase& aGeometry,
    MRendererProgram& aProgram,
    Matrix& aM,
    Matrix& aV,
    Matrix& aP)
{
    Matrix mv   = aV * aM;
    Matrix mvp  = aP * mv;
    Matrix itmv = Matrix::matrixInvert(&mv);
    Matrix::matrixTranspose(&itmv);

    aProgram.use();

    aProgram.setUniform(MRendererProgram::U_LOC_MAT_M, aM);
    aProgram.setUniform(MRendererProgram::U_LOC_MAT_MV, mv);
    aProgram.setUniform(MRendererProgram::U_LOC_MAT_MV_INV_TRANS, itmv);
    aProgram.setUniform(MRendererProgram::U_LOC_MAT_MVP, mvp);

    aGeometry.render(aProgram);
}

void renderFrame(void)
{
    theTime.update();
    theCamRotYAnim.update(theTime);
    theLightRotYAnim.update(theTime);

    /* Create modelview matrix for light, looking at middle of far cube. */
    Matrix sMatTrans = Matrix::createTranslation( 0.0, 0.0f, -4.0f);
    Matrix sMatRotX = Matrix::createRotationX(30.0f);
    Matrix sMatRotY = Matrix::createRotationY(theLightRotYAnim.getValue());

    sMatLightV = sMatTrans * sMatRotX;
    sMatLightV = sMatLightV * sMatRotY;
    sMatLightM = Matrix::matrixInvert(&sMatLightV);

    sMatRotY = Matrix::createRotationY(theCamRotYAnim.getValue());
    sMatCameraV = sMatSceneV * sMatRotY;

    /* Set up modelview and projection matrices for Light's scene. */
    Matrix matModel = Matrix::identityMatrix;
    {
        Matrix mT = Matrix::createTranslation(0.0f, 1.0f, 1.2f);
        Matrix mR = Matrix::createRotationX(theCamRotYAnim.getValue() * 2.0f);
        matModel = mT * mR;
    }

    /* Set up modelview and projection matrices for Light's scene. */
    Matrix matLightInCamera = sMatCameraV * sMatLightM;
    Vec4f lightPos = {0.0f, 0.0f, 0.0f, 1.0f};
    lightPos = Matrix::vertexTransform(&lightPos, &matLightInCamera);

    /* Calculate ambient for current frame */
    {
        MVector3f ambientCoef(lightPos.x, lightPos.y, lightPos.z);
        ambientCoef.normalize();
        float cosAlpha = ambientCoef.dot(MVector3f(0.0, 0.0, -1.0)) * 5.0f - 3.5f;

        if (cosAlpha < 0.0f)
            cosAlpha = 0.0f;
        if (cosAlpha > 1.0f)
            cosAlpha = 1.0f;

        theGlobalAmbient.set(cosAlpha, cosAlpha, cosAlpha, 1.0f);
    }

    theProgramShadowMap->use();
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_GENERIC_2, lightPos);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_MAT_0, sMatLightV);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_MAT_1, sMatLightProj);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_GENERIC_1, MVector4f(1.0f, 1.0f, 0.0f, 1.0f));
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_GENERIC_3, theGlobalAmbient);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_SAMPLER_0, 0);

    /* Prepare shadow texture - render scene into texture from light point of view. */

    if (!theRT->bindRT())
    {
        LOGI("Error: Framebuffer incomplete at %s:%i\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Set clear screen color and clear screen. */
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0));
    GL_CHECK(glClearDepthf(1.0f));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthMask(true));
    GL_CHECK(glDepthFunc(GL_LEQUAL));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_CHECK(glCullFace(GL_BACK));

    renderGeometry(*theGeometryComplex,
                   *theProgramShadow,
                   sMatIdentity,
                   sMatLightV,
                   sMatLightProj);

    renderGeometry(*theGeometryTorus,
                   *theProgramShadow,
                   matModel,
                   sMatLightV,
                   sMatLightProj);

    theRT->unbindRT();

    /* Draw Scene with shadows into framebuffer and use created shadow texture. */

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    theRT->bindTexture();

    /* Render main scene. */
    GL_CHECK(glViewport(0, 0, g_iWindowW, g_iWindowH));

    GL_CHECK(glClearColor(theGlobalAmbient[0],
                        theGlobalAmbient[1],
                        theGlobalAmbient[2],
                        theGlobalAmbient[3]));
    GL_CHECK(glClearDepthf(1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    GL_CHECK(glCullFace(GL_BACK));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthMask(true));
    GL_CHECK(glDepthFunc(GL_LEQUAL));

    renderGeometry(*theGeometryComplex,
                   *theProgramShadowMap,
                   sMatIdentity,
                   sMatCameraV,
                   sMatSceneProj);

    renderGeometry(*theGeometryTorus,
                   *theProgramShadowMap,
                   matModel,
                   sMatCameraV,
                   sMatSceneProj);

    /* Draw floor. */
#define DRAW_FLOOR
#ifdef DRAW_FLOOR
    Matrix floorMVP = sMatSceneProj * sMatCameraV;
    Matrix itmv = Matrix::matrixInvert(&sMatCameraV);
    Matrix::matrixTranspose(&itmv);

    theProgramShadowMap->use();
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_MAT_M, Matrix::identityMatrix);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_MAT_MV, sMatCameraV);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_MAT_MV_INV_TRANS, itmv);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_MAT_MVP, floorMVP);
    theProgramShadowMap->setUniform(MRendererProgram::U_LOC_GENERIC_1, MVector4f(0.0f, 0.0f, 0.0f, 1.0f));

    GLint iLocPosition = theProgramShadowMap->getLocAttrib(MRendererProgram::A_LOC_VERTEX);
    GLint iLocFillColor = theProgramShadowMap->getLocAttrib(MRendererProgram::A_LOC_COLOR);
    GLint iLocNormal = theProgramShadowMap->getLocAttrib(MRendererProgram::A_LOC_NORMAL);

    if (iLocPosition != -1)
    {
        GL_CHECK(glEnableVertexAttribArray(iLocPosition));
        GL_CHECK(glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aFloorVertex));
    }
    if (iLocFillColor != -1)
    {
        GL_CHECK(glEnableVertexAttribArray(iLocFillColor));
        GL_CHECK(glVertexAttribPointer(iLocFillColor, 3, GL_FLOAT, GL_FALSE, 0, aFloorColor));
    }
    if (iLocNormal != -1)
    {
        GL_CHECK(glEnableVertexAttribArray(iLocNormal));
        GL_CHECK(glVertexAttribPointer(iLocNormal, 3, GL_FLOAT, GL_FALSE, 0, aFloorNormal));
    }
    GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, aFloorIndex));
#endif

#define DRAW_LIGHT
#ifdef DRAW_LIGHT
    /* Transform light position into world space position and draw Light. */
    Matrix timvp = sMatCameraV * sMatLightM;
    /* The Cone geometry must be rotated by 90 degres to give appropriate viewing results. */
    Matrix mRX90 = Matrix::createRotationX(90.0f);
    timvp = timvp * mRX90;
    timvp = sMatSceneProj *timvp;

    theProgramSolid->use();
    theProgramSolid->setUniform(MRendererProgram::U_LOC_MAT_MVP, timvp);
    theGeometryCone->render(*theProgramSolid);


    /* Draw light flare. */
    GL_CHECK(glDisable(GL_CULL_FACE));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, theTextureFlare->getName()));

    timvp = Matrix::createTranslation(lightPos.x, lightPos.y, lightPos.z);
    /* Rotate flare according to angle between viewing point and light. */
    Matrix mRZF = Matrix::createRotationZ(theGlobalAmbient[0] * 90.0f);
    timvp = timvp * mRZF;
    timvp = sMatSceneProj * timvp;

    theProgramFlare->use();
    /*
     * Set Flare transparent according to light position in theGlobalAmbient is stored cosinus angle between viewing point
     * of view and light.
     */
    theProgramFlare->setUniform(MRendererProgram::U_LOC_GENERIC_1, MVector4f(1.0f, 1.0f, 1.0f, theGlobalAmbient[0]));
    theProgramFlare->setUniform(MRendererProgram::U_LOC_MAT_MVP, timvp);
    /* Render Rectangle with texture mapped on it. */
    theGeometryRectangle->render(*theProgramFlare);

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    theRT->bindTexture();
    GL_CHECK(glEnable(GL_CULL_FACE));
#endif
}

int main(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform *platform = Platform::getInstance();

    /* Initialize windowing system. */
    platform->createWindow(WINDOW_W, WINDOW_H);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES2);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

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

    /* Release resources of the sample */
    terminateGraphics();

    /* Shut down OpenGL ES. */
    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object*/
    delete platform;

    return 0;
}
