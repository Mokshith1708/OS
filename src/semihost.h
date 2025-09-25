#pragma once
#include <stdint.h>

int      sh_open(const char *path, uint32_t mode);   // returns handle or -1
int      sh_close(int fd);
int      sh_read (int fd, void *buf, uint32_t len);  // returns remaining bytes
int      sh_write(int fd, const void *buf, uint32_t len); // returns remaining
int      sh_flen (int fd);                           // returns length or -1
void     sh_write0(const char *s);
void     sh_exit(int code);
