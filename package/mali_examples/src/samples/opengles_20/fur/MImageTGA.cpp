#include "MImageTGA.h"

#include <stdio.h>
#include <memory.h>

using namespace MaliSDK;

unsigned char* allocateData(
    unsigned int aDataSize,
    unsigned char aClearValue = 127)
  {
  unsigned char* data = new unsigned char[aDataSize];
  memset(data, aClearValue, aDataSize);
  return data;
  }

unsigned char* allocateData(
    unsigned int aWidth,
    unsigned int aHeight,
    unsigned int aBitsPerPixel,
    unsigned char aClearValue = 127)
  {
  unsigned int dataSize = aWidth * aHeight * (aBitsPerPixel / 8);
  return allocateData(dataSize, aClearValue);
  }

MImageTGA::MImageTGA()
    :
    theData(NULL),
    theWidth(0),
    theHeight(0),
    theBitsPerPixel(32),
    theBytesPerPixel(theBitsPerPixel / 8)
  {
  }

MImageTGA::~MImageTGA()
  {
  release();
  }

void MImageTGA::release()
  {
  if (theData == NULL)
    return;
  //
  delete [] theData;
  theData = NULL;
  theWidth = 0;
  theHeight = 0;
  }

unsigned int MImageTGA::reallocateData(
    const Header& aHeader)
  {
  // Free memory before allocation
  release();
  // Allocate memory
  theWidth = aHeader.theWidth;
  theHeight = aHeader.theHeight;
  unsigned int dataSize = theWidth * theHeight * (theBytesPerPixel);
  theData = allocateData(dataSize, 127);
  //
  return dataSize;
  }

void MImageTGA::copyData(
    unsigned int aSrcWidth,
    unsigned int aSrcHeight,
    unsigned int aSrcBitsPerPixel,
    const unsigned char* aSrcData,
    unsigned int aDstWidth,
    unsigned int aDstHeight,
    unsigned int aDstBitsPerPixel,
    unsigned char* aDstData) const
  {
  if (aSrcWidth != aDstWidth ||
      aSrcHeight != aDstHeight)
    {
    LOGE("[MImageTGA::copyData] Dimensions are incorrect to copy data!\n");
    return;
    }
  unsigned int dstIndex = 0;
  unsigned int dstBytesPerPixel = aDstBitsPerPixel / 8;
  unsigned int srcIndex = 0;
  unsigned int srcBytesPerPixel = aSrcBitsPerPixel / 8;
  for (unsigned int y = 0; y < aSrcHeight; y++)
    {
    for (unsigned int x = 0; x < aSrcWidth; x++)
      {
      for (unsigned int c = 0; c < dstBytesPerPixel; c++)
        {
        dstIndex = y * aDstWidth * dstBytesPerPixel + x * dstBytesPerPixel + c;
        if (c < srcBytesPerPixel)
          {
          srcIndex = y * aSrcWidth * srcBytesPerPixel + x * srcBytesPerPixel + c;
          aDstData[dstIndex] = aSrcData[srcIndex];
          }
        else
          aDstData[dstIndex] = 255;
        }
      }
    }
  }

bool MImageTGA::load(
    const MPath& aFileNameTGA)
  {
  FILE* fileDes = fopen(aFileNameTGA.getData(), "rb");
  if (fileDes == NULL)
    {
    LOGE("[MImageTGA::load] Cannot open \"%s\" file to load it!\n", (const char*)aFileNameTGA.getData());
    return false;
    }
  //
  Header header = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  size_t read = 0;
  // Write TGA header into the file
  read = fread(&header, sizeof(header), 1, fileDes);
  if (read != 1)
    {
    LOGE("[MImageTGA::load] Cannot read TGA header from the \"%s\" file!\n", (const char*)aFileNameTGA.getData());
    fclose(fileDes);
    return false;
    }
  if (header.theWidth == 0 ||
      header.theHeight == 0)
    {
    LOGE("[MImageTGA::load] The size of TGA image seems to be corrupted!\n");
    fclose(fileDes);
    return false;
    }
  if (header.theImageType != 2)
    {
    LOGE("[MImageTGA::load] The compressed format is not supported!\n");
    fclose(fileDes);
    return false;
    }
  if (header.theBits != 32 &&
      header.theBits != 24)
    {
    LOGE("[MImageTGA::load] The 32 and 24 bits per pixel formats are supported only!\n");
    fclose(fileDes);
    return false;
    }
  //
  unsigned int tmpDataSize = header.theWidth * header.theHeight * header.theBits / 8;
  unsigned char* tmpData = allocateData(tmpDataSize, 127);
  read = fread(tmpData, sizeof(unsigned char), tmpDataSize, fileDes);
  if (read != tmpDataSize)
    LOGI("WARNING: [MImageTGA::load] Image data was not loaded entirely!\n");
  //
  unsigned int dataSize = reallocateData(header);
  copyData(header.theWidth, header.theHeight, header.theBits, tmpData,
           theWidth, theHeight, theBitsPerPixel, theData);
  //
  delete [] tmpData;
  tmpData = NULL;
  //
  convertBGRA2RGBA(theData, dataSize);
  //
  if (header.theYStart)
    flipVertical();
  //
  if (fclose(fileDes))
    LOGI("[MImageTGA::load] Couldn't properly close the \"%s\" file!\n", (const char*)aFileNameTGA.getData());
  //
  return true;
  }

void MImageTGA::set(
    const unsigned char* aData,
    unsigned int aWidth,
    unsigned int aHeight)
  {
  Header header = {0, 0, 2, 0, 0, 0, 0, 0, aWidth, aHeight, 32, 0};
  unsigned int dataSize = reallocateData(header);
  copyData(header.theWidth, header.theHeight, header.theBits, aData,
           theWidth, theHeight, theBitsPerPixel, theData);
  flipVertical();
  }

bool MImageTGA::save(
    const MPath& aFileNameTGA)
  {
  FILE* fileDes = fopen(aFileNameTGA.getData(), "wb");
  if (fileDes == NULL)
    {
    LOGE("[MImageTGA::save] Cannot open \"%s\" file to write into it!\n", aFileNameTGA.getData());
    return false;
    }
  size_t written = 0;
  // Write TGA header into the file
  Header header = {0, 0, 2, 0, 0, 0, 0, 0, theWidth, theHeight, 32, 0};
  written = fwrite(&header, sizeof(header), 1, fileDes);
  //
  unsigned int dataSize = theBytesPerPixel * theWidth * theHeight;
  written = fwrite(theData, sizeof(unsigned char), dataSize, fileDes);
  if (written != dataSize)
    {
    LOGE("[MImageTGA::save] File:\"%s\" was not written entirely.!\n", aFileNameTGA.getData());
    }
  //
  if (fclose(fileDes))
    {
    LOGE("[MImageTGA::save] File:\"%s\" couldn't be closed!\n", aFileNameTGA.getData());
    }
  return true;
  }

void MImageTGA::convertBGRA2RGBA(
    unsigned char* aData,
    unsigned int aDataSize) const
  {
  unsigned char* ptrEnd = aData + aDataSize;
  unsigned char tmpVal = 0;
  for (unsigned char *p = aData, *p2 = (aData + 2); p < ptrEnd; p += 4, p2 += 4)
    {
    tmpVal = *p2;
    *p2 = *p;
    *p = tmpVal;
    // Alpha and Green components leave unchanged
    }
  }

void MImageTGA::flipVertical()
  {
  unsigned int halfHeight = (theHeight - (theHeight % 2)) / 2;
  unsigned int upIndex = 0;
  unsigned int bottomIndex = 0;
  unsigned char byteC = 0;
  //
  for (unsigned int y=0; y<halfHeight; y++)
    {
    for (unsigned int x=0; x<theWidth; x++)
      {
      for (unsigned int c=0; c<theBytesPerPixel; c++)
        {
        upIndex = y * theWidth * theBytesPerPixel + x * theBytesPerPixel + c;
        bottomIndex = (theHeight - y - 1) * theWidth * theBytesPerPixel + x * theBytesPerPixel + c;
        //
        byteC = theData[upIndex];
        theData[upIndex] = theData[bottomIndex];
        theData[bottomIndex] = byteC;
        }
      }
    }
  }

void MImageTGA::getAlphaComponent(
    MImageTGA& aOutImage) const
  {
  aOutImage.release();
  // Allocate data
  aOutImage.theWidth = theWidth;
  aOutImage.theHeight = theHeight;
  aOutImage.theBytesPerPixel = 1;
  aOutImage.theBitsPerPixel = 8;
  unsigned int size = aOutImage.theWidth * aOutImage.theHeight;
  aOutImage.theData = allocateData(size);
  // Now strip out the alpha component and copy it
  for (unsigned int index = 0; index < size; index++)
    aOutImage.theData[index] = theData[index << 2];
  }
