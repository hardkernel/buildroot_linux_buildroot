#ifndef _JPEG_H_
#define _JPEG_H_

#include <stdbool.h>

unsigned long save_jpeg (unsigned char *image_buffer,
    int image_width, int image_height, int quality,
    char **outbuf);

#endif /* _JPEG_H_ */
