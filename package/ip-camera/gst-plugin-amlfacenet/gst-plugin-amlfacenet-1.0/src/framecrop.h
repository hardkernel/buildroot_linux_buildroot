#ifndef _FRAMECROP_H
#define _FRAMECROP_H

void frmcrop_init ();
void frmcrop_deinit();
char *frmcrop_begin (const char* input, int iw, int ih,
    int x0, int y0, int x1, int y1, int ow, int oh);
void frmcrop_end ();
#endif /* _FRAMECROP_H */
