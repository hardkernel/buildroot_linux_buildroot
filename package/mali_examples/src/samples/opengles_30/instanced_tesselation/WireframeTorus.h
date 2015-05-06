/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef WIREFRAME_TORUS_H
#define WIREFRAME_TORUS_H

#include "Torus.h"

/** 
 * \brief Class derived from the Torus abstract class. It manages drawing of a rotating wireframed
 *        unicolor torus. Apart from inherited components, it manages a buffer that stores 
 *        indices needed for the glDrawElements() call and also is of determining those indices.
 *        As input attributes, it directly passes the vertices of a torus.
 */
class WireframeTorus : public Torus
{
private:

    /* Number of indices needed for a single glDrawElements() call. */
    static const unsigned int indicesCount = 4 * circlesCount * pointsPerCircleCount;

    /* Index of a GL_ELEMENT_ARRAY_BUFFER buffer, to store determined indices. */
    GLuint indicesBufferID;

    /**
     * \brief Determine indices needed for a single glDrawElements() call in GL_LINES mode.
     */
    void initializeBufferForIndices(void);

    bool initializeVertexAttribs(void);

public:

    /**
     * \brief Instantiates a representation of a solid torus, using user-provided radius and tube radius.
     *
     * \param torusRadius  [in] Distance between the center of torus and the center of its tube.
     * \param circleRadius [in] Radius of the circle that models the tube.
     */
    WireframeTorus(float torusRadius, float circleRadius);

    ~WireframeTorus(void);

    void draw(float* rotationVector);
};

#endif /* WIREFRAME_TORUS_H */
