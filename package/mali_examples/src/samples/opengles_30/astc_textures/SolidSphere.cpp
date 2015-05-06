/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "SolidSphere.h"
#include "Platform.h"
#include "AstcTextures.h"

#include <cstdlib>
#include <cmath>

using namespace MaliSDK;

SolidSphere::SolidSphere(const float radius, const unsigned int rings, const unsigned int sectors)
{
    unsigned int r = 0;
    unsigned int s = 0;;

    /* Number of cooridnates for vertex, texture and normal coordinates. */
    const unsigned int n_vertex_coordinates  = 3;
    const unsigned int n_texture_coordinates = 2;
    const unsigned int n_normal_coordinates  = 3;
    const unsigned int n_indices_per_vertex  = 6;

    float const R = 1.0f / (float)(rings - 1);
    float const S = 1.0f / (float)(sectors - 1);

    sphere_vertex_data_size = rings * sectors * n_vertex_coordinates  * sizeof(float);
    sphere_normal_data_size = rings * sectors * n_normal_coordinates  * sizeof(float);
    sphere_texcoords_size   = rings * sectors * n_texture_coordinates * sizeof(float);
    sphere_n_indices        = rings * sectors * n_indices_per_vertex;

    sphere_vertices  = (float*)          malloc(sphere_vertex_data_size);
    sphere_normals   = (float*)          malloc(sphere_normal_data_size);
    sphere_texcoords = (float*)          malloc(sphere_texcoords_size);
    sphere_indices   = (unsigned short*) malloc(sphere_n_indices * sizeof(unsigned short));

    if (sphere_vertices == NULL || sphere_normals == NULL || sphere_texcoords == NULL || sphere_indices == NULL)
    {
        LOGE("Memory allocation error FILE: %s LINE: %i\n", __FILE__, __LINE__);
        exit(2);
    }

    float*          vertices  = sphere_vertices;
    float*          normals   = sphere_normals;
    float*          texcoords = sphere_texcoords;
    unsigned short* indices   = sphere_indices;

    for (r = 0; r < rings; r++)
    {
        for (s = 0; s < sectors; s++)
        {
            float const x = sin(M_PI * r * R) * cos(2.0f * M_PI * s * S);
            float const y = sin(-M_PI_2 + M_PI * r * R);
            float const z = sin(2.0f * M_PI * s * S) * sin(M_PI * r * R);

            *texcoords++ = s * S;
            *texcoords++ = r * R;

            *vertices++ = x * radius;
            *vertices++ = y * radius;
            *vertices++ = z * radius;

            *normals++ = x;
            *normals++ = y;
            *normals++ = z;
        }
    }

    for (r = 0; r < rings; r++)
    {
        for (s = 0; s < sectors; s++)
        {
            /* First triangle. */
            *indices++ = r       * sectors + s;
            *indices++ = r       * sectors + (s + 1);
            *indices++ = (r + 1) * sectors + s;

            /* Second tringle. */
            *indices++ = r       * sectors + (s + 1);
            *indices++ = (r + 1) * sectors + (s + 1);
            *indices++ = (r + 1) * sectors + s;
        }
    }
}

float* SolidSphere::getSphereVertexData(int* vertex_data_size)
{
    if (vertex_data_size == NULL)
    {
        LOGE("Memory error FILE: %s LINE: %i\n", __FILE__, __LINE__);
        exit(2);
    }

    *vertex_data_size = sphere_vertex_data_size;

    return sphere_vertices;
}

float* SolidSphere::getSphereNormalData(int* normal_data_size)
{
    if (normal_data_size == NULL)
    {
        LOGE("Memory error FILE: %s LINE: %i\n", __FILE__, __LINE__);
        exit(2);
    }

    *normal_data_size = sphere_normal_data_size;

    return sphere_normals;
}

float* SolidSphere::getSphereTexcoords(int* texcoords_size)
{
    if (texcoords_size == NULL)
    {
        LOGE("Memory error FILE: %s LINE: %i\n", __FILE__, __LINE__);
        exit(2);
    }

    *texcoords_size = sphere_texcoords_size;

    return sphere_texcoords;
}

unsigned short* SolidSphere::getSphereIndices(int* n_indices)
{
    if (n_indices == NULL)
    {
        LOGE("Memory error FILE: %s LINE: %i\n", __FILE__, __LINE__);
        exit(2);
    }

    *n_indices = sphere_n_indices;

    return sphere_indices;
}