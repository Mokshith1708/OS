#include <stdint.h>
#include <sys/stat.h>
#include "hal/hal_console.h"

extern int _write(int file, char *ptr, int len);
extern int _read(int file, char *ptr, int len);
extern void *_sbrk(int incr);
extern int _close(int file);
extern int _fstat(int file, struct stat *st);
extern int _isatty(int file);
extern int _lseek(int file, int ptr, int dir);
extern void _exit(int code);
extern int _kill(int pid, int sig);
extern int _getpid(void);

typedef int (*syscall_fn_t)(uint32_t, uint32_t, uint32_t);

static int sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2) { _exit(code); return 0; }
static int sys_write(uint32_t file, uint32_t ptr, uint32_t len) { return _write(file, (char*)ptr, len); }
static int sys_read(uint32_t file, uint32_t ptr, uint32_t len) { return _read(file, (char*)ptr, len); }
static int sys_sbrk(uint32_t incr, uint32_t unused1, uint32_t unused2) { return (int)_sbrk(incr); }
static int sys_close(uint32_t file, uint32_t unused1, uint32_t unused2) { return _close(file); }
static int sys_fstat(uint32_t file, uint32_t st, uint32_t unused2) { return _fstat(file, (void*)st); }
static int sys_isatty(uint32_t file, uint32_t unused1, uint32_t unused2) { return _isatty(file); }
static int sys_lseek(uint32_t file, uint32_t ptr, uint32_t dir) { return _lseek(file, ptr, dir); }
static int sys_kill(uint32_t pid, uint32_t sig, uint32_t unused2) { return _kill(pid, sig); }
static int sys_getpid(uint32_t unused1, uint32_t unused2, uint32_t unused3) { return _getpid(); }

static syscall_fn_t syscall_table[] = {
    sys_exit,    // 0
    sys_write,   // 1
    sys_read,    // 2
    sys_sbrk,    // 3
    sys_close,   // 4
    sys_fstat,   // 5
    sys_isatty,  // 6
    sys_lseek,   // 7
    sys_kill,    // 8
    sys_getpid,  // 9
    // Add more as needed
};

void syscall_dispatch(uint32_t *stack) {
    uint32_t svc_num;
    // SVC number is in the instruction at stacked PC - 2
    uint32_t pc = stack[6];
    uint16_t svc_instr = *((uint16_t *)(pc - 2));
    svc_num = svc_instr & 0xFF;

    if (svc_num < sizeof(syscall_table)/sizeof(syscall_table[0])) {
        int ret = syscall_table[svc_num](stack[0], stack[1], stack[2]);
        stack[0] = ret; // Return value in r0
    }
}