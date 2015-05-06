/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "Platform.h"

#include <cstdio>
#include <cstdlib>

namespace MaliSDK
{
    /** Structure holding bitmap file header elements. */
    struct tagBITMAPFILEHEADER
    {
        short bfType;
        int   bfSize;
        short bfReserved1;
        short bfReserved2;
        int   bfOffBits;
    };

    /** Structure holding bitmap info header elements. */
    struct tagBITMAPINFOHEADER
    {
        int   biSize;
        int   biWidth;
        int   biHeight;
        short biPlanes;
        short biBitCount;
        int   biCompression;
        int   biSizeImage;
        int   biXPelsPerMeter;
        int   biYPelsPerMeter;
        int   biClrUsed;
        int   biClrImportant;
    };

    /** 
     * \brief Functions for working with textures.
     */
    class Texture
    {
    private:
        /**
         * \brief Read BMP file header.
         *
         * \param filePtr             File pointer where BMP file header data is stored.
         *                            Cannot be NULL.
         * \param bitmapFileHeaderPtr Deref will be used to store loaded data.
         *                            Cannot be NULL.
         */
        static void readBitmapFileHeader(FILE* filePtr, tagBITMAPFILEHEADER* bitmapFileHeaderPtr);
        /**
         * \brief Read BMP info header.
         *
         * \param filePtr             File pointer where BMP info header data is stored.
         *                            Cannot be NULL.
         * \param bitmapInfoHeaderPtr Deref will be used to store loaded data.
         *                            Cannot be NULL.
         */
        static void readBitmapInforHeader(FILE* filePtr, tagBITMAPINFOHEADER* bitmapInfoHeaderPtr);

    public:
        /**
         * \brief Load BMP texture data from a file into memory.
         *
         * \param fileName          The file name of the texture to be loaded.
         *                          Cannot be NULL.
         * \param imageWidthPtr     Deref will be used to store image width.
         * \param imageHeightPtr    Deref will be used to store image height.
         * \param textureDataPtrPtr Pointer to a memory where loaded texture data will be stored.
         *                          Cannot be NULL.
         */
        static void loadBmpImageData(const char*     fileName,
                                     int*            imageWidthPtr,
                                     int*            imageHeightPtr,
                                     unsigned char** textureDataPtrPtr);
    };
}
#endif /* TEXTURE_H */