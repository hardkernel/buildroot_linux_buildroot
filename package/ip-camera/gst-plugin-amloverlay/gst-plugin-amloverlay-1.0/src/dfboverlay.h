#ifndef _DFB_OVERLAY_H
#define _DFB_OVERLAY_H

void  overlay_init();
void  overlay_deinit();

void  overlay_create_inputbuffer(unsigned char* buf, int width, int height);
void  overlay_destroy_inputbuffer();

void* overlay_create_font (const char* fontfile, int fontsize, int outline_width);
void  overlay_destroy_font(void *font);

void  overlay_get_string_wh(const char* txt, void *font, int *w, int *h);
void* overlay_create_text_surface(const char* txt, void *font, int color, int bgcolor);
void* overlay_create_image_surface(const char* img, int width, int height);
void  overlay_draw_surface(void *surface, int x, int y);
void  overlay_destroy_surface(void *surface);

void  overlay_draw_rect(int x, int y, int w, int h, int thickness, int color);
#endif /* _DFB_OVERLAY_H */
