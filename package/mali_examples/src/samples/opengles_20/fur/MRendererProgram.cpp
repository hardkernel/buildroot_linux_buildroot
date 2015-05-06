/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MRendererProgram.h"

using namespace MaliSDK;

MRendererProgram::MRendererProgram()
    :
    theProgram(0),
    theProgramVertex(0),
    theProgramFragment(0),
    //
    theLocAttrib(),
    theLocUniform()
  {
  theLocAttrib.setCount(ATTRIBS_COUNT);
  theLocAttrib.fill(-1);
  theLocUniform.setCount(UNIFORMS_COUNT);
  theLocUniform.fill(-1);
  }

MRendererProgram::~MRendererProgram()
  {
  }

bool MRendererProgram::initialize(
    const MPath& aPathVP,
    const MPath& aPathFP)
  {
  /* Shader initialization */
  Shader::processShader(&theProgramVertex, (char*)aPathVP.getData(), GL_VERTEX_SHADER);
  Shader::processShader(&theProgramFragment, (char*)aPathFP.getData(), GL_FRAGMENT_SHADER);

  /* Create theProgram (ready to attach shaders) */
  theProgram = GL_CHECK(glCreateProgram());

  /* Attach shaders and link theProgram */
  GL_CHECK(glAttachShader(theProgram, theProgramVertex));
  GL_CHECK(glAttachShader(theProgram, theProgramFragment));
  GL_CHECK(glLinkProgram(theProgram));
  GL_CHECK(glUseProgram(theProgram));

  return true;
  }

void MRendererProgram::bindAttrib(
    MDRAttribLocation aAttribLocationType,
    const char* aAttribName)
  {
  theLocAttrib[aAttribLocationType] = GL_CHECK(glGetAttribLocation(theProgram, aAttribName));
  LOGD("%s = %i\n", aAttribName, theLocAttrib[aAttribLocationType]);
  }

void MRendererProgram::bindUniform(
    MUniformLocation aUniformLocationType,
    const char* aUniformName)
  {
  theLocUniform[aUniformLocationType] = GL_CHECK(glGetUniformLocation(theProgram, aUniformName));
  LOGD("%s = %i\n", aUniformName, theLocUniform[aUniformLocationType]);
  }

void MRendererProgram::use() const
  {
  GL_CHECK(glUseProgram(theProgram));
  }

void MRendererProgram::setUniform(
    MUniformLocation aUniformLocationType,
    const Matrix& aMatrix) const
  {
  GLint loc = getLocUniform(aUniformLocationType);
  if (loc == -1)
    return;
  Matrix copy = aMatrix;
  GL_CHECK(glUniformMatrix4fv(getLocUniform(aUniformLocationType), 1, GL_FALSE, copy.getAsArray()));
  }

void MRendererProgram::setUniform(
    MUniformLocation aUniformLocationType,
    const MMatrix4f& aMatrix) const
  {
  GLint loc = getLocUniform(aUniformLocationType);
  if (loc == -1)
    return;
  GL_CHECK(glUniformMatrix4fv(getLocUniform(aUniformLocationType), 1, GL_FALSE, aMatrix.getData()));
  }

void MRendererProgram::setUniform(
    MUniformLocation aUniformLocationType,
    const MVector4f& aVector) const
  {
  GLint loc = getLocUniform(aUniformLocationType);
  if (loc == -1)
    return;
  GL_CHECK(glUniform4fv(getLocUniform(aUniformLocationType), 1, (const GLfloat*)aVector.getData()));
  }

void MRendererProgram::setUniform(
    MUniformLocation aUniformLocationType,
    const Vec4f& aVector) const
  {
  GLint loc = getLocUniform(aUniformLocationType);
  if (loc == -1)
    return;
  GL_CHECK(glUniform4fv(getLocUniform(aUniformLocationType), 1, (const GLfloat*)&aVector));
  }

void MRendererProgram::setUniform(
    MUniformLocation aUniformLocationType,
    int aValue) const
  {
  GLint loc = getLocUniform(aUniformLocationType);
  if (loc == -1)
    return;
  GL_CHECK(glUniform1i(getLocUniform(aUniformLocationType), aValue));
  }
