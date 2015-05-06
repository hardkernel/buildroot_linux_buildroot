/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "InstancedTesselation.h"
#include "Torus.h"

#include "Platform.h"
#include "Matrix.h"
#include "Shader.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>

using namespace MaliSDK;
using std::string;

string Torus::resourceDirectory;

Torus::Torus()
{
    if (resourceDirectory.empty())
    {
        LOGD("Resource Directory has not been set\n");
    }
}

Torus::~Torus()
{
    GL_CHECK(glDeleteProgram(programID));
    GL_CHECK(glDeleteVertexArrays(1, &vaoID));
}

void Torus::setColor(float red, float green, float blue, float alpha)
{
    GLint colorLocation = GL_CHECK(glGetUniformLocation(programID, "color"));

    float color[] = {red, green, blue, alpha};

    GL_CHECK(glUseProgram(programID));

    if (colorLocation != -1)
    {
        GL_CHECK(glUniform4fv(colorLocation, 1, color));
    }
    else
    {
        LOGD("Could not locate \"color\" uniform in program [%d]", programID);
    }
}

void Torus::setProjectionMatrix(Matrix* projectionMatrix)
{
    GLint projectionMatrixLocation = GL_CHECK(glGetUniformLocation(programID, "projectionMatrix"));

    GL_CHECK(glUseProgram(programID));
    GL_CHECK(glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix->getAsArray()));
}

void Torus::setupGraphics(const string vertexShaderPath, const string fragmentShaderPath)
{
    GLuint fragmentShaderID = 0;
    GLuint vertexShaderID   = 0;

    Shader::processShader(&vertexShaderID,   vertexShaderPath.c_str(),   GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    programID = GL_CHECK(glCreateProgram());

    GL_CHECK(glAttachShader(programID, vertexShaderID));
    GL_CHECK(glAttachShader(programID, fragmentShaderID));

    GL_CHECK(glLinkProgram(programID));

    float scalingFactor     =  0.7f;
    float cameraTranslation = -2.5f;

    Matrix cameraMatrix = Matrix::createTranslation(0.0f, 0.0f, cameraTranslation);
    Matrix scaleMatrix  = Matrix::createScaling(scalingFactor, scalingFactor, scalingFactor);

    GLint scaleMatrixLocation  = GL_CHECK(glGetUniformLocation(programID, "scaleMatrix"));
    GLint cameraMatrixLocation = GL_CHECK(glGetUniformLocation(programID, "cameraMatrix"));

    GL_CHECK(glUseProgram(programID));

    GL_CHECK(glUniformMatrix4fv(scaleMatrixLocation,  1, GL_FALSE, scaleMatrix.getAsArray()));
    GL_CHECK(glUniformMatrix4fv(cameraMatrixLocation, 1, GL_FALSE, cameraMatrix.getAsArray()));
}

void Torus::setResourceDirectory(string requiredResourceDirectory)
{
    resourceDirectory = requiredResourceDirectory;
}