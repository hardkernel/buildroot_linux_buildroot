/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef HEIGHTMAP_H__
#define HEIGHTMAP_H__

#include <GLES3/gl3.h>
#include "vector_math.h"
#include <vector>

class Heightmap
{
public:
    Heightmap(unsigned int size, unsigned int levels);
    ~Heightmap();

    void update_heightmap(const std::vector<vec2>& level_offsets);
    void reset();
    GLuint get_texture() const { return texture; }

private:
    GLuint texture;
    GLuint pixel_buffer[2];
    unsigned int pixel_buffer_index;
    unsigned int pixel_buffer_size;
    unsigned int size;
    unsigned int levels;

    struct LevelInfo
    {
        int x; // top-left coord of texture in texels.
        int y;
        bool cleared;
    };
    std::vector<LevelInfo> level_info;

    struct UploadInfo
    {
        int x;
        int y;
        int width;
        int height;
        int level;
        uintptr_t offset;
    };
    std::vector<UploadInfo> upload_info;

    void update_level(vec2 *buffer, unsigned int& pixel_offset, const vec2& level_offset, unsigned level);
    vec2 compute_heightmap(int x, int y, int level);
    float sample_heightmap(int x, int y);
    void update_region(vec2 *buffer, unsigned int& pixel_offset, int x, int y,
        int width, int height,
        int start_x, int start_y,
        int level);

    std::vector<float> heightmap;
    unsigned int heightmap_size;
    void init_heightmap();
};

#endif
