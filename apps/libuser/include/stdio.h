#pragma once

#include <stddef.h>

#define EOF (-1)

int putchar(int c);
int puts(const char *s);
int getchar(void);
int printf(const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
