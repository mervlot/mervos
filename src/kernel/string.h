#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t n);
int strcmp(const char *a, const char *b);
char *strcpy(char *dest, const char *src);

#endif 
