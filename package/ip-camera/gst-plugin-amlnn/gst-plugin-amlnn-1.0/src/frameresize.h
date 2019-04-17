#ifndef _FRAMERESIZE_H
#define _FRAMERESIZE_H

void frameresize_init ();
void frameresize_deinit();
const char *frameresize_begin (const char* input,
    int iw, int ih, int ow, int oh);
void frameresize_end (const char *buf);
#endif /* _FRAMERESIZE_H */
