/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "MGeometryBase.h"

#include "MRendererProgram.h"

MGeometryBase::MGeometryBase()
    :
    thePrimitive()
  {
  }

MGeometryBase::~MGeometryBase()
  {
  }

void MGeometryBase::render(
    const MRendererProgram& aProgram)
  {
  thePrimitive.setAttribIndex(MRendererPrimitive::ATTRIB_VERTICES,   aProgram.getLocAttrib(MRendererProgram::A_LOC_VERTEX));
  thePrimitive.setAttribIndex(MRendererPrimitive::ATTRIB_COLORS,     aProgram.getLocAttrib(MRendererProgram::A_LOC_COLOR));
  thePrimitive.setAttribIndex(MRendererPrimitive::ATTRIB_NORMALS,    aProgram.getLocAttrib(MRendererProgram::A_LOC_NORMAL));
  thePrimitive.setAttribIndex(MRendererPrimitive::ATTRIB_TEXCOORDS_0,aProgram.getLocAttrib(MRendererProgram::A_LOC_TEXCOORD_0));
  //
  thePrimitive.render();
  }
