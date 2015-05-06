/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "Matrix.h"
#include "Platform.h"
#include "AstcTextures.h"

#include <cmath>
#include <cstring>
#include <cstdlib>

namespace MaliSDK
{
    /* Identity matrix. */
    const float Matrix::identityArray[16] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    Matrix::Matrix(const float* array)
    {
        memcpy(elements, array, 16 * sizeof(float));
    }

    Matrix Matrix::identityMatrix = Matrix(identityArray);

    Matrix::Matrix(void){}

    float* Matrix::getAsArray(void)
    {
        return elements;
    }

    float& Matrix::operator[](unsigned element)
    { 
        if (element > 15)
        {
            LOGE("Matrix only has 16 elements, tried to access element %d", element);
            exit(1);
        } 
        return elements[element]; 
    }

    Matrix Matrix::operator*(Matrix right)
    {
        return multiply(this, &right);
    }

    Matrix& Matrix::operator=(const Matrix &another)
    {
        if(this != &another)
        {
            memcpy(this->elements, another.elements, 16 * sizeof(float));
        }

        return *this;
    }

    Matrix Matrix::matrixOrthographic(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        Matrix result = identityMatrix;

        result.elements[0] = 2.0f / (right - left);
        result.elements[12] = -(right + left) / (right - left);

        result.elements[5] = 2.0f / (top - bottom);
        result.elements[13] = -(top + bottom) / (top - bottom);

        result.elements[10] = -2.0f / (zFar - zNear);
        result.elements[14] = -(zFar + zNear) / (zFar - zNear);

        return result;
    }

    Matrix Matrix::matrixPerspective(float FOV, float ratio, float zNear, float zFar)
    {
        Matrix result = identityMatrix;

        FOV = 1.0f / tan(FOV * 0.5f);

        result.elements[ 0] = FOV / ratio;
        result.elements[ 5] = FOV;
        result.elements[10] = -(zFar + zNear) / (zFar - zNear);
        result.elements[11] = -1.0f;
        result.elements[14] = (-2.0f * zFar * zNear) / (zFar - zNear);
        result.elements[15] = 0.0f;

        return result;
    }

    Matrix Matrix::createRotationX(float angle)
    {
        Matrix result = identityMatrix;
        float angle_converted_to_radians = M_PI * angle / 180.0f;

        result.elements[5]  = cos(angle_converted_to_radians);
        result.elements[9]  = -sin(angle_converted_to_radians);
        result.elements[6]  = sin(angle_converted_to_radians);
        result.elements[10] = cos(angle_converted_to_radians);

        return result;
    }

    Matrix Matrix::createRotationY(float angle)
    {
        Matrix result = identityMatrix;
        float angle_converted_to_radians = M_PI * angle / 180.0f;

        result.elements[0]  = cos(angle_converted_to_radians);
        result.elements[8]  = sin(angle_converted_to_radians);
        result.elements[2]  = -sin(angle_converted_to_radians);
        result.elements[10] = cos(angle_converted_to_radians);

        return result;
    }

    Matrix Matrix::createRotationZ(float angle)
    {
        Matrix result = identityMatrix;
        float angle_converted_to_radians = M_PI * angle / 180.0f;

        result.elements[0] = cos(angle_converted_to_radians);
        result.elements[4] = -sin(angle_converted_to_radians);
        result.elements[1] = sin(angle_converted_to_radians);
        result.elements[5] = cos(angle_converted_to_radians);

        return result;
    }

    Matrix Matrix::multiply(Matrix *left, Matrix *right)
    {
        Matrix result;

        for(int row = 0; row < 4; row++)
        {
            for(int column = 0; column < 4; column ++)
            {
                float accumulator = 0.0f;

                for(int allElements = 0; allElements < 4; allElements ++)
                {
                    accumulator += left->elements[allElements * 4 + row] * right->elements[column * 4 + allElements];
                }

                result.elements[column * 4 + row] = accumulator;
            }
        }

        return result;
    }
}