/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef SOLID_SPHERE_H
#define SOLID_SPHERE_H

class SolidSphere
{
    public:
        /** \brief Solid sphere constructor. It generates vertex position, normals 
         *         and texture coordinates data based on user-provided arguments.
         * 
         *  \param[in] radius  radius of sphere.
         *  \param[in] rings   number of parallels sphere consists of.
         *  \param[in] sectors number of meridians sphere consists of.
         */
        SolidSphere(const float radius, const unsigned int rings, const unsigned int sectors);

        float*          getSphereVertexData(int* vertex_data_size);
        float*          getSphereNormalData(int* normal_data_size);
        float*          getSphereTexcoords (int* texcoords_size);
        unsigned short* getSphereIndices   (int* n_indices);

    private:
        float*          sphere_vertices;
        float*          sphere_normals;
        float*          sphere_texcoords;
        unsigned short* sphere_indices;

        int sphere_vertex_data_size;
        int sphere_normal_data_size;
        int sphere_texcoords_size;
        int sphere_n_indices;
};

#endif /* SOLID_SPHERE_H */