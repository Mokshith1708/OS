#ifndef _SYS_CALLS_H
#define _SYS_CALLS_H

// Syscall numbers - MUST match OS/src/syscalls.c
#define SYS_EXIT  1
#define SYS_YIELD 2
#define SYS_READ  3
#define SYS_WRITE 4
#define SYS_EXEC  5
#define SYS_SBRK  6
#define SYS_OPEN  7
#define SYS_CLOSE 8
#define SYS_LSEEK 9
#define SYS_FSTAT 10
#define SYS_SLEEP 11
#define SYS_GETTIMEOFDAY 12
#define SYS_SEND_MSG 13
#define SYS_RECEIVE_MSG 14
#define SYS_ISATTY 15
#define SYS_KILL 16
#define SYS_GETPID 17

// File open flags - MUST match OS/src/include/fs.h
#define O_RDONLY    0x0001
#define O_WRONLY    0x0002
#define O_RDWR      0x0004
#define O_CREAT     0x0008
#define O_TRUNC     0x0010
#define O_APPEND    0x0020

#endif // _SYS_CALLS_H
