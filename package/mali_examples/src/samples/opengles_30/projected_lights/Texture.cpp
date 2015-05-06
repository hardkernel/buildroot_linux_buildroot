/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "Texture.h"

namespace MaliSDK
{
    /* Please see header for the specification. */
    void Texture::loadBmpImageData(const char*     fileName,
                                   int*            imageWidthPtr,
                                   int*            imageHeightPtr,
                                   unsigned char** textureDataPtrPtr)
    {
        ASSERT(fileName          != NULL,
               "Invalid file name.");
        ASSERT(textureDataPtrPtr != NULL,
               "Cannot use NULL pointer to store image data.");

        tagBITMAPFILEHEADER bitmapFileHeader;
        tagBITMAPINFOHEADER bitmapInfoHeader;
        FILE*               file              = NULL;
        unsigned char*      loadedTexture     = NULL;

        /* Try to open file. */
        file = fopen(fileName, "rb");

        ASSERT(file != NULL, "Failed to open file");

        /* Try to read the bitmap file header. */
        readBitmapFileHeader(file, &bitmapFileHeader);

        /* Try to read the bitmap info header. */
        readBitmapInforHeader(file, &bitmapInfoHeader);

        /* Try to allocate memory to store texture image data. */
        loadedTexture = (unsigned char*) malloc(bitmapInfoHeader.biSizeImage);

        ASSERT(loadedTexture != NULL, "Could not allocate memory to store texture image data.");

        /* Move the file pointer to the begging of the bitmap image data. */
        fseek(file, bitmapFileHeader.bfOffBits, 0);

        /* Read in the image data. */
        fread(loadedTexture, bitmapInfoHeader.biSizeImage, 1, file);

        unsigned char tempElement;

        /* As data in bmp file is stored in BGR, we need to convert it into RGB. */
        for (unsigned int imageIdx  = 0;
                          imageIdx  < bitmapInfoHeader.biSizeImage;
                          imageIdx += 3)
        {
            tempElement                 = loadedTexture[imageIdx];
            loadedTexture[imageIdx]     = loadedTexture[imageIdx + 2];
            loadedTexture[imageIdx + 2] = tempElement;
        }

        /* At the end, close the file. */
        fclose(file);

        /* Return retrieved data. */
        *textureDataPtrPtr = loadedTexture;

        /* Store the image dimensions if requested. */
        if (imageHeightPtr != NULL)
        {
            *imageHeightPtr = bitmapInfoHeader.biHeight;
        }

        if (imageWidthPtr != NULL)
        {
            *imageWidthPtr = bitmapInfoHeader.biWidth;
        }
    }

    /* Please see header for the specification. */
    void Texture::readBitmapFileHeader(FILE* filePtr, tagBITMAPFILEHEADER* bitmapFileHeaderPtr)
    {
        ASSERT(filePtr             != NULL &&
               bitmapFileHeaderPtr != NULL,
               "Invalid arguments used to read bitmap file header.");

        fread(&bitmapFileHeaderPtr->bfType,      sizeof(bitmapFileHeaderPtr->bfType),      1, filePtr);
        fread(&bitmapFileHeaderPtr->bfSize,      sizeof(bitmapFileHeaderPtr->bfSize),      1, filePtr);
        fread(&bitmapFileHeaderPtr->bfReserved1, sizeof(bitmapFileHeaderPtr->bfReserved1), 1, filePtr);
        fread(&bitmapFileHeaderPtr->bfReserved2, sizeof(bitmapFileHeaderPtr->bfReserved2), 1, filePtr);
        fread(&bitmapFileHeaderPtr->bfOffBits,   sizeof(bitmapFileHeaderPtr->bfOffBits),   1, filePtr);

        /* Make sure that file type is valid. */
        ASSERT(bitmapFileHeaderPtr->bfType == 0x4D42,
               "Invalid file type read");
    }

    /* Please see header for the specification. */
    void Texture::readBitmapInforHeader(FILE* filePtr, tagBITMAPINFOHEADER* bitmapInfoHeaderPtr)
    {
        ASSERT(filePtr != NULL &&
               bitmapInfoHeaderPtr != NULL,
               "Invalid arguments used to read bitmap info header.");

        fread(&bitmapInfoHeaderPtr->biSize,          sizeof(bitmapInfoHeaderPtr->biSize),          1, filePtr);
        fread(&bitmapInfoHeaderPtr->biWidth,         sizeof(bitmapInfoHeaderPtr->biWidth),         1, filePtr);
        fread(&bitmapInfoHeaderPtr->biHeight,        sizeof(bitmapInfoHeaderPtr->biHeight),        1, filePtr);
        fread(&bitmapInfoHeaderPtr->biPlanes,        sizeof(bitmapInfoHeaderPtr->biPlanes),        1, filePtr);
        fread(&bitmapInfoHeaderPtr->biBitCount,      sizeof(bitmapInfoHeaderPtr->biBitCount),      1, filePtr);
        fread(&bitmapInfoHeaderPtr->biCompression,   sizeof(bitmapInfoHeaderPtr->biCompression),   1, filePtr);
        fread(&bitmapInfoHeaderPtr->biSizeImage,     sizeof(bitmapInfoHeaderPtr->biSizeImage),     1, filePtr);
        fread(&bitmapInfoHeaderPtr->biXPelsPerMeter, sizeof(bitmapInfoHeaderPtr->biXPelsPerMeter), 1, filePtr);
        fread(&bitmapInfoHeaderPtr->biYPelsPerMeter, sizeof(bitmapInfoHeaderPtr->biYPelsPerMeter), 1, filePtr);
        fread(&bitmapInfoHeaderPtr->biClrUsed,       sizeof(bitmapInfoHeaderPtr->biClrUsed),       1, filePtr);
        fread(&bitmapInfoHeaderPtr->biClrImportant,  sizeof(bitmapInfoHeaderPtr->biClrImportant),  1, filePtr);
    }
}
