/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef ASTC_TEXTURES_H
#define ASTC_TEXTURES_H

#include <string>

#ifndef M_PI
/**
 * \brief The value of pi.
 */
#define M_PI 3.14159265358979323846f
#endif /* M_PI */

#ifndef M_PI_2
/**
 * \brief The value of pi/2.
 */
#define M_PI_2 1.57079632679489661923f
#endif /* M_PI_2 */

/* ASTC texture compression internal formats. */
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR            (0x93B0)
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR            (0x93B1)
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR            (0x93B2)
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR            (0x93B3)
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR            (0x93B4)
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR            (0x93B5)
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR            (0x93B6)
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR            (0x93B7)
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR           (0x93B8)
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR           (0x93B9)
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR           (0x93BA)
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR          (0x93BB)
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR          (0x93BC)
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR          (0x93BD)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR    (0x93D0)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR    (0x93D1)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR    (0x93D2)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR    (0x93D3)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR    (0x93D4)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR    (0x93D5)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR    (0x93D6)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR    (0x93D7)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR   (0x93D8)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR   (0x93D9)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR   (0x93DA)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR  (0x93DB)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR  (0x93DC)
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR  (0x93DD)

/* Number of texture sets. */
#define NUM_OF_TEXTURE_IDS                         (28)

/* Time period for each texture set to be displayed. */
#define ASTC_TEXTURE_SWITCH_INTERVAL               (5) /* sec */

/* Initial value of x-angle. */
#define START_ANGLE_X_ROTATION                     (12)

/* Angular rates around several axes. */
#define X_ROTATION_SPEED                           (360/(20*3))
#define Y_ROTATION_SPEED                           (360/(30*3))
#define Z_ROTATION_SPEED                           (360/(60*3))

/* ASTC header declaration. */
typedef struct
{
    unsigned char magic[4];
    unsigned char blockdim_x;
    unsigned char blockdim_y;
    unsigned char blockdim_z;
    unsigned char xsize[3];   /* x-size = xsize[0] + xsize[1] + xsize[2] */
    unsigned char ysize[3];   /* x-size, y-size and z-size are given in texels */
    unsigned char zsize[3];   /* block count is inferred */
} astc_header;

/* Contains information about texture set bindings. */
typedef struct texture_set
{
    /* Bindings for each texture unit. */
    unsigned int cloud_and_gloss_texture_id;
    unsigned int earth_color_texture_id;
    unsigned int earth_night_texture_id;

    /* Name of compression algorithm. */
    std::string name;
} texture_set;


/* Contains information about texture set files. */
typedef struct texture_set_info
{
    /* Type of compression: ASTC copressed image internal format. */
    const int   compression_type_name;

    /* Paths to texture images for one texture set. */
    std::string cloud_and_gloss_texture_file_path;
    std::string earth_color_texture_file_path;
    std::string earth_night_texture_file_path;

    /* Name of compression algorithm. */
    std::string compressed_texture_format_name;
} texture_set_info;

#endif /* ASTC_TEXTURES_H */
