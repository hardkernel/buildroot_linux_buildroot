#ifndef _FRAMERESIZE_H
#define _FRAMERESIZE_H

void frameresize_init ();
void frameresize_deinit();
char* frameresize_begin (char* input,
    int iw, int ih, int ow, int oh);
void frameresize_end (char *buf);
#endif /* _FRAMERESIZE_H */
