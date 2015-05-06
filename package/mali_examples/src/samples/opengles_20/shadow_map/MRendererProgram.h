/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAPDR_RENDERER_PROGRAM_HPP
#define M_SHADOWMAPDR_RENDERER_PROGRAM_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MArray.h"
#include "MMatrix.h"
#include "MVector4.h"
#include "MString.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents vertex and fragment programs combined together.
 */
class MRendererProgram
  {
public:

  // ----- Types -----

  ///
  enum MDRAttribLocation
    {
    ///
    A_LOC_VERTEX,
    ///
    A_LOC_NORMAL,
    ///
    A_LOC_COLOR,
    ///
    A_LOC_TEXCOORD_0,
    ///
    A_LOC_TEXCOORD_1,
    ///
    A_LOC_TEXCOORD_2,
    ///
    A_LOC_TEXCOORD_3,
    ///
    A_LOC_TEXCOORD_4,
    ///
    A_LOC_TEXCOORD_5,
    ///
    A_LOC_TEXCOORD_6,
    ///
    A_LOC_TEXCOORD_7,
    ///
    ATTRIBS_COUNT
    };

  ///
  enum MUniformLocation
    {
    ///
    U_LOC_MAT_MVP,
    ///
    U_LOC_MAT_MV,
    ///
    U_LOC_MAT_M,
    ///
    U_LOC_MAT_V,
    ///
    U_LOC_MAT_P,
    ///
    U_LOC_MAT_MV_INV,
    ///
    U_LOC_MAT_MV_INV_TRANS,
    ///
    U_LOC_MAT_0,
    ///
    U_LOC_MAT_1,
    ///
    U_LOC_MAT_2,
    ///
    U_LOC_MAT_3,
    ///
    U_LOC_MAT_4,
    ///
    U_LOC_MAT_5,
    ///
    U_LOC_MAT_6,
    ///
    U_LOC_MAT_7,
    ///
    U_LOC_SAMPLER_0,
    ///
    U_LOC_SAMPLER_1,
    ///
    U_LOC_SAMPLER_2,
    ///
    U_LOC_SAMPLER_3,
    ///
    U_LOC_SAMPLER_4,
    ///
    U_LOC_GENERIC_1,
    ///
    U_LOC_GENERIC_2,
    ///
    U_LOC_GENERIC_3,
    ///
    U_LOC_GENERIC_4,
    ///
    U_LOC_GENERIC_5,
    ///
    U_LOC_GENERIC_6,
    ///
    U_LOC_GENERIC_7,
    ///
    U_LOC_GENERIC_8,
    ///
    U_LOC_GENERIC_9,
    ///
    U_LOC_GENERIC_10,
    ///
    U_LOC_GENERIC_11,
    ///
    U_LOC_GENERIC_12,
    ///
    U_LOC_GENERIC_13,
    ///
    U_LOC_GENERIC_14,
    ///
    U_LOC_GENERIC_15,
    ///
    UNIFORMS_COUNT
    };

  ///
  typedef GLint MDRLocValueType;

  // ----- Constructors and destructors -----

  ///
  MRendererProgram();

  ///
  ~MRendererProgram();

  // ----- Accessors and mutators -----

  /// The method retrieves an index of a requested attribute.
  MDRLocValueType getLocAttrib(MDRAttribLocation aAttribLocationType) const
    { return theLocAttrib[aAttribLocationType]; }

  /// The method retrieves an index of a requested uniform parameters.
  MDRLocValueType getLocUniform(MUniformLocation aUniformLocationType) const
    { return theLocUniform[aUniformLocationType]; }

  // ----- Miscellaneous -----

  /// The method creates a vertex and a fragment programs from specified files.
  bool initialize(const MPath& aPathVP,
                  const MPath& aPathFP);

  /// The method maps an attribute name onto a predefined one.
  void bindAttrib(MDRAttribLocation aAttribLocationType,
                  const char* aAttribName);

  /// The method maps a uniform parameter name onto a predefined one.
  void bindUniform(MUniformLocation aUniformLocationType,
                   const char* aUniformName);

  /// The methods binds the shaders
  void use() const;

  /// The method sets a uniform parameter as a matrix.
  void setUniform(MUniformLocation aUniformLocationType,
                  const MaliSDK::Matrix& aMatrix) const;

  /// The method sets a uniform parameter as a matrix.
  void setUniform(MUniformLocation aUniformLocationType,
                  const MMatrix4f& aMatrix) const;

  /// The method sets a uniform parameter as a vector.
  void setUniform(MUniformLocation aUniformLocationType,
                  const MVector4f& aVector) const;

  /// The method sets a uniform parameter as a vector.
  void setUniform(MUniformLocation aUniformLocationType,
                  const MaliSDK::Vec4f& aVector) const;

  /// The method sets a uniform parameter as an integer.
  void setUniform(MUniformLocation aUniformLocationType,
                  int aValue) const;

private:

  // ----- Types -----

  //
  typedef MArray<MDRLocValueType> MDRLocArray;

  // ----- Fields -----

  //
  GLuint theProgram;
  GLuint theProgramVertex;
  GLuint theProgramFragment;
  //
  MDRLocArray theLocAttrib;
  MDRLocArray theLocUniform;
  };

#endif
