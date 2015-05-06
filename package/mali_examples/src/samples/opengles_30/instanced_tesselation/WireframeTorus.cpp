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
#include "WireframeTorus.h"

#include "Platform.h"
#include "Shader.h"
#include "Matrix.h"
#include "TorusModel.h"

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <string>

using namespace MaliSDK;
using std::string;

WireframeTorus::WireframeTorus(float torusRadius, float circleRadius)
{
    /* Initialize class fields. */
    this->torusRadius     = torusRadius;
    this->circleRadius    = circleRadius;
    this->indicesBufferID = 0;

    /* Name of the file in which fragment shader's body is located. */
    const string fragmentShaderPath = resourceDirectory + "Instanced_Tessellation_Wireframe_shader.frag";
    /* Name of the file in which vertex shader's body is located. */
    const string vertexShaderPath   = resourceDirectory + "Instanced_Tessellation_Wireframe_shader.vert";

    /* Initialize shaders and program corresponding to the constructed torus object. */
    setupGraphics(vertexShaderPath, fragmentShaderPath);

    /* Determine indices of the mesh. */
    initializeBufferForIndices();

    /* Generate buffers and vertex arrays to store torus vertices and colors associated with them. */
    initializeVertexAttribs();

    /* Set wireframe color to orange. */
    setColor(1.0f, 0.3f, 0.0f, 1.0f);
}

WireframeTorus::~WireframeTorus()
{
    GL_CHECK(glDeleteBuffers(1, &indicesBufferID));
}

void WireframeTorus::initializeBufferForIndices()
{
    /* Temporary array to store determined indices. */
    unsigned int indices[indicesCount];

    TorusModel::calculateWireframeIndices(circlesCount, pointsPerCircleCount, indices);

    /* Create GL_ELEMENT_ARRAY_BUFFER and store indices in it. */
    GL_CHECK(glGenBuffers(1, &indicesBufferID));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBufferID));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(int), indices, GL_STATIC_DRAW));
}

void WireframeTorus::draw(float* rotationVector)
{
    GLint rotationVectorLocation = GL_CHECK(glGetUniformLocation(programID, "rotationVector"));

    /* Set required elements to draw mesh torus. */
    GL_CHECK(glUseProgram(programID));
    GL_CHECK(glBindVertexArray(vaoID));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBufferID));

    /* Pass Model-View matrix elements to the shader. */
    GL_CHECK(glUniform3fv(rotationVectorLocation, 1, rotationVector));

    /* Draw lines described by previously determined indices. */
    GL_CHECK(glDrawElements(GL_LINES, indicesCount, GL_UNSIGNED_INT, 0));
}

bool WireframeTorus::initializeVertexAttribs()
{
    /* Get input attribute locations. */
    GLint  positionLocation = GL_CHECK(glGetAttribLocation(programID, "position"));

    /* ID of a buffer that stores vertices. */
    GLuint vertexBufferID = 0;

    /* Temporary arrays to keep vertex and color data. */
    float torusVertices[componentsCount];

    TorusModel::generateVertices(torusRadius, circleRadius, circlesCount, pointsPerCircleCount, torusVertices);

    /* Generate and bind vertex array object. */
    GL_CHECK(glGenVertexArrays(1, &vaoID));
    GL_CHECK(glBindVertexArray(vaoID));

    if (positionLocation != -1)
    {
        /* Generate and bind buffer object to store vertex data. */
        GL_CHECK(glGenBuffers(1, &vertexBufferID));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID));

        /* Store torus vertices inside the generated buffer. */
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, componentsCount * sizeof(float), torusVertices, GL_STATIC_DRAW));

        /* Set vertex attrib pointer to the beginning of the bound array buffer. */
        GL_CHECK(glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, NULL));
        GL_CHECK(glEnableVertexAttribArray(positionLocation));
    }
    else
    {
        LOGE("Could not locate \"position\" input attribute in program [%d].", programID);
        return false;
    }

    return true;
}