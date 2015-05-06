/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MDRRendererProgram.h"

#include "MMatrix.h"

#include "Shader.h"

using namespace MaliSDK;

MDRRendererProgram::MDRRendererProgram()
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
  theLocUniform.setCount(UNIFORM_LOC_COUNT);
  theLocUniform.fill(-1);
  }

MDRRendererProgram::~MDRRendererProgram()
  {
  }

bool MDRRendererProgram::initialize(
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

  /* Get attribute locations of non-fixed attributes like colour and texture coordinates. */
  theLocAttrib[ATTRIB_LOC_VERTEX] = GL_CHECK(glGetAttribLocation(theProgram, "av4position"));
  theLocAttrib[ATTRIB_LOC_COLOR] = GL_CHECK(glGetAttribLocation(theProgram, "av3colour"));
  theLocAttrib[ATTRIB_LOC_TEXCOORD0] = GL_CHECK(glGetAttribLocation(theProgram, "a_v2TexCoord"));

  LOGD("iLocPosition = %i\n", theLocAttrib[ATTRIB_LOC_VERTEX]);
  LOGD("iLocColour   = %i\n", theLocAttrib[ATTRIB_LOC_COLOR]);
  LOGD("a_v2TexCoord   = %i\n", theLocAttrib[ATTRIB_LOC_TEXCOORD0]);

  /* Get uniform locations */
  theLocUniform[UNIFORM_LOC_MATRIX_MVP] = GL_CHECK(glGetUniformLocation(theProgram, "mvp"));
  theLocUniform[UNIFORM_LOC_MATRIX_T0] = GL_CHECK(glGetUniformLocation(theProgram, "u_m4Texture"));
  theLocUniform[UNIFORM_LOC_SAMPLER0] = GL_CHECK(glGetUniformLocation(theProgram, "u_s2dTexture"));

  LOGD("iLocMVP      = %i\n", theLocUniform[UNIFORM_LOC_MATRIX_MVP]);
  LOGD("u_m4Texture  = %i\n", theLocUniform[UNIFORM_LOC_MATRIX_T0]);
  LOGD("u_s2dTexture  = %i\n", theLocUniform[UNIFORM_LOC_SAMPLER0]);

  // Set sampler variable with intex 0 onto first texture unit
  setUniform(UNIFORM_LOC_SAMPLER0, 0);

  return true;
  }

void MDRRendererProgram::use() const
  {
  GL_CHECK(glUseProgram(theProgram));
  }

void MDRRendererProgram::setUniform(
    MDRUniformLocation aUniformLocationType,
    const MMatrix4f& aMatrix) const
  {
  GL_CHECK(glUniformMatrix4fv(getLocUniform(aUniformLocationType), 1, GL_FALSE, aMatrix.getData()));
  }

void MDRRendererProgram::setUniform(
    MDRUniformLocation aUniformLocationType,
    int aValue) const
  {
  GL_CHECK(glUniform1i(getLocUniform(aUniformLocationType), aValue));
  }
