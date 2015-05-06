/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M_SHADOWMAP_IMAGE_TGA_HPP
#define M_SHADOWMAP_IMAGE_TGA_HPP

//------------------------------------------
// INCLUDES

#include "mCommon.h"

#include "MString.h"

//------------------------------------------
// BEGIN OF CLASS DECLARATION

/**
 * The class represents a simple TGA loader, which supports uncompressed TGA files with 24 and 32 bits format only.
 */
class MImageTGA
  {
public:

  // ----- Types -----

  ///
  #pragma pack(push, 1)
  typedef struct
    {
    unsigned char theIdentSize;          // size of ID field that follows 18 byte header (0 usually)
    unsigned char theColourMapType;      // type of colour map 0=none, 1=has palette
    unsigned char theImageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short theColourMapStart;             // first colour map entry in palette
    short theColourMapLength;            // number of colours in palette
    unsigned char theColourMapBits;      // number of bits per palette entry 15,16,24,32

    short theXStart;                     // image x origin
    short theYStart;                     // image y origin
    short theWidth;                      // image width in pixels
    short theHeight;                     // image height in pixels

    unsigned char theBits;               // image bits per pixel 8,16,24,32
    unsigned char theDescriptor;         // image descriptor bits (vh flip bits)
    // pixel data follows header
    } Header;
  #pragma pack(pop)

  // ----- Constructors and destructors -----

  ///
  MImageTGA();

  ///
  virtual ~MImageTGA();

  // ----- Accessors and mutators -----

  ///
  unsigned int getWidth() const
    { return theWidth; }

  ///
  unsigned int getHeight() const
    { return theHeight; }

  ///
  const unsigned char* getData() const
    { return theData; }

  // ----- Miscellaneous -----

  /// The method load an image from file
  bool load(const MPath& aFileNameTGA);

  /// The method saves the current image to a specified file.
  /// If the specified file already exists then the method overrides this file.
  bool save(const MPath& aFileNameTGA);


  /// The method creates an image from memory where aData is a RAW data image
  /// This is an alternative method to the "load" one.
  void set(const unsigned char* aData,
           unsigned int aWidth,
           unsigned int aHeight);

  /// The method flips the image vertically
  void flipVertical();

  /// The method releases the internal memory buffer
  void release();

  /// The method reads an Alpha component out from the image and writes into aOutImage
  void getAlphaComponent(MImageTGA& aOutImage) const;

protected:

private:

  // ----- Fields -----

  //
  unsigned char* theData;
  unsigned int theWidth;
  unsigned int theHeight;
  unsigned int theBitsPerPixel;
  unsigned int theBytesPerPixel;

  // ----- Miscellaneous -----

  //
  unsigned int reallocateData(const Header& aHeader);

  //
  void convertBGRA2RGBA(unsigned char* aData,
                        unsigned int aDataSize) const;

  //
  void copyData(unsigned int aSrcWidth,
                unsigned int aSrcHeight,
                unsigned int aSrcBitsPerPixel,
                const unsigned char* aSrcData,
                unsigned int aDstWidth,
                unsigned int aDstHeight,
                unsigned int aDstBitsPerPixel,
                unsigned char* aDstData) const;
  };


#endif  // __M_IMAGE_TGA_HPP
