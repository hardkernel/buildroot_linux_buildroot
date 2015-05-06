/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_FURDR_RENDERER_PROGRAM_HPP
#define M_FURDR_RENDERER_PROGRAM_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MArray.h"
#include "MMatrix.h"
#include "MString.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class is a specialized wrapper over vertex and fragment programs and parameters
 * associated to them just for this example purpose.
 */
class MDRRendererProgram
  {
public:

  // ----- Types -----

  ///
  enum MDRAttribLocation
    {
    ///
    ATTRIB_LOC_VERTEX = 0,
    ///
    ATTRIB_LOC_COLOR = 1,
    ///
    ATTRIB_LOC_TEXCOORD0 = 2,
    ///
    ATTRIBS_COUNT = 3
    };

  ///
  enum MDRUniformLocation
    {
    ///
    UNIFORM_LOC_MATRIX_MVP = 0,
    ///
    UNIFORM_LOC_MATRIX_T0 = 1,
    ///
    UNIFORM_LOC_SAMPLER0 = 2,
    ///
    UNIFORM_LOC_COUNT = 3
    };

  ///
  typedef GLint MDRLocValueType;

  // ----- Constructors and destructors -----

  ///
  MDRRendererProgram();

  ///
  ~MDRRendererProgram();

  // ----- Accessors and mutators -----

  /// Returns native handle to an attribute, which is used for uploading vertices, coordinates, colors etc.
  MDRLocValueType getLocAttrib(MDRAttribLocation aAttribLocationType) const
    { return theLocAttrib[aAttribLocationType]; }

  /// Returns native handle to a uniform parameter such as: MVP matrix, a texture matrix and a texture sampler.
  MDRLocValueType getLocUniform(MDRUniformLocation aUniformLocationType) const
    { return theLocUniform[aUniformLocationType]; }

  // ----- Miscellaneous -----

  /// Loads vertex and fragment programs from a given file names and sets up attributes and uniforms handlers.
  bool initialize(const MPath& aPathVP,
                  const MPath& aPathFP);

  /// Binds vertex and fragment programs
  void use() const;

  /// Sets a matrix value for a specific uniform parameter
  void setUniform(MDRUniformLocation aUniformLocationType,
                  const MMatrix4f& aMatrix) const;

  /// Sets an integer value for a specific uniform parameter
  void setUniform(MDRUniformLocation aUniformLocationType,
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
