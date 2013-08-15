#ifndef CURL_COMMON_H_
#define CURL_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

#define CURLMAX(a,b) ((a) > (b) ? (a) : (b))
#define CURLMIN(a,b) ((a) > (b) ? (b) : (a))
#define CURLSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)

void *c_malloc(unsigned int size);
void *c_realloc(void *ptr, unsigned int size);
void c_freep(void **arg);
void c_free(void *arg);
void *c_mallocz(unsigned int size);

int c_strstart(const char *str, const char *pfx, const char **ptr);
int c_stristart(const char *str, const char *pfx, const char **ptr);
char *c_stristr(const char *haystack, const char *needle);
size_t c_strlcpy(char *dst, const char *src, size_t size);
size_t c_strlcat(char *dst, const char *src, size_t size);
size_t c_strlcatf(char *dst, size_t size, const char *fmt, ...);

#endif
