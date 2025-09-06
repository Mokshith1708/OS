#pragma once
#include <stdint.h>

enum {
  SH_OPEN   = 0x01,
  SH_CLOSE  = 0x02,
  SH_WRITEC = 0x03,
  SH_WRITE0 = 0x04,
  SH_WRITE  = 0x05,
  SH_READ   = 0x06,
  SH_ISERROR= 0x08,
  SH_SEEK   = 0x0A,
  SH_FLEN   = 0x0C,
  SH_REMOVE = 0x0E,
  SH_RENAME = 0x0F,
  SH_OPENW  = 0x01, 
  SH_EXIT   = 0x18
};

typedef struct {
  const char *path;
  uint32_t mode;  // 0=r, 4=wb, etc.
  uint32_t len;
} sh_open_block_t;

int      sh_open(const char *path, uint32_t mode);   // returns handle or -1
int      sh_close(int fd);
int      sh_read (int fd, void *buf, uint32_t len);  // returns remaining bytes
int      sh_write(int fd, const void *buf, uint32_t len); // returns remaining
int      sh_flen (int fd);                           // returns length or -1
void     sh_write0(const char *s);
void     sh_exit(int code);
