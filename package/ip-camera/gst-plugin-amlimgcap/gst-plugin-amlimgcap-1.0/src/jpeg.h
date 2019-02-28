#ifndef _JPEG_H_
#define _JPEG_H_

#include <stdbool.h>

bool write_JPEG_file (unsigned char *image_buffer,
    int image_width, int image_height,
    char * filename, int quality);

#endif /* _JPEG_H_ */
