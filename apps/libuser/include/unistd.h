#ifndef UNISTD_H
#define UNISTD_H

#include <stdint.h>
#include <stddef.h> // For size_t

typedef int ssize_t; // Define ssize_t as signed size_t

// Change the program break (i.e., the end of the heap)
void* sbrk(intptr_t increment);

// Execute a new program. This will replace the current process image.
int exec(const char *path, int argc, char *const argv[]);

// Write to a file descriptor
ssize_t write(int fd, const void *buf, size_t count);

// Yield the CPU to another process
void yield(void);

#endif