#include "semihost.h"

static inline uint32_t sh_call(uint32_t op, void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register void    *r1 __asm__("r1") = arg;
  __asm__ volatile ("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
  return r0;
}

int sh_open(const char *path, uint32_t mode) {
  sh_open_block_t blk = { path, mode, 0 };
  const char *p = path;
  while (p[blk.len]) blk.len++;
  int handle = (int)sh_call(SH_OPEN, &blk);
  return (handle == -1) ? -1 : handle;
}

int sh_close(int fd) {
  return (int)sh_call(SH_CLOSE, &fd);
}

int sh_read(int fd, void *buf, uint32_t len) {
  uint32_t blk[3] = { (uint32_t)fd, (uint32_t)buf, len };
  return (int)sh_call(SH_READ, blk);  // returns bytes NOT read
}

int sh_write(int fd, const void *buf, uint32_t len) {
  uint32_t blk[3] = { (uint32_t)fd, (uint32_t)buf, len };
  return (int)sh_call(SH_WRITE, blk); // returns bytes NOT written
}

int sh_flen(int fd) {
  return (int)sh_call(SH_FLEN, &fd);
}

void sh_write0(const char *s) { sh_call(SH_WRITE0, (void*)s); }
void sh_exit(int code)        { sh_call(SH_EXIT, (void*)(uintptr_t)code); }
